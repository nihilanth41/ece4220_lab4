#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>
#include <unistd.h>
#include "serial_ece4220.h"

int main(int argc, char **argv) {

	// Attempt to open serial port
	int port_id = serial_open(0, 0, 5);
        unsigned char buf=0;
	int i=0;
	ssize_t num_bytes=0;
	for(i=0; i<200; i++)
	{
		num_bytes = read(port_id, &buf, 1);
		printf("%d\n", buf);
		fflush(stdout);
	}

	serial_close(port_id);

	return 0;
}
