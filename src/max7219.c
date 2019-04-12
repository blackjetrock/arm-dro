// Test of MAX7219 display

#include "chip.h"
#include "board.h"

#if 1

const int DIN_PIN =  14;
const int CS_PIN  =  16;
const int CLK_PIN =  13;

#define MAX_HIGH 0
#define MAX_LOW  1

const int CMD_NOOP    = 0;
const int CMD_DIGIT0  = 1;
const int CMD_DIGIT1  = 2;
const int CMD_DIGIT2  = 3;
const int CMD_DIGIT3  = 4;
const int CMD_DIGIT4  = 5;
const int CMD_DIGIT5  = 6;
const int CMD_DIGIT6  = 7;
const int CMD_DIGIT7  = 8;

const int CMD_DECODE_MODE = 0x9;
const int CMD_INTENSITY   = 0xa;
const int CMD_SCAN_LIMIT  = 0xb;
const int CMD_SHUTDOWN    = 0xc;
const int CMD_TEST        = 0xf;

const int DIGIT_CHAR_DASH  = 0xa;
const int DIGIT_CHAR_BLANK = 0xf;

void max7219_setup(void)
{
  Chip_GPIO_SetPinDIROutput(LPC_GPIO, 1, DIN_PIN);
  Chip_GPIO_SetPinDIROutput(LPC_GPIO, 1, CS_PIN);
  Chip_GPIO_SetPinDIROutput(LPC_GPIO, 1, CLK_PIN);

  max7219_write_cmd(0, CMD_TEST,           0);
  max7219_write_cmd(0, CMD_INTENSITY,      1);
  max7219_write_cmd(0, CMD_DECODE_MODE, 0xFF);
  max7219_write_cmd(0, CMD_SHUTDOWN,       1);
  max7219_write_cmd(0, CMD_SCAN_LIMIT,     7);

  
  max7219_write_cmd(1, CMD_TEST,           0);
  max7219_write_cmd(1, CMD_INTENSITY, 1);
  max7219_write_cmd(1, CMD_DECODE_MODE, 0xff);
  max7219_write_cmd(1, CMD_SHUTDOWN, 1);
  max7219_write_cmd(1, CMD_SCAN_LIMIT, 7);


  max7219_write_cmd(2, CMD_TEST,           0);
  max7219_write_cmd(2, CMD_INTENSITY, 1);
  max7219_write_cmd(2, CMD_DECODE_MODE, 0xff);
  max7219_write_cmd(2, CMD_SHUTDOWN, 1);
  max7219_write_cmd(2, CMD_SCAN_LIMIT, 7);

}

//
// Write data to an address, n displays
//

#define NUM_DIGITS 3

int current_digit[NUM_DIGITS];

#define WAIT_7219 20

void max7219_write_cmd(int digit, int command, int cmd_data)
{
  int i,j;
  int digit_index;
  int data;
  
  for(digit_index=0; digit_index< NUM_DIGITS; digit_index++)
    {
      if( digit == digit_index )
	{
	  data = ((command & 0xf)<<8) | (cmd_data & 0xff);
	}
      else
	{
	  data = current_digit[digit_index];
	}
      
      // Write a binary pattern repeatedly
      for(i=15; i>=0; i--)
	{
	  for(j=0; j<WAIT_7219;j++)
	    {
	    }

	  // Clock out MSB first
	  if( data & (1 << i))
	    {
	      Chip_GPIO_SetPinState(LPC_GPIO, 1, DIN_PIN, MAX_HIGH);

	      //	      write_port d(DIN_PIN, MAX_HIGH);
	    }
	  else
	    {
	      Chip_GPIO_SetPinState(LPC_GPIO, 1, DIN_PIN, MAX_LOW);
	      //	      write_portd(DIN_PIN, MAX_LOW);
	    }
	  
	  // Clock data in
	      Chip_GPIO_SetPinState(LPC_GPIO, 1, CLK_PIN, MAX_HIGH);
	      //	  write_portd(CLK_PIN, MAX_HIGH);

	  for(j=0; j<WAIT_7219;j++)
	    {
	    }

	      Chip_GPIO_SetPinState(LPC_GPIO, 1, CLK_PIN, MAX_LOW);
	      //	  write_portd(CLK_PIN, MAX_LOW);
	}
      }
  
  // Load data
  Chip_GPIO_SetPinState(LPC_GPIO, 1, CS_PIN, MAX_HIGH);
  for(j=0; j<WAIT_7219;j++)
    {
    }

  Chip_GPIO_SetPinState(LPC_GPIO, 1, CS_PIN, MAX_LOW);
  //write_portd(CS_PIN, MAX_HIGH);
  //write_portd(CS_PIN, MAX_LOW);
  
}

int d4;
#define MAX_DIGITS 8
#define DISPLAYED_DIGITS 6

//
// Displays an integer n, with leading zero suppression on or
// off and a decimal point position (-1 is no DP)
// Any display can be specified.
// Heartbeat on top DP of dislay 0

unsigned char hb = 0;

void max7219_write_int(int display, long int n, int lead_zero, int dp_pos)
{
  int dig;
  int v;
  int l0_flag = lead_zero ? 1:0;
  int digitbuffer[MAX_DIGITS];
  long int number = n;
  int end_l0_digit;

  if ( dp_pos == -1 )
    {
      // No DP so end digit is end of number
      end_l0_digit = 0;
    }
  else
    {
      // DP is present so last digit is the one the DP is on
      end_l0_digit = dp_pos;
    }

  // Blank buffer
  for(dig=0; dig<MAX_DIGITS; dig++)
    {
      digitbuffer[dig] = DIGIT_CHAR_BLANK;
    }

  // Process sign 
  if ( number < 0 )
    {
      digitbuffer[7] = DIGIT_CHAR_DASH;
      n = -n;
    }
  else
    {
      digitbuffer[7] = DIGIT_CHAR_BLANK;
    }


  // Digits displayed from MS to LS
  for(dig=0; dig<DISPLAYED_DIGITS; dig++)
    {
      digitbuffer[dig] = (n % 10);
      n /= 10;
    }

  // Process leading zeros
  if ( lead_zero )
    {
      // Scan all digits after sign digit
      for(dig=(DISPLAYED_DIGITS-1); dig>=0; dig--)
	{
	  v = digitbuffer[dig];

	  // Turn off leadin zeroing if we see a non zero diit, or we have reached the 
	  // end of the number
	  // We also ned to stop if there's a DP position, as we don't want to
	  // remove leadin zeros beyond the DP digit
	  if( (v != 0) || (dig == end_l0_digit))
	    {
	      l0_flag = 0;
	    }
	  
	  if( l0_flag && (v == 0))
	    {
	      v = DIGIT_CHAR_BLANK;
	    }

	  digitbuffer[dig] = v;	  
	}
    }

#if 0
  if( display == 0 )
    {
      digitbuffer[7] |= (hb % 8)>=3?0x80:0x00;
      hb++;
    }
#endif

  // Write digits to display
  for(dig=0; dig<MAX_DIGITS; dig++)
    {
      // Process DP
      if( dp_pos == dig )
	{
	  digitbuffer[dig] |= 0x80;
	}

      max7219_write_cmd(display, CMD_DIGIT0+dig, digitbuffer[dig]);
    }
}

#endif
