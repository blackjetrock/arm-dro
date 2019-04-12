/*
 * tft_il9341.c
 *
 *  Created on: 24 Jul 2016
 *      Author: menadue
 */
#include "chip.h"
#include "board.h"

#include "i2c_functions.h"
#include "tft_il9341.h"

// We use the OLED module font
#include "oled096.h"

#define _RAWERR 10

int rotation;

int _height, _width;
uint8_t  tabcolor;

int16_t TFT_Touch_ReadAxis(int Axis);
uint16_t TFT_Touch_ReadData(void);

/*
 * TFT pinout:
 * CS        : 1_23 
 * RESET     : 0_12
 * DC        : 0_13
 * MOSI      : 0_21
 * SCK       : 1_15
 * MISO      : 0_22
 * TCLK      : 0_14
 * TCS       : 0_16
 * TDIN      : 0_17
 * TDO       : 0_20
 * TIRQ      : 0_6
 *
 */

#define CS_PIN    1, 23
#define RESET_PIN 0, 9
#define DC_PIN    1, 21
#define MOSI_PIN  0, 21
#define SCK_PIN   1, 15
#define MISO_PIN  0, 22
#define TCLK_PIN  1, 25
#define TCS_PIN   0, 16
#define TDIN_PIN  0, 17
#define TDO_PIN   0, 20
#define TIRQ_PIN  0, 6

int _rst = 0;

uint16_t touch_xcoord, touch_ycoord;
uint16_t touch_xraw, touch_yraw;
uint16_t touch_hmin;
uint16_t touch_hmax;
uint16_t touch_vmin;
uint16_t touch_vmax;
uint16_t touch_hres;
uint16_t touch_vres;
int touch_xflip;
int touch_yflip;
int touch_xyswap;
int touch_axis;

#define HMIN 0
#define HMAX 4095
#define VMIN 0
#define VMAX 4095
#define XYSWAP 0 // 0 or 1

int abs(int x)
{
  if( x < 0 )
    return(-x);

  return(x);
}


long map(long x, long in_min, long in_max, long out_min, long out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

int constrain(int x, int a, int b)
{
  if( x < a)
    {
      return(a);
    }

  if( x > b )
    {
      return(b);
    }

  return(x);

}

void tft_delay(int time)
{
  int i;

  for(i=0; i<time; i++)
    {
    }

}

void tft_init(void)
{
  // Screen
  Chip_GPIO_SetPinDIROutput(LPC_GPIO, CS_PIN);
  Chip_GPIO_SetPinDIROutput(LPC_GPIO, RESET_PIN);
  Chip_GPIO_SetPinDIROutput(LPC_GPIO, DC_PIN);
  Chip_GPIO_SetPinDIROutput(LPC_GPIO, SCK_PIN);
  Chip_GPIO_SetPinDIROutput(LPC_GPIO, MOSI_PIN);
  Chip_GPIO_SetPinDIRInput( LPC_GPIO, MISO_PIN);

  // Touch controller
  // pinMode(_CS, OUTPUT); 
  //pinMode(_Clk, OUTPUT); 
  //pinMode(_Din, OUTPUT); 
  Chip_GPIO_SetPinDIROutput(LPC_GPIO, TDIN_PIN);
  Chip_GPIO_SetPinDIROutput(LPC_GPIO, TCLK_PIN);
  Chip_GPIO_SetPinDIROutput(LPC_GPIO, TCS_PIN);
  Chip_GPIO_SetPinDIRInput( LPC_GPIO, TDO_PIN);
  //  pinMode(_Dout, INPUT); 
  
  Chip_GPIO_SetPinState(LPC_GPIO, TCS_PIN, IL_HIGH);
  Chip_GPIO_SetPinState(LPC_GPIO, TCLK_PIN, IL_LOW);
  Chip_GPIO_SetPinState(LPC_GPIO, TDIN_PIN, IL_LOW);
  //digitalWrite(_CS, HIGH);  
  //digitalWrite(_Clk, LOW);
  //digitalWrite(_Din, LOW);

  touch_hmin = 0;
  touch_hmax = 4095;
  touch_vmin = 0;
  touch_vmax = 4095;
  touch_hres = 240;
  touch_vres = 320;

  touch_xyswap = 1;
  touch_xflip = 1;
  touch_yflip = 1;

  touch_axis = 0;

  tft_begin();

  TFT_Touch_setCal(HMIN+30, HMAX-30, VMIN+30, VMAX-30, touch_hres, touch_vres, XYSWAP); // Raw xmin, xmax, ymin, ymax, width, height
  //TFT_Touch_setRotation(1);

}

void tft_spiwrite(unsigned char c)
{
  int bit;
 
  for(bit = 0x80; bit; bit >>= 1)
    {
      if(c & bit)
	{
	  Chip_GPIO_SetPinState(LPC_GPIO, MOSI_PIN, IL_HIGH);
	  //	  Chip_GPIO_SetPinState(LPC_GPIO, MOSI_PIN, IL_HIGH);
	  //digitalWrite(_mosi, HIGH);
	} 
      else
	{
	  //Chip_GPIO_SetPinState(LPC_GPIO, MOSI_PIN, IL_LOW);
	  Chip_GPIO_SetPinState(LPC_GPIO, MOSI_PIN, IL_LOW);
	  //digitalWrite(_mosi, LOW);
	}

      //Chip_GPIO_SetPinState(LPC_GPIO, SCK_PIN, IL_HIGH);
      Chip_GPIO_SetPinState(LPC_GPIO, SCK_PIN, IL_HIGH);
      //Chip_GPIO_SetPinState(LPC_GPIO, SCK_PIN, IL_LOW);
      Chip_GPIO_SetPinState(LPC_GPIO, SCK_PIN, IL_LOW);
      //digitalWrite(_sclk, HIGH);
      //digitalWrite(_sclk, IL_LOW);
    }
}


void tft_writecommand(unsigned char c)
{
  Chip_GPIO_SetPinState(LPC_GPIO, DC_PIN, IL_LOW);
  Chip_GPIO_SetPinState(LPC_GPIO, SCK_PIN, IL_LOW);
  Chip_GPIO_SetPinState(LPC_GPIO, CS_PIN, IL_LOW);
  //digitalWrite(_dc, IL_LOW);
  //digitalWrite(_sclk, IL_LOW);
  //digitalWrite(_cs, IL_LOW);

  tft_spiwrite(c);

  Chip_GPIO_SetPinState(LPC_GPIO, CS_PIN, IL_HIGH);
  //digitalWrite(_cs, HIGH);
}


void tft_writedata(unsigned char c)
{
  Chip_GPIO_SetPinState(LPC_GPIO, DC_PIN, IL_HIGH);
  // ?????  Chip_GPIO_SetPinState(LPC_GPIO, SCK_PIN, IL_LOW);
  Chip_GPIO_SetPinState(LPC_GPIO, CS_PIN, IL_LOW);

  //  digitalWrite(_dc, HIGH);
  //digitalWrite(_cs, IL_LOW);

  tft_spiwrite(c);

  Chip_GPIO_SetPinState(LPC_GPIO, CS_PIN, IL_HIGH);
  //digitalWrite(_cs, HIGH);
}


// Rather than a bazillion writecommand() and writedata() calls, screen
// initialization commands and arguments are organized in these tables
// stored in PROGMEM.  The table may look bulky, but that's mostly the
// formatting -- storage-wise this is hundreds of bytes more compact
// than the equivalent code.  Companion function follows.
#define DELAY 0x80


#if 0
// Companion code to the above tables.  Reads and issues
// a series of LCD commands stored in PROGMEM byte array.
void tft_commandList(unsigned char *addr)
{
  
  unsigned char  numCommands, numArgs;
  int ms;
  
  numCommands = pgm_read_byte(addr++);   // Number of commands to follow

  while(numCommands--)
    {                 // For each command...
      tft_writecommand(pgm_read_byte(addr++)); //   Read, issue command
      numArgs  = pgm_read_byte(addr++);    //   Number of args to follow
      ms       = numArgs & DELAY;          //   If hibit set, delay follows args
      numArgs &= ~DELAY;                   //   Mask out delay bit

      while(numArgs--)
	{                   //   For each argument...
	  tft_writedata(pgm_read_byte(addr++));  //     Read, issue argument
	}
      
      if(ms)
	{
	  ms = pgm_read_byte(addr++); // Read post-command delay time (ms)
	  if(ms == 255)
	    { 
	    ms = 500;     // If 255, delay for 500 ms
	    }
	  for(int i=0; i<1000; i++)
	  {
	  }
	  }
	}
    }
}
#endif

void tft_begin(void)
{

  if (_rst > 0)
    {
      Chip_GPIO_SetPinState(LPC_GPIO, RESET_PIN, IL_LOW);
      //      pinMode(_rst, OUTPUT);
      //digitalWrite(_rst, IL_LOW);
    }

  //  pinMode(_dc, OUTPUT);
  //pinMode(_cs, OUTPUT);

  // toggle RST low to reset
  if (_rst > 0) {
    //Chip_GPIO_SetPinState(LPC_GPIO, RESET_PIN, HIGH);
    //    digitalWrite(_rst, HIGH);
    //delay(5);
    //digitalWrite(_rst, IL_LOW);
    //delay(20);
    //digitalWrite(_rst, HIGH);
    //delay(150);
  }

  /*
  uint8_t x = readcommand8(ILI9341_RDMODE);
  Serial.print("\nDisplay Power Mode: 0x"); Serial.println(x, HEX);
  x = readcommand8(ILI9341_RDMADCTL);
  Serial.print("\nMADCTL Mode: 0x"); Serial.println(x, HEX);
  x = readcommand8(ILI9341_RDPIXFMT);
  Serial.print("\nPixel Format: 0x"); Serial.println(x, HEX);
  x = readcommand8(ILI9341_RDIMGFMT);
  Serial.print("\nImage Format: 0x"); Serial.println(x, HEX);
  x = readcommand8(ILI9341_RDSELFDIAG);
  Serial.print("\nSelf Diagnostic: 0x"); Serial.println(x, HEX);
*/
  //if(cmdList) commandList(cmdList);
  Chip_GPIO_SetPinState(LPC_GPIO, RESET_PIN, IL_HIGH);

  tft_writecommand(0xEF);
  tft_writedata(0x03);
  tft_writedata(0x80);
  tft_writedata(0x02);

  tft_writecommand(0xCF);
  tft_writedata(0x00);
  tft_writedata(0XC1);
  tft_writedata(0X30);

  tft_writecommand(0xED);
  tft_writedata(0x64);
  tft_writedata(0x03);
  tft_writedata(0X12);
  tft_writedata(0X81);

  tft_writecommand(0xE8);
  tft_writedata(0x85);
  tft_writedata(0x00);
  tft_writedata(0x78);

  tft_writecommand(0xCB);
  tft_writedata(0x39);
  tft_writedata(0x2C);
  tft_writedata(0x00);
  tft_writedata(0x34);
  tft_writedata(0x02);

  tft_writecommand(0xF7);
  tft_writedata(0x20);

  tft_writecommand(0xEA);
  tft_writedata(0x00);
  tft_writedata(0x00);

  tft_writecommand(ILI9341_PWCTR1);    //Power control
  tft_writedata(0x23);   //VRH[5:0]

  tft_writecommand(ILI9341_PWCTR2);    //Power control
  tft_writedata(0x10);   //SAP[2:0];BT[3:0]

  tft_writecommand(ILI9341_VMCTR1);    //VCM control
  tft_writedata(0x3e); //¶Ô±È¶Èµ÷½Ú
  tft_writedata(0x28);

  tft_writecommand(ILI9341_VMCTR2);    //VCM control2
  tft_writedata(0x86);  //--

  tft_writecommand(ILI9341_MADCTL);    // Memory Access Control
  tft_writedata(0x48);

  tft_writecommand(ILI9341_PIXFMT);
  tft_writedata(0x55);

  tft_writecommand(ILI9341_FRMCTR1);
  tft_writedata(0x00);
  tft_writedata(0x18);

  tft_writecommand(ILI9341_DFUNCTR);    // Display Function Control
  tft_writedata(0x08);
  tft_writedata(0x82);
  tft_writedata(0x27);

  tft_writecommand(0xF2);    // 3Gamma Function Disable
  tft_writedata(0x00);

  tft_writecommand(ILI9341_GAMMASET);    //Gamma curve selected
  tft_writedata(0x01);

  tft_writecommand(ILI9341_GMCTRP1);    //Set Gamma
  tft_writedata(0x0F);
  tft_writedata(0x31);
  tft_writedata(0x2B);
  tft_writedata(0x0C);
  tft_writedata(0x0E);
  tft_writedata(0x08);
  tft_writedata(0x4E);
  tft_writedata(0xF1);
  tft_writedata(0x37);
  tft_writedata(0x07);
  tft_writedata(0x10);
  tft_writedata(0x03);
  tft_writedata(0x0E);
  tft_writedata(0x09);
  tft_writedata(0x00);

  tft_writecommand(ILI9341_GMCTRN1);    //Set Gamma
  tft_writedata(0x00);
  tft_writedata(0x0E);
  tft_writedata(0x14);
  tft_writedata(0x03);
  tft_writedata(0x11);
  tft_writedata(0x07);
  tft_writedata(0x31);
  tft_writedata(0xC1);
  tft_writedata(0x48);
  tft_writedata(0x08);
  tft_writedata(0x0F);
  tft_writedata(0x0C);
  tft_writedata(0x31);
  tft_writedata(0x36);
  tft_writedata(0x0F);

  tft_writecommand(ILI9341_SLPOUT);    //Exit Sleep

  tft_writecommand(ILI9341_DISPON);    //Display on

}


void tft_setAddrWindow(uint16_t x0, uint16_t y0, uint16_t x1,
 uint16_t y1) 
{

  tft_writecommand(ILI9341_CASET); // Column addr set
  tft_writedata(x0 >> 8);
  tft_writedata(x0 & 0xFF);     // XSTART
  tft_writedata(x1 >> 8);
  tft_writedata(x1 & 0xFF);     // XEND

  tft_writecommand(ILI9341_PASET); // Row addr set
  tft_writedata(y0>>8);
  tft_writedata(y0);     // YSTART
  tft_writedata(y1>>8);
  tft_writedata(y1);     // YEND

  tft_writecommand(ILI9341_RAMWR); // write to RAM
}


void tft_pushColor(uint16_t color) 
{
  Chip_GPIO_SetPinState(LPC_GPIO, DC_PIN, IL_HIGH);
  Chip_GPIO_SetPinState(LPC_GPIO, CS_PIN, IL_LOW);
  //digitalWrite(_dc, HIGH);
  //digitalWrite(_cs, IL_LOW);

  tft_spiwrite(color >> 8);
  tft_spiwrite(color);

  Chip_GPIO_SetPinState(LPC_GPIO, CS_PIN, IL_HIGH);
  //digitalWrite(_cs, HIGH);
}

void tft_drawPixel(int16_t y, int16_t x, uint16_t color) 
{
  y = _height - y;

  // if((x < 0) ||(x >= _width) || (y < 0) || (y >= _height))
  //{
  //  return;
  //}

  tft_setAddrWindow(x,y,x+1,y+1);

  Chip_GPIO_SetPinState(LPC_GPIO, DC_PIN, IL_HIGH);
  Chip_GPIO_SetPinState(LPC_GPIO, CS_PIN, IL_LOW);

  // digitalWrite(_dc, HIGH);
  //digitalWrite(_cs, IL_LOW);

  tft_spiwrite(color >> 8);
  tft_spiwrite(color);

  Chip_GPIO_SetPinState(LPC_GPIO, CS_PIN, IL_HIGH);
  //digitalWrite(_cs, HIGH);

}


void tft_drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color)
{
  
  // Rudimentary clipping
  if((x >= _width) || (y >= _height))
    {
      return;
    }

  if((y+h-1) >= _height)
    {
    h = _height-y;
    }

  tft_setAddrWindow(x, y, x, y+h-1);

  uint8_t hi = color >> 8, lo = color;

  Chip_GPIO_SetPinState(LPC_GPIO, DC_PIN, IL_HIGH);
  Chip_GPIO_SetPinState(LPC_GPIO, CS_PIN, IL_LOW);

  // digitalWrite(_dc, HIGH);
  //digitalWrite(_cs, IL_LOW);

  while (h--) {
    tft_spiwrite(hi);
    tft_spiwrite(lo);
  }

  Chip_GPIO_SetPinState(LPC_GPIO, CS_PIN, IL_HIGH);
  //digitalWrite(_cs, HIGH);

}


void tft_drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color)
{
  
  // Rudimentary clipping
  if((x >= _width) || (y >= _height))
    {
      return;
    }

  if((x+w-1) >= _width)
    {
      w = _width-x;
    }

  tft_setAddrWindow(x, y, x+w-1, y);

  uint8_t hi = color >> 8, lo = color;

  //digitalWrite(_dc, HIGH);
  //digitalWrite(_cs, IL_LOW);

  while (w--) {
    tft_spiwrite(hi);
    tft_spiwrite(lo);
  }

  Chip_GPIO_SetPinState(LPC_GPIO, CS_PIN, IL_HIGH);
  //digitalWrite(_cs, HIGH);

}

void tft_drawBox(int x, int y, int w, int h, int color)
{
  int i;

  for(i = x; i <= x+w; i++)
    {
      tft_drawPixel(i, y, color);
      tft_drawPixel(i, y+h, color);
    }

  for(i = y; i <= y+h; i++)
    {
      tft_drawPixel(x, i, color);
      tft_drawPixel(x+w, i, color);
    }


}



void tft_drawString(int x, int y, char *string, int color)
{
  char c;
  unsigned char *pattern;
  int xb, yb;
  int bitno;
  int s;
  int ys = y;
  int x2;

  while( (c = *(string++)) != '\0' )
    {
      pattern = &font_5x7_letters[(c-' ')*5];
     
      for(xb = 0; xb < 5; xb++)
	{ 
	  for(x2 =0 ; x2<TSCALE; x2++)
	    {
	      y = ys;
	      
	      for(bitno = 0, yb = 0; yb<7; yb++, bitno++)
		{
		  for(s=0; s<TSCALE; s++)
		    {
		      if( *pattern & (1<<bitno) )
			{
			      tft_drawPixel(x, y, color);
			}
		      else
			{
			  //if ( color == ILI9341_BLACK )
			  //  {
			  //    tft_drawPixel(x, y, ILI9341_BLACK);
			  //  }
			}
		      y++;
		    }
		}	  
	      x++;
	    }
	  pattern++;
	  //x++;
	}
      
      x++;
    }
  
}

void tft_fillScreen(uint16_t color)
{
  tft_fillRect(0, 0,  _width, _height, color);
}

// fill a rectangle
void tft_fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
{
  
  // rudimentary clipping (drawChar w/big text requires this)
  if((x >= _width) || (y >= _height))
    {
 return;
    }

  if((x + w - 1) >= _width)
    {
      w = _width  - x;
    }

  if((y + h - 1) >= _height)
    {
      h = _height - y;
    }

  tft_setAddrWindow(x, y, x+w-1, y+h-1);

  uint8_t hi = color >> 8, lo = color;

  Chip_GPIO_SetPinState(LPC_GPIO, DC_PIN, IL_HIGH);
  Chip_GPIO_SetPinState(LPC_GPIO, CS_PIN, IL_LOW);

  //digitalWrite(_dc, HIGH);
  //digitalWrite(_cs, IL_LOW );

  for(y=h; y>0; y--)
    {
      for(x=w; x>0; x--)
	{
	  tft_spiwrite(hi);
	  tft_spiwrite(lo);
	}
    }

  Chip_GPIO_SetPinState(LPC_GPIO, CS_PIN, IL_HIGH);
  //digitalWrite(_cs, HIGH);
}


// Pass 8-bit (each) R,G,B, get back 16-bit packed color
uint16_t tft_color565(uint8_t r, uint8_t g, uint8_t b)
{
  return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}


#define MADCTL_MY  0x80
#define MADCTL_MX  0x40
#define MADCTL_MV  0x20
#define MADCTL_ML  0x10
#define MADCTL_RGB 0x00
#define MADCTL_BGR 0x08
#define MADCTL_MH  0x04

void tft_setRotation(uint8_t m) {

  tft_writecommand(ILI9341_MADCTL);
  
  rotation = m % 4; // can't be higher than 3
  
  switch (rotation) 
    {
    case 0:
      tft_writedata(MADCTL_MX | MADCTL_BGR);
      _width  = ILI9341_TFTWIDTH;
      _height = ILI9341_TFTHEIGHT;
      break;

    case 1:
      tft_writedata(MADCTL_MV | MADCTL_BGR);
      _width  = ILI9341_TFTHEIGHT;
      _height = ILI9341_TFTWIDTH;
      break;

    case 2:
      tft_writedata(MADCTL_MY | MADCTL_BGR);
     _width  = ILI9341_TFTWIDTH;
     _height = ILI9341_TFTHEIGHT;
    break;

    case 3:
      tft_writedata(MADCTL_MX | MADCTL_MY | MADCTL_MV | MADCTL_BGR);
      _width  = ILI9341_TFTHEIGHT;
      _height = ILI9341_TFTWIDTH;
      break;
    }
}


void tft_invertDisplay(int i)
{
  tft_writecommand(i ? ILI9341_INVON : ILI9341_INVOFF);
}


////////////////////////////////////////////////////////////////////////////////
//
// Touch screen
//
//

/* Detects if the touch screen is currently pressed. Returns TRUE if pressed */
int TFT_Touch_Pressed(void)
{

  // Get the raw contact coordinates
  touch_xraw = TFT_Touch_ReadAxis(touch_axis);
  touch_yraw = TFT_Touch_ReadAxis(!touch_axis);

  // Now double check the touch is still near the initial contact point
  // This helps to debounce the touch contact
  // We are working with signed integers intentionally
  tft_delay(1);
  if (abs(touch_xraw - TFT_Touch_ReadAxis(touch_axis)) > _RAWERR)
    {
      return false;
    }

  if (abs(touch_yraw - TFT_Touch_ReadAxis(!touch_axis)) > _RAWERR)
    {
      return false;
    }

  tft_delay(1);

  if (abs(touch_xraw - TFT_Touch_ReadAxis(touch_axis)) > _RAWERR)
    {
      return false;
    }

  if (abs(touch_yraw - TFT_Touch_ReadAxis(!touch_axis)) > _RAWERR)
    {
      return false;
    }

/*
  // Get the raw contact coordinates, temporary test to emulate a flip and rotation
  _xraw = _ReadAxis(!_axis);
  _yraw = 4095 - _ReadAxis(_axis);

  // Now double check the touch is still near the initial contact point
  // This helps to debounce the touch contact
  // We are working with signed integers intentionally
  delay(1);
  if (abs(_xraw - _ReadAxis(!_axis)) > _RAWERR) return false;
  if (abs(_yraw - (4095 - _ReadAxis(_axis))) > _RAWERR) return false;
  delay(1);
  if (abs(_xraw - _ReadAxis(!_axis)) > _RAWERR) return false;
  if (abs(_yraw - (4095 - _ReadAxis(_axis))) > _RAWERR) return false;
*/

  // Check values are in calibration range
  //if((_yraw > _vmin && _yraw < _vmax) && (_xraw > _hmin && _xraw < _hmax))
  if((touch_yraw > 0 && touch_yraw < 4095) && 
     (touch_xraw > 0 && touch_xraw < 4095))
  {
    if (touch_xyswap)
      {
      touch_xcoord = constrain(map(touch_yraw, touch_vmin, touch_vmax, 0, touch_vres), 0, touch_vres);
      touch_ycoord = constrain(map(touch_xraw, touch_hmin, touch_hmax, 0, touch_hres), 0, touch_hres);
      if(touch_xflip)
	{
	  touch_xcoord = touch_vres - touch_xcoord;
	}

      if(touch_yflip)
	{
	  touch_ycoord = touch_hres - touch_ycoord;
	}
    }
    else
    {
      touch_xcoord = constrain(map(touch_xraw, touch_hmin, touch_hmax, 0, touch_hres), 0, touch_hres);
      touch_ycoord = constrain(map(touch_yraw, touch_vmin, touch_vmax, 0, touch_vres), 0, touch_vres);
      if(touch_xflip)
	{
	  touch_xcoord = touch_hres - touch_xcoord;
	}

      if(touch_yflip)
	{
	  touch_ycoord = touch_vres - touch_ycoord;
	}
    }
    
    return true; 
  }
  else
    {
      return false;
    } 
}

/* Reads one of the axis (XAXIS or YAXIS) raw coordinates. */
int16_t TFT_Touch_ReadAxis(int Axis)
{
  int Data;

  Chip_GPIO_SetPinState(LPC_GPIO, TCS_PIN, IL_LOW);
  //  digitalWrite(_CS, LOW);
  if (Axis)
    {
      TFT_Touch_OutputData(0x90);   
    }
  else
    {
      TFT_Touch_OutputData(0xD0);
    }
  
  Chip_GPIO_SetPinState(LPC_GPIO, TCLK_PIN, IL_HIGH);
  Chip_GPIO_SetPinState(LPC_GPIO, TCLK_PIN, IL_LOW);
  //  digitalWrite(_Clk, HIGH); digitalWrite(_Clk, LOW); //_PulseClock();

  Data = TFT_Touch_ReadData();

  Chip_GPIO_SetPinState(LPC_GPIO, TCLK_PIN, IL_HIGH);
  Chip_GPIO_SetPinState(LPC_GPIO, TCLK_PIN, IL_LOW);
  Chip_GPIO_SetPinState(LPC_GPIO, TCLK_PIN, IL_HIGH);
  Chip_GPIO_SetPinState(LPC_GPIO, TCLK_PIN, IL_LOW);
  Chip_GPIO_SetPinState(LPC_GPIO, TCLK_PIN, IL_HIGH);
  Chip_GPIO_SetPinState(LPC_GPIO, TCLK_PIN, IL_LOW);

  //  digitalWrite(_Clk, HIGH); digitalWrite(_Clk, LOW); //_PulseClock();
  //digitalWrite(_Clk, HIGH); digitalWrite(_Clk, LOW); //_PulseClock();
  //digitalWrite(_Clk, HIGH); digitalWrite(_Clk, LOW); //_PulseClock();


  Chip_GPIO_SetPinState(LPC_GPIO, TCS_PIN, IL_HIGH);
  //digitalWrite(_CS, HIGH); 
  Chip_GPIO_SetPinState(LPC_GPIO, TDIN_PIN, IL_LOW);
  //digitalWrite(_Din, LOW);
  
  return Data;
}

/* Reads the raw data from the touch screen */
uint16_t TFT_Touch_ReadData(void)
{
  int index;
  uint16_t Data;
  int bit;

  Data = 0;

  for (index = 12; index > 0; index--)
    {
      bit = Chip_GPIO_GetPinState(LPC_GPIO, TDO_PIN);
      Data += bit << (index-1);
      Chip_GPIO_SetPinState(LPC_GPIO, TCLK_PIN, IL_HIGH);
      Chip_GPIO_SetPinState(LPC_GPIO, TCLK_PIN, IL_LOW);
      //digitalWrite(_Clk, HIGH); digitalWrite(_Clk, LOW); //_PulseClock();
    }
  return Data;
}  

/* Writes to the touch screen's configuration register */
void TFT_Touch_OutputData(int Data)
{
  int index;
  
  for (index = 8; index > 0; index--)
  {
    Chip_GPIO_SetPinState(LPC_GPIO, TDIN_PIN, (Data >> (index -1)) & 1);
    Chip_GPIO_SetPinState(LPC_GPIO, TCLK_PIN, IL_HIGH);
    Chip_GPIO_SetPinState(LPC_GPIO, TCLK_PIN, IL_LOW);
    //    digitalWrite(_Din, (Data >> (index -1)) & 1);
    //digitalWrite(_Clk, HIGH); digitalWrite(_Clk, LOW); //_PulseClock();
  }
}

/* Read the current position from the touch screen and return it as a pixel position. */
uint16_t TFT_Touch__ReadRawX(void)
{
  return TFT_Touch_ReadAxis(touch_axis);;
}

/* Read the current position from the touch screen and return it as a pixel position. */
uint16_t TFT_Touch_ReadRawY(void)
{
  return TFT_Touch_ReadAxis(!touch_axis);;
}

/* Read the last position from the touch screen and return it as a pixel position. */
uint16_t TFT_Touch_RawX(void)
{
  return touch_xraw;
}

/* Read the last position from the touch screen and return it as a pixel position. */
uint16_t TFT_Touch_RawY(void)
{
  return touch_yraw;
}

/* Read the last position from the touch screen and return it as a pixel position. */
uint16_t TFT_Touch_X(void)
{
  return touch_xcoord;
}

/* Read the last position from the touch screen and return it as a pixel position. */
uint16_t TFT_Touch_Y(void)
{
  return touch_ycoord;
}

/* Read the last zone from the touch screen and return it as a pixel position. */
int TFT_Touch_Zone(void)
{
  return touch_xcoord + (uint32_t)touch_ycoord *touch_hres;
}

/* Set the screen resolution in pixels. */
void TFT_Touch_setResolution(uint16_t hres, uint16_t vres)
{
  touch_hres = hres;
  touch_vres = vres;
}

/* Set the screen calibration values */
void TFT_Touch_setCal(uint16_t hmin, uint16_t hmax,
                       uint16_t vmin, uint16_t vmax,
                       uint16_t hres, uint16_t vres,
                       int axis)//, bool xflip, bool yflip)
{
  touch_hmin = hmin;
  touch_hmax = hmax;
  touch_vmin = vmin;
  touch_vmax = vmax;
  touch_hres = hres;
  touch_vres = vres;

  touch_axis   = axis;
  //_Xflip  = xflip;
  //_Yflip  = yflip;
}

int16_t TFT_Touch_ReadCal(int param)
{
  switch(param) {
  case 1:
    return touch_hmin;
    break;
  case 2:
    return touch_hmax;
    break;
  case 3:
    return touch_vmin;
    break;
  case 4:
    return touch_vmax;
    break;
  case 5:
    return touch_hres;
    break;
  case 6:
    return touch_vres;
    break;
  case 7:
    return touch_axis;
    break;
    //case 8:
    //  return _xflip;
    //  break;
    //case 9:
    //  return _yflip;
    //  break;
  }
}

void TFT_Touch_setRotation(int rotation)
{
  switch(rotation) {
  case 1:
    touch_xyswap  = 0;
    touch_xflip = 0;//_Xflip;
    touch_yflip = 0;//_Yflip;
    break;

  case 2:
    touch_xyswap  = 1;
    touch_xflip = 0;//_Xflip;
    touch_yflip = 1;//!_Yflip;
    break;

  case 3:
      touch_xyswap  = 0;
      touch_xflip = 1;//!_Xflip;
      touch_yflip = 1;//!_Yflip;
      break;
      
  case 0:
    touch_xyswap  = 1;
    touch_xflip = 0;//!_Xflip;
    touch_yflip = 0;//_Yflip;
    break;
  }
}
