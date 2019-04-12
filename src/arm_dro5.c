/*
  ===============================================================================
  Name        : arm_dro5.c
  Author      : $(author)
  Version     :
  Copyright   : $(copyright)
  Description : main definition
  ===============================================================================
*/

#if defined (__USE_LPCOPEN)
#if defined(NO_BOARD_LIB)
#include "chip.h"
#else
#include "board.h"
#endif
#endif

#include <cr_section_macros.h>
#include <string.h>
#include <stdio.h>

#include "max7219.h"
#include "i2c_functions.h"
#include "oled096.h"
#include "tft_il9341.h"
#include "menu.h"
#include "arm_dro.h"

long int gauge_x, gauge_y, gauge_z = 0;
long offset_x, offset_y, offset_z = 0;
SUM_MODE sum_mode_x = SUM_MODE_X_X, sum_mode_y = SUM_MODE_Y_Y, sum_mode_z;

// OLED displays
I2C_PORT_DESC i2c_bus_0 =
  {
    0, 18,
    0, 7,
  };

I2C_PORT_DESC i2c_bus_1 =
  {
    0, 2,
    0, 19,
  };

// Swapped due to wiring problem
I2C_PORT_DESC i2c_bus_2 =
  {
    1, 19,
    1, 20,
  };

I2C_SLAVE_DESC oled0 =
  {
    &i2c_bus_0,
    0x78 >>1,
  };

I2C_SLAVE_DESC oled1 =
  {
    &i2c_bus_1,
    0x78 >>1,
  };

I2C_SLAVE_DESC oled2 =
  {
    &i2c_bus_2,
    0x78 >>1,
  };



I2C_SLAVE_DESC gauge1 =
  {
    &i2c_bus_0,
    0x40 >>1,
  };

I2C_SLAVE_DESC gauge4 =
  {
    &i2c_bus_1,
    0x40 >>1,
  };

I2C_SLAVE_DESC gauge5 =
  {
    &i2c_bus_1,
    0x30 >>1,
  };



//--------------------------------------------------------------------------------

I2C_SLAVE_DESC gauge2 =
  {
    &i2c_bus_2,
    0x30 >>1,
  };

I2C_SLAVE_DESC gauge3 =
  {
    &i2c_bus_2,
    0x40 >>1,
  };

I2C_SLAVE_DESC gauge0 =
  {
    &i2c_bus_0,
    0x30 >>1,
  };

void dro_delay(int delay_ms)
{
  int i;
  
  for(i=0; i<delay_ms; i++)
    {
    }
}



//////////////////////////////////////////////////////////////////////
//
// Gauge reading loop
//
//

long int value3 = 0;
long int value4 = 20;

long int gauge_calc_24(unsigned char gd[])
{
  long long  x;
  long x1;

  x1 = (gd[0]<<0)+ ((signed long)gd[1]<<8)+(((signed long)gd[2]&0x0f)<<16);

  x = x1;

  //x = x * 127;
  //x = x / 1024;

  if (gd[2] & 0x10)
    {
      x = -x;
    }

  return(x);
}

long int value = 0;
long int value2 = 20;

long int gauge_calc_2x24(unsigned char gd[])
{
  long long  x;
  long x1;

  x1 = (gd[0]<<0)+ ((signed long)gd[1]<<8)+((signed long)gd[2]<<16);

  x = x1;

  x = x * 127;
  x = x / 1024;

  return(x);
}

unsigned char gauge_data[4];
unsigned char gauge1_data[4];

long int gauge_value;

//
// Processes gauge_data and puts a long int into gauge_value
//

void gauge_proc_loop(int display)
{
  long int x, y;

  gauge_value = gauge_calc_24(gauge_data);

  // max7219_write_int(1, y, 1, 2);

  //max7219_write_int(display, x, 1, 2);
  
  // Put some text up
#if 0
  //set_xy(&oled_port, 8, 8);
  display_int(&oled_port, gauge1_data[0], 4);

  //set_xy(&oled_port, 8, 16);
  display_int(&oled_port, gauge1_data[1], 4);

  //set_xy(&oled_port, 8, 24);
  display_int(&oled_port, gauge1_data[2], 4);
  //  display_string(&oled_port, " mm");
#endif

  value+=1;
  value2-=1;
}

//
// Gets a valid sample from a gauge slave
//
// Data is put into gauge_data global

void gauge_loop(I2C_SLAVE_DESC *slave)
{
  int i;

  i = 0;
  while(1)
    {  
      i++;

      if ( i > 30 )
	{
	  gauge_data[0] = 0;
	  gauge_data[1] = 0;
	  gauge_data[2] = 0;

	  break;
	}

      //      i2c_read_bytes(port, 3, &(gauge_data[0]));
      i2c_read_bytes(slave, 4, &(gauge_data[0]));
      if( ((gauge_data[0] + gauge_data[1] + gauge_data[2]) & 0xff) != gauge_data[3] )
	{
	  continue;
	}

      {
#if 0
	if( (gauge_data[0] == 255) &&(gauge_data[0] == 255) &&(gauge_data[0] == 255) )
	  {
	    //  continue;
	  }
	if( (gauge1_data[0] == 255) &&(gauge1_data[0] == 255) &&(gauge1_data[0] == 255) )
	  {
	    continue;
	  }
#endif
      }
      break;
    }
}

/* Main program */
void touch_loop()
{
  unsigned int X;
  unsigned int Y;

  /* Check if the touch screen is currently pressed*/
  // Raw and coordinate values are stored within library at this instant
  // for later retrieval by GetRaw and GetCoord functions.
  // This avoids getting duff values returned
  if (TFT_Touch_Pressed()) // Note this function updates coordinates stored within library variables
  {
    /* Read the current X and Y axis as raw co-ordinates at the last touch time*/
    // The values returned were captured when Pressed() was called!
    
    X = TFT_Touch_X();
    Y = TFT_Touch_Y();

    // Maybe draw pixel
#if 0
    tft_drawPixel(X, Y, 0xffffff);
#endif

 }
}



////////////////////////////////////////////////////////////////////////////
//

int main(void) {
  bool hb_state;
  int restart_count;
  int x, y;
  int demo_x, demo_y, demo_z;
  int demo_delta_x=1, demo_delta_y=1, demo_delta_z=1;

  demo_x = 0;
  demo_y = 0;
  demo_z = 0;

#if defined (__USE_LPCOPEN)
  // Read clock settings and update SystemCoreClock variable
  SystemCoreClockUpdate();
#if !defined(NO_BOARD_LIB)
  // Set up and initialize all required blocks and
  // functions related to the board hardware
  Board_Init();
  // Set the LED to the state of "On"
  //Board_LED_Set(0, true);
#endif
#endif
  
  // TODO: insert code here
  
  // Force the counter to be placed into memory
  volatile static int i = 0, j=0 ;
  
  // Enter an infinite loop, just incrementing a counter

  Chip_GPIO_SetPinDIROutput(LPC_GPIO, 0, 19);
  Chip_GPIO_SetPinDIROutput(LPC_GPIO, 0, 2);
  Chip_GPIO_SetPinDIRInput(LPC_GPIO, 0, 1);
  Chip_GPIO_SetPinDIRInput(LPC_GPIO, 0, 3);
  Chip_GPIO_SetPinDIRInput(LPC_GPIO, 0, 5);
  
  // Set PLL
  *(unsigned int *)0x40048008 = 0x26;   // 
  *(unsigned int *)0x40048070 = 0x03;   // 
  *(unsigned int *)0x40048074 = 0x00;   // 
  *(unsigned int *)0x40048074 = 0x01;   // 

  // Set to GPIO function
  // TFT
  *(unsigned int *)0x400440bc = 0;   //1_23
  *(unsigned int *)0x4004402c = 0;   //0_9
  *(unsigned int *)0x400440b4 = 0;   //1_21
  *(unsigned int *)0x40044054 = 0;   //0_21
  *(unsigned int *)0x4004409c = 0;   //1_15
  *(unsigned int *)0x40044058 = 0;   //0_22
  *(unsigned int *)0x400440c4 = 0;   //1_25
  *(unsigned int *)0x40044040 = 0;   //0_16
  *(unsigned int *)0x40044044 = 0;   //0_17
  *(unsigned int *)0x40044050 = 0;   //0_20
  *(unsigned int *)0x40044018 = 0;   //0_6

  // I2C busses
  *(unsigned int *)0x4004404c = 0;
  *(unsigned int *)0x40044008 = 0;
  *(unsigned int *)0x4004401c = 0;
  *(unsigned int *)0x40044044 = 0;   //0_17
  *(unsigned int *)0x400440ac = 0;
  *(unsigned int *)0x400440b0 = 0;
  *(unsigned int *)0x40044048 = 0;   //0_18

  // MAX 7219
  *(unsigned int *)0x40044094 = 0;   //1_13
  *(unsigned int *)0x40044098 = 0;   //1_14
  *(unsigned int *)0x400440a0 = 0;   //1_16

  // Switch inputs
  *(unsigned int *)0x40044004 = 0;   //0_1
  *(unsigned int *)0x4004400c = 0;   //0_3
  *(unsigned int *)0x40044014 = 0;   //0_5

  // Add a delay here so things can settle down
  dro_delay(100000);
  
  max7219_setup();
  
  int q,r;
  char s1[20], s2[20];
  q = *(unsigned int *)0x4004800c;
  r = *(unsigned int *)0x40048008;
  
  oled_setup(&oled0);
  oled_set_xy(&oled0, 0,0);
  //oled_display_int(&oled0, q, 5);
  //oled_set_xy(&oled0, 0,8);
  //oled_display_int(&oled0, r, 5);
  oled_display_string(&oled0, "DRO Axis 1");
  oled_display_scaled_string_xy(&oled0, " X ", 10, 20, 5);
  
  oled_setup(&oled1);
  oled_set_xy(&oled1, 0,0);
  oled_display_string(&oled1, "DRO Axis 2");
  oled_display_scaled_string_xy(&oled1, " Y ", 10, 20, 5);

  oled_setup(&oled2);
  oled_set_xy(&oled2, 0,0);
  oled_display_string(&oled2, "DRO Axis 3");
  oled_display_scaled_string_xy(&oled2, " Z ", 10, 20, 5);

  tft_init();
  tft_setRotation(0);

  dro_delay(100000);
  tft_fillScreen(ILI9341_WHITE);
  tft_fillScreen(ILI9341_BLACK);

  menu_setup();

  //  tft_init();

  // 0: 24 bit
  // 1: fixed
  // 2: 2x24 bit
  // 3: tmr1

  unsigned char fixed_command[3] = {0x01, 0x00, 0x00};

  //i2c_send_bytes(&gauge2, 3, &(fixed_command[0]));

  // draw coloured pattern
#if 0
  for( x = 0; x< 100; x++)
    {
      for( y = 0; y< 10; y++)
	{
	  tft_drawPixel(x, y, x*y); 
	} 
    }
#endif

  while(1) {

    touch_loop();
    
    process_menu();

   // Read switches
    if( Chip_GPIO_GetPinState(LPC_GPIO, 0, 3) == 0 )
      {
	offset_x = gauge_x;
      }

    if( Chip_GPIO_GetPinState(LPC_GPIO, 0, 5) == 0 )
      {
	offset_y = gauge_y;
      }

    if( Chip_GPIO_GetPinState(LPC_GPIO, 0, 1) == 0 )
      {
	offset_z = gauge_z;
      }

    // read gauges

    //gauge_loop(&gauge0);
    //gauge_loop(&gauge1);
    // Get gauge value now and again
    if ( (i % 2)==0 )
      {

	if ( is_lathe )
	  {
	    gauge_value = 0;
	    gauge_loop(&gauge2);
	    gauge_proc_loop(0);
	    gauge_x = gauge_value;
	    
	    gauge_value = 0;
	    gauge_loop(&gauge3);
	    gauge_proc_loop(1);
	    gauge_y = gauge_value;
	    
	    gauge_value = 0;
	    gauge_loop(&gauge0);
	    gauge_proc_loop(2);
	    gauge_z = gauge_value;
	  }

	if ( is_mill )
	  {
	    gauge_value = 0;
	    gauge_loop(&gauge1);
	    gauge_proc_loop(0);
	    gauge_x = gauge_value;
	    
	    gauge_value = 0;
	    gauge_loop(&gauge4);
	    gauge_proc_loop(1);
	    gauge_y = gauge_value;
	    
	    gauge_value = 0;
	    gauge_loop(&gauge5);
	    gauge_proc_loop(2);
	    gauge_z = gauge_value;
	  }

	if( demo_mode )
	  {
	    demo_x += demo_delta_x;
	    if( demo_x > 1000 )
	      {
		demo_delta_x *= -1;
	      }
	    if( demo_x < -1000 )
	      {
		demo_delta_x *= -1;
	      }

	    demo_y += demo_delta_y;

	    if( demo_y > 1000 )
	      {
		demo_delta_y *= -1;
	      }

	    if( demo_y < -1000 )
	      {
		demo_delta_y *= -1;
	      }

	    demo_z += demo_delta_z;

	    if( demo_z > 10000 )
	      {
		demo_delta_z *= -1;
	      }

	    if( demo_z < -10000 )
	      {
		demo_delta_z *= -1;
	      }

	    gauge_x += demo_x;
	    gauge_y += demo_y/10;
	    gauge_z += demo_z/100;
	  }

	// Display all values
#if 1
	switch(sum_mode_x)
	  {
	  case SUM_MODE_X_X:
	    max7219_write_int(0, gauge_x-offset_x, 1, 2);
	    break;

	  case SUM_MODE_X_XY:
	    max7219_write_int(0, gauge_x-offset_x+gauge_y-offset_y, 1, 2);
	    break;

	  case SUM_MODE_X_XZ:
	    max7219_write_int(0, gauge_x-offset_x+gauge_z-offset_z, 1, 2);
	    break;
	  }

	switch(sum_mode_y)
	  {
	  case SUM_MODE_Y_Y:
	    max7219_write_int(1, gauge_y-offset_y, 1, 2);
	    break;

	  case SUM_MODE_Y_YZ:
	    max7219_write_int(1, gauge_y-offset_y+gauge_z-offset_z, 1, 2);
	    break;

	  }

	max7219_write_int(2, gauge_z-offset_z, 1, 2);
#else
	max7219_write_int(0, 100, 1, 2);
	max7219_write_int(1, 200, 1, 2);
	max7219_write_int(2, 300, 1, 2);
#endif

      }

    //max7219_write_int(0,  j/5, 0, 4);
    //max7219_write_int(1,  j/3, 0, 2);
    //max7219_write_int(2,  j, 0, 3);

    // Every so often, reset the LED displays
    restart_count++;

    if( restart_count > 300 )
      {
	//i2c_send_bytes(&gauge2, 3, &(fixed_command[0]));
	max7219_setup();
	restart_count = 0;
      }

    //tft_init();

    i++ ;
    j++;
#if 0
    max7219_write_int(0,  j/5, 0, 4);
    max7219_write_int(1,  j/3, 0, 2);
    max7219_write_int(2,  j, 0, 3);
#endif
    if( i> 20)
      {
	i=0;
	hb_state = !hb_state;
#if 1
	// Display heartbeat and also Lathe/Mill setting character
	oled_set_xy(&oled2, 0,59);
	oled_display_string(&oled2, hb_state?"X":"+");
	oled_set_xy(&oled2, 8,59);
	oled_display_string(&oled2, is_lathe?"L":"");
	oled_set_xy(&oled2, 8,59);
	oled_display_string(&oled2, is_mill?"M":"");
#endif
	//	    Board_LED_Set(0, hb_state);

      }
  }
  return 0 ;
}
