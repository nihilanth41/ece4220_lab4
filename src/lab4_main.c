#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <fcntl.h>
#include <pthread.h>
#include "serial_ece4220.h"

#define DATA_MAX 256
#define FIFO_ID 1

typedef struct gps_st {
	uint8_t data;
	struct timeval tv;
} gps_t;

pthread_t tid;

// Common buffer for data and time
int i=0;
gps_t data_buffer[DATA_MAX];

void read_fifo(void *args) {
	struct timeval tv_buf;
	int fd = open("/dev/rtf/1", O_RDWR);
	if(-1 == fd)
	{
		return;
	}
	while(1) {
		int count = read(fd, &tv_buf, sizeof(tv_buf));
		if(count < 0)
		{
			return;
		}
		else
		{
			// spin up thread on each button press to wait for the next data
			printf("Button pressed\n");
			printf("Kernel Event Time: %u.%ld \n", (unsigned int)tv_buf.tv_sec, tv_buf.tv_usec);
			printf("Last GPS Event: Data: %d, Time: %u.%ld \n", data_buffer[i-1].data, (unsigned int)data_buffer[i-1].tv.tv_sec, data_buffer[i-1].tv.tv_usec);
		}
	}
}

int main(int argc, char **argv) {
	// Create thread to read from fifo (kernel)
	pthread_create(&tid, NULL, (void *)read_fifo, NULL); //(void *)
	// Attempt to open serial port
	int port_id = serial_open(0, 0, 5);
        unsigned char buf=0;
	ssize_t num_bytes=0;
	// Endlessly loop and read from serial port
	while(1)
	{
		// Read into array at pos(i);
		for(i=0; i<DATA_MAX; i++)
		{
			struct timeval *tv = &(data_buffer[i].tv);
			num_bytes = read(port_id, &buf, 1);
			if(-1 == num_bytes)
			{
				// Error read()
				break;
			}
			data_buffer[i].data = buf;
			// Get timestamp
			int ret = gettimeofday(tv, NULL);
			if(-1 == ret)
			{
				// Error gettime()
				break; 
			}
			//printf("Data: %d, Time: %u.%ld \n", data_buffer[i].data, (unsigned int)data_buffer[i].tv.tv_sec, data_buffer[i].tv.tv_usec);
			//fflush(stdout);
		}
	}
	serial_close(port_id);
	return 0;
}
