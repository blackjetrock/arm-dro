/*
 * oled096.c
 *
 *  Created on: 23 Jul 2016
 *      Author: menadue
 */

//
// 0.96" OLED display functions
//


// Sends a block of commands to the SSD1306
// command causes 80 to be sent before each byte
// command = 0 causes 0x40 and just data bytes
// Repeat = 1, repeat data byte 0

#include <string.h>
#include "i2c_functions.h"
#include "oled096.h"

void oled_send_cmd(I2C_SLAVE_DESC *slave, int n, unsigned char *data, int command, int repeat)
{
  int i;

  i2c_start(slave->port);

  // Send slave address
  i2c_send_byte(slave->port, (slave->slave_7bit_addr)<<1);

  if ( !command )
    {
      i2c_send_byte(slave->port, 0x40);
    }

  // Send the command bytes, each preceded bu 0x80 (continuation)
  for (i = 0; i < n; i++) {
    if ( command )
      {
        i2c_send_byte(slave->port, 0x80);
      }
    i2c_send_byte(slave->port, *data);

    if ( !repeat )
      {
	data++;
      }
  }

  i2c_stop(slave->port);
}

//--------------------------------------------------------------------------------
// OLED display functions
//

const unsigned char init_seq[] = {
  0xae,                       // ???
  0xd5, 0x80,

  0xa8,0x3f,

  0xd3,0x00,
  0x8d,0x14,0x40,0xa6,0xa4,0xa1,0xc8,0xda,0x12,0x81,0x8f,
  0xd9,         //??

  0xf1,0xdb,0x40,0xaf,
};

// From data sheet
const unsigned char init_seq2[] = {
  //  0xae,          // Display Off
  0xa8,0x3f,       // MUX ratio works for now
  0xd3,0x00,       // Set display offset 0
  0x40,            // Set display start Line
  0x8d,0x14,       // Charge pump ON, needed or blank display
  0xa1,            // Seg remap 180 rotate
  0xc8,            // Inverted COM scan
  0xda, 0x12,      // COM config: A5:Disable left/right remap, A4:Alternate COM pin config
  0x81,0x2f,      // Contrast value
  0xd9, 0xf1,     // Precharge, quite important
  0xdb, 0x40,     // Set Vcomh level, leave it out and inverted display.

  0xa6,            // Normal display (not inverted)
  0xd5,0x80,       // Set display oscillator

  0x20, 0x02,        // Page addressing mode
  0x21,0x00,0x7f,               // Set display RAM start and end address
                                // seems to have effect in page addressing mode.
  0x22,0x00,0x07,               // Set display page address

  0xa4,
  0xaf,             // Display ON
};

const unsigned char display_setup_seq[] = {
  0x21,0x00,0x7f,               // Set display RAM start and end address
  0x22,0x00,0x3f,               // Set display page address
};


const unsigned char display_text_seq[] = {
  0x7e,0x11,0x11,0x11,0x7e,0x00,0x7f,0x49,0x49,0x49,0x36,0x00,0x00,0x3e,0x41,0x41,0x41,0x22,0x00,0x7f,0x41,0x41,0x41,0x22,0x1c,0x00,0x7f,0x49,0x49,0x49,0x41,0x00,
};

// Set XY to given position
// We attempt to position to the byte that holds the pixel (x,y)
// Page addressing mode

void oled_set_xy(I2C_SLAVE_DESC *slave, int x, int y)
{
  unsigned char seq[3];

  x = x % 128;
  y = y % 64;
  seq[0] = 0xB0+y/8;      // Set page
  seq[1] = 0x00+x%16;     // Set low part of start address
  seq[2] = 0x10+x/16;     // Set high part

  oled_send_cmd(slave, sizeof(seq), &seq, I2C_CMD, I2C_NO_REPEAT);
}

unsigned char byteval;

// Writes bit pattern at x,y
void oled_set_pixel_xy(I2C_SLAVE_DESC *slave, int x, int y)

{
  byteval = 1<< (y % 8);

  oled_set_xy(slave->port, x, y);
  oled_send_cmd(slave, 1, &byteval, I2C_DATA, I2C_NO_REPEAT);
}

void oled_set_byte_xy(I2C_SLAVE_DESC *slave, int x, int y, int b)
{
  byteval = b;

  oled_set_xy(slave, x, y);
  oled_send_cmd(slave, 1, &byteval, I2C_DATA, I2C_NO_REPEAT);
}


// Font

const unsigned char font_5x7_letters[] = {
  0x00, 0x00, 0x00, 0x00, 0x00,// (space)
  0x00, 0x00, 0x5F, 0x00, 0x00,// !
  0x00, 0x07, 0x00, 0x07, 0x00,// "
  0x14, 0x7F, 0x14, 0x7F, 0x14,// #
  0x24, 0x2A, 0x7F, 0x2A, 0x12,// $
  0x23, 0x13, 0x08, 0x64, 0x62,// %
  0x36, 0x49, 0x55, 0x22, 0x50,// &
  0x00, 0x05, 0x03, 0x00, 0x00,// '
  0x00, 0x1C, 0x22, 0x41, 0x00,// (
  0x00, 0x41, 0x22, 0x1C, 0x00,// )
  0x08, 0x2A, 0x1C, 0x2A, 0x08,// *
  0x08, 0x08, 0x3E, 0x08, 0x08,// +
  0x00, 0x50, 0x30, 0x00, 0x00,// ,
  0x08, 0x08, 0x08, 0x08, 0x08,// -
  0x00, 0x60, 0x60, 0x00, 0x00,// .
  0x20, 0x10, 0x08, 0x04, 0x02,// /
  0x3E, 0x51, 0x49, 0x45, 0x3E,// 0
  0x00, 0x42, 0x7F, 0x40, 0x00,// 1
  0x42, 0x61, 0x51, 0x49, 0x46,// 2
  0x21, 0x41, 0x45, 0x4B, 0x31,// 3
  0x18, 0x14, 0x12, 0x7F, 0x10,// 4
  0x27, 0x45, 0x45, 0x45, 0x39,// 5
  0x3C, 0x4A, 0x49, 0x49, 0x30,// 6
  0x01, 0x71, 0x09, 0x05, 0x03,// 7
  0x36, 0x49, 0x49, 0x49, 0x36,// 8
  0x06, 0x49, 0x49, 0x29, 0x1E,// 9
  0x00, 0x36, 0x36, 0x00, 0x00,// :
  0x00, 0x56, 0x36, 0x00, 0x00,// ;
  0x00, 0x08, 0x14, 0x22, 0x41,// <
  0x14, 0x14, 0x14, 0x14, 0x14,// =
  0x41, 0x22, 0x14, 0x08, 0x00,// >
  0x02, 0x01, 0x51, 0x09, 0x06,// ?
  0x32, 0x49, 0x79, 0x41, 0x3E,// @
  0x7E, 0x11, 0x11, 0x11, 0x7E,// A
  0x7F, 0x49, 0x49, 0x49, 0x36,// B
  0x3E, 0x41, 0x41, 0x41, 0x22,// C
  0x7F, 0x41, 0x41, 0x22, 0x1C,// D
  0x7F, 0x49, 0x49, 0x49, 0x41,// E
  0x7F, 0x09, 0x09, 0x01, 0x01,// F
  0x3E, 0x41, 0x41, 0x51, 0x32,// G
  0x7F, 0x08, 0x08, 0x08, 0x7F,// H
  0x00, 0x41, 0x7F, 0x41, 0x00,// I
  0x20, 0x40, 0x41, 0x3F, 0x01,// J
  0x7F, 0x08, 0x14, 0x22, 0x41,// K
  0x7F, 0x40, 0x40, 0x40, 0x40,// L
  0x7F, 0x02, 0x04, 0x02, 0x7F,// M
  0x7F, 0x04, 0x08, 0x10, 0x7F,// N
  0x3E, 0x41, 0x41, 0x41, 0x3E,// O
  0x7F, 0x09, 0x09, 0x09, 0x06,// P
  0x3E, 0x41, 0x51, 0x21, 0x5E,// Q
  0x7F, 0x09, 0x19, 0x29, 0x46,// R
  0x46, 0x49, 0x49, 0x49, 0x31,// S
  0x01, 0x01, 0x7F, 0x01, 0x01,// T
  0x3F, 0x40, 0x40, 0x40, 0x3F,// U
  0x1F, 0x20, 0x40, 0x20, 0x1F,// V
  0x7F, 0x20, 0x18, 0x20, 0x7F,// W
  0x63, 0x14, 0x08, 0x14, 0x63,// X
  0x03, 0x04, 0x78, 0x04, 0x03,// Y
  0x61, 0x51, 0x49, 0x45, 0x43,// Z
  0x00, 0x00, 0x7F, 0x41, 0x41,// [
  0x02, 0x04, 0x08, 0x10, 0x20,// "\"
  0x41, 0x41, 0x7F, 0x00, 0x00,// ]
  0x04, 0x02, 0x01, 0x02, 0x04,// ^
  0x40, 0x40, 0x40, 0x40, 0x40,// _
  0x00, 0x01, 0x02, 0x04, 0x00,// `
  0x20, 0x54, 0x54, 0x54, 0x78,// a
  0x7F, 0x48, 0x44, 0x44, 0x38,// b
  0x38, 0x44, 0x44, 0x44, 0x20,// c
  0x38, 0x44, 0x44, 0x48, 0x7F,// d
  0x38, 0x54, 0x54, 0x54, 0x18,// e
  0x08, 0x7E, 0x09, 0x01, 0x02,// f
  0x08, 0x14, 0x54, 0x54, 0x3C,// g
  0x7F, 0x08, 0x04, 0x04, 0x78,// h
  0x00, 0x44, 0x7D, 0x40, 0x00,// i
  0x20, 0x40, 0x44, 0x3D, 0x00,// j
  0x00, 0x7F, 0x10, 0x28, 0x44,// k
  0x00, 0x41, 0x7F, 0x40, 0x00,// l
  0x7C, 0x04, 0x18, 0x04, 0x78,// m
  0x7C, 0x08, 0x04, 0x04, 0x78,// n
  0x38, 0x44, 0x44, 0x44, 0x38,// o
  0x7C, 0x14, 0x14, 0x14, 0x08,// p
  0x08, 0x14, 0x14, 0x18, 0x7C,// q
  0x7C, 0x08, 0x04, 0x04, 0x08,// r
  0x48, 0x54, 0x54, 0x54, 0x20,// s
  0x04, 0x3F, 0x44, 0x40, 0x20,// t
  0x3C, 0x40, 0x40, 0x20, 0x7C,// u
  0x1C, 0x20, 0x40, 0x20, 0x1C,// v
  0x3C, 0x40, 0x30, 0x40, 0x3C,// w
  0x44, 0x28, 0x10, 0x28, 0x44,// x
  0x0C, 0x50, 0x50, 0x50, 0x3C,// y
  0x44, 0x64, 0x54, 0x4C, 0x44,// z
  0x00, 0x08, 0x36, 0x41, 0x00,// {
  0x00, 0x00, 0x7F, 0x00, 0x00,// |
  0x00, 0x41, 0x36, 0x08, 0x00,// }
  0x08, 0x08, 0x2A, 0x1C, 0x08,// ->
  0x08, 0x1C, 0x2A, 0x08, 0x08 // <-
};

// Turn display off
const unsigned char off_seq = {
  0xae
};

// Turn display on
const unsigned char on_seq = {
  0xaf
};


// Prints a character gap
void oled_gap(I2C_SLAVE_DESC *slave)
{
  unsigned char zero = 0;

  oled_send_cmd(slave, 1, &zero, I2C_DATA, I2C_NO_REPEAT);
}

// Displays an integer in decimal, number of digits displayed is specified, MS digits dropped

void oled_display_int(I2C_SLAVE_DESC *slave, long int n, int num_digits)
{
  int i;
  int dig;
  int mul = 10000;

  if( n < 0 )
    {
      oled_display_string(slave, "-");
      n = -n;
    }
  else
    {
      oled_display_string(slave, " ");
    }

  for(i=5; i>0; i--)
    {
      dig = n / mul;
      n = n % mul;
      mul /=10;

      // Display digit
      if ( i <= num_digits )
	{
	  oled_send_cmd(slave, 5, font_5x7_letters+('0'-' ')*5+dig*5, I2C_DATA, I2C_NO_REPEAT);
	  oled_gap(slave);
	}
    }
}

void oled_display_string(I2C_SLAVE_DESC *slave, char *string)
{
  int j;
  int len = strlen(string);

  for(j=0; j<len; j++)
    {
      oled_send_cmd(slave, 5, font_5x7_letters+((*string++) - ' ')*5, I2C_DATA, I2C_NO_REPEAT);
      oled_gap(slave);
    }
}


void oled_clear_display(I2C_SLAVE_DESC *slave)
{
  int y;
  unsigned char zero = 0;

  for(y=0; y<64; y+=8)
    {
      oled_set_xy(slave, 0, y);
      oled_send_cmd(slave, 128, &zero, I2C_DATA, I2C_REPEAT);
    }

}

//
// Displays a string with pixel scaling
//
unsigned char scaled_line[100];

void oled_display_scaled_string_xy(I2C_SLAVE_DESC *slave, char *string, int x, int y, int scale)
{
  unsigned char i;
  int  j, k, s;
  int scale_i;
  int len = strlen(string);
  int b;

  // We now display the string a character at a time, each character being scaled
  scale_i = 0;

  for(k=0; k<len; k++)
    {
      // Scale character
      // We start with 5 bytes, each of 8 bits, these are vertical slices of the bit pattern
      // We need to stretch the character vertically by scale

      for(i=0; i<5; i++)
	{
	  // For each byte we build a new bitmap that is scaled
	  for(j=0; j<8; j++)
	    {
	      // Get next bit
	      b  = (*(font_5x7_letters+((*string)-' ')*5+i));
	      b &= (1 << (7-j));
	      b >>= (7-j);

	      // Shift more bits in to scaled data
	      for(s=0; s<scale;s++)
		{
		  scaled_line[scale_i/8] <<= 1;
		  scaled_line[scale_i/8] |= b;
		  scale_i++;
		}
	    }
#if 0
	  // Move to next byte
	  while( (scale_i % 8) != 0 )
	    {
	      scale_i++;
	    }
#endif
	}

      // Put a gap between chars
      for(s=0; s<scale;s++)
	{
	  scaled_line[scale_i/8] = 0;
	  scale_i+=8;
	}

      string++;
    }

  // Send scaled data to display
  for(j=0; j < scale; j++)
    {
      oled_set_xy(slave, x, y+8*((scale-1)-j));
      for(i=0; i<5*len*scale; i+=scale)
	{
	  for(s=0; s<scale;s++)
	    {
	      oled_send_cmd(slave, 1, &(scaled_line[i+j]), I2C_DATA, I2C_NO_REPEAT);
	    }
	}
      oled_gap(slave);
    }

#if 0
  oled_set_xy(slave->port, 0,0);
  for(j=0;j<scale_i/8;j++)
    {
      oled_send_cmd(slave, 1, &(scaled_line[j]), I2C_DATA, I2C_NO_REPEAT);
    }


  oled_set_xy(slave, 30, 24);
  oled_display_int(slave, scale_i,5);
  oled_set_xy(slave, 30, 32);
  oled_display_int(slave, len,5);
#endif
}

void oled_setup(I2C_SLAVE_DESC *slave)
{
    int i;

  // Release bus
  i2c_release(slave->port);

  // Delay to allow things to start up.
  for(i=0; i<10000; i++)
    {
    }

  // Then clear any glitched I2C commands with a start and stop
  i2c_start(slave->port);
  i2c_stop(slave->port);

  // Initialise display
  oled_send_cmd(slave, sizeof(init_seq2), &init_seq2, 1, 0);

  // Clear display
  oled_clear_display(slave);


}
