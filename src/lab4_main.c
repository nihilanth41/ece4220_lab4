#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/time.h>
#include "serial_ece4220.h"

#define DATA_MAX 256

typedef struct gps_st {
	uint8_t data;
	struct timeval tv;
} gps_t;

// Common buffer for data and time
gps_t data_buffer[DATA_MAX];

int main(int argc, char **argv) {

	// Attempt to open serial port
	int port_id = serial_open(0, 0, 5);
        unsigned char buf=0;
	int i=0;
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
			printf("Data: %d, Time: %u.%ld \n", data_buffer[i].data, (unsigned int)data_buffer[i].tv.tv_sec, data_buffer[i].tv.tv_usec);
			fflush(stdout);
		}
	}
	serial_close(port_id);
	return 0;
}
