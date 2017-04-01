#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <fcntl.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include "serial_ece4220.h"

// Size of global buffer
#define DATA_MAX 256
// Max number of push button events that can happen in the 250ms window
// Determines size of pthread_t array and thread argument array
#define EVENT_MAX 5
#define FIFO_READ /dev/rtf/1  // /dev/rtf/<FIFO_ID>
#define FIFO_WRITE named_pipe // Thread 1 will write, Thread 2 will read and print to stdout

// Struct to hold data + timestamp from serial port
typedef struct gps_st {
  uint8_t data;
  struct timeval tv;
} gps_t;

// Struct to hold above struct and the timestamp from the pushbutton event.
typedef struct gps_final {
  unsigned int gps_last_index;
  gps_t gps_last;
  gps_t gps_next;
  struct timeval event_tv;
} gps_final;

gps_t data_buffer[DATA_MAX]; // Global buffer for gps data + time stamp
int i=0; // Global iterator for global buffer
static unsigned int event_count = 0; // Push button event counter - index into dynamic thread array


// Returns the index in the global data_buffer array of the last GPS event
// from the serial port.
unsigned int lastIndex(unsigned int j) {
  if(0 == j)
    {
      return (unsigned int)(DATA_MAX-1);
    }
  else
    return (unsigned int)j-1;
}

unsigned int nextIndex(unsigned int j) {
  if(DATA_MAX-1 == j)
    {
      return (unsigned int)0;
    }
  else
    return (unsigned int)j+1;
}

void fifo_print(void) {
  int fd = open("FIFO_WRITE", O_RDONLY);
  if(fd < 0)
    {
      // Error
      fprintf(stderr, "Error open() FIFO_WRITE\n%s\n", strerror(errno));
      return;
    }
  while(1)
    {
      gps_final gps_data;
      int ret = read(fd, (void *)&gps_data, sizeof(gps_data));
      if(ret < 0)
	{
	    // Error
	    fprintf(stderr, "Error read() FIFO_WRITE\n%s\n", strerror(errno));
	    return;
	}
      // TODO interpolate
      // y = y0 + (x - x0)( (y1-y0) / (x1-x0) )
      // x = time, y = data
      double x0,x1,y0,y1,x,y;
      // Combine integral usec and second values into a single floating point value.
      x0 = (double)gps_data.gps_last.tv.tv_sec + (((double)(gps_data.gps_last.tv.tv_usec))*(0.000001));
      x = (double)gps_data.event_tv.tv_sec + (((double)(gps_data.event_tv.tv_usec))*(0.000001));
      x1 = (double)gps_data.gps_next.tv.tv_sec + (((double)(gps_data.gps_next.tv.tv_usec))*(0.000001));
      y0 = (double)gps_data.gps_last.data;
      y1 = (double)gps_data.gps_next.data;
      // Linearly Interpolate data point (y) given the three time points (x)
      y = y0 + (x-x0)*((y1-y0)/(x1-x0));

      // Print to stdout
      printf("Last GPS event: %d, %u.%ld\n", gps_data.gps_last.data, (unsigned int)gps_data.gps_last.tv.tv_sec, gps_data.gps_last.tv.tv_usec);
      printf("(Interpolated) Push Button event: %lf, %u.%ld\n", y, (unsigned int)gps_data.event_tv.tv_sec, gps_data.event_tv.tv_usec);
      printf("Next GPS event: %d, %u.%ld\n", gps_data.gps_last.data, (unsigned int)gps_data.gps_last.tv.tv_sec, gps_data.gps_last.tv.tv_usec);
    }
}

// Function to wait for the next serial port event after a button press event
// Args: gps_final struct populated with everything except the gps_last field.
// Waits for the next serial port data (i.e. global i = last_index + 2)
// Populates the gps_next field with data_buffer[last_index+1] 
void serial_wait(void *args) {
  if(NULL == args)
    {
      return;
    }
  gps_final *gps_dp = (gps_final *)args;

  int fd = open("FIFO_WRITE", O_WRONLY);
  if(-1 == fd)
    {
      // Error
      fprintf(stderr, "Error open() FIFO_WRITE\n%s\n", strerror(errno));
      return;
    }
  while(1) // Loop until iterator is equal to gps_last_index + 2
    // Then write data to the named pipe
    {
      // Copy value of global iterator
      unsigned int j = (unsigned)i;
      // If global iterator == last_index + 2
      if(j == nextIndex(nextIndex(gps_dp->gps_last_index)))
	{
	  // Copy the last gps value
	  gps_dp->gps_last = data_buffer[lastIndex(j)];
	}
      // Write data to fifo
      int ret = write(fd, (void *)gps_dp, sizeof(gps_dp));
      if(-1 == ret)
	{
	    fprintf(stderr, "Error write FIFO_WRITE\n%s\n", strerror(errno));
	}
      break;
    }
}


// Scheduled every 75ms
void read_fifo(void) {
  struct timeval event_tv;
  static pthread_t thread_buf[EVENT_MAX];
  static gps_final gps_data[EVENT_MAX];

  int fd = open("FIFO_READ", O_RDWR);
  if(-1 == fd)
    {
      // Error
      fprintf(stderr, "Error open() RT_FIFO\n%s\n", strerror(errno));
      return;
    }
  while(1) {
    // Get time stamp of push button event 
    int count = read(fd, &event_tv, sizeof(event_tv));
    if(count < 0)
      {
	// Error
	fprintf(stderr, "Error read() from RT_FIFO\n%s\n", strerror(errno));
	return;
      }
    else
      {
	unsigned int last_index = lastIndex(i);
	// Assign last index and last data + timestamp
	gps_data[event_count].gps_last_index = last_index;
	gps_data[event_count].gps_last = data_buffer[last_index];
	gps_data[event_count].event_tv = event_tv;
	// Spin up thread on each button press
	pthread_create(&thread_buf[event_count], NULL, (void *)serial_wait, (void *)&gps_data[event_count]);
	// Increment event counter
	event_count++;
      }
  }
}

int main(void) {
  pthread_t tid;
  unsigned char buf=0;
  ssize_t num_bytes=0;
  // Create named pipe
  int ret = system("mkfifo FIFO_WRITE");
  // Create thread to read from kernel fifo (real time fifo)
  pthread_create(&tid, NULL, (void *)read_fifo, NULL);
  // Create thread to read from user space fifo (written by serial_wait())
  // Attempt to open serial port
  int port_id = serial_open(0, 0, 5);
  // Endlessly loop and read from serial port
  while(1)
    {
      // Read into array at pos(i);
      for(i=0; i<DATA_MAX; i++)
	{
	  num_bytes = read(port_id, &buf, 1);
	  if(-1 == num_bytes)
	    {
	      // Error read()
	      return EXIT_FAILURE;
	    }
	  // Reset event count each time serial data is received
	  event_count = 0; 
	  data_buffer[i].data = buf;
	  // Get timestamp
	  struct timeval *tv_ptr = NULL;
	  ret = gettimeofday(tv_ptr, NULL);
	  if(-1 == ret)
	    {
	      // Error gettime()
	      return EXIT_FAILURE;
	    }
	  else
	    {
	      data_buffer[i].tv = *tv_ptr;
	    }
	}
    }
  serial_close(port_id);
  return EXIT_SUCCESS;
}
