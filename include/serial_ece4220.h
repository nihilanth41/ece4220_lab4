/*
 * serial_siavash.h
 *
 *  Created on: Jan 7, 2013
 *      Author: sfkt6
 */

#ifndef SERIAL_ECE4220_H_
#define SERIAL_ECE4220_H_

void serial_read(int port_id, unsigned char *x, int num_bytes);
void serial_write(int port_id, unsigned char *x, int num_bytes);
int serial_open(int port, int initial_baud, int final_baud);
void serial_close(int ofp);

#endif /* SERIAL_ECE4220_H_ */
