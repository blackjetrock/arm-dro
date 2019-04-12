/*
 * oled096.h
 *
 *  Created on: 23 Jul 2016
 *      Author: menadue
 */

#ifndef OLED096_H_
#define OLED096_H_

#define I2C_CMD       1
#define I2C_DATA      0
#define I2C_REPEAT    1
#define I2C_NO_REPEAT 0

void oled_send_cmd(I2C_SLAVE_DESC *slave, int n, unsigned char *data, int command, int repeat);
void oled_set_xy(I2C_SLAVE_DESC *slave, int x, int y);
void oled_set_pixel_xy(I2C_SLAVE_DESC *slave, int x, int y);
void oled_set_byte_xy(I2C_SLAVE_DESC *slave, int x, int y, int b);
void oled_gap(I2C_SLAVE_DESC *slave);
void oled_display_int(I2C_SLAVE_DESC *slave, long int n, int num_digits);
void oled_display_string(I2C_SLAVE_DESC *slave, char *string);
void oled_clear_display(I2C_SLAVE_DESC *slave);

void oled_setup(I2C_SLAVE_DESC *slave);
void oled_display_scaled_string_xy(I2C_SLAVE_DESC *slave, char *string, int x, int y, int scale);

extern const unsigned char font_5x7_letters[];

#endif /* OLED096_H_ */
