/*
 * max7219.h
 *
 *  Created on: 24 Jul 2016
 *      Author: menadue
 */

#ifndef MAX7219_H_
#define MAX7219_H_

void max7219_setup(void);
void max7219_write_cmd(int digit, int command, int cmd_data);
void max7219_write_int(int display, long int n, int lead_zero, int dp_pos);


#endif /* MAX7219_H_ */
