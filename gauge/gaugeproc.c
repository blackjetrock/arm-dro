/*
 * File:   newmain.c
 * Author: menadue
 *
 * Created on 07 June 2016, 07:13
 */

#define DEMO 0
#define I2C_slave_address 0x30

#include <xc.h>
#include <pic12f1822.h>

//__PROGICONFIG (1,0x3F84);
//__PROGICONFIG (2,0x3FFF);

// CONFIG1
#pragma config FOSC = INTOSC   // INTOSC oscillator: I/O function on CLKIN pin
#pragma config WDTE = OFF       // Watchdog Timer Enable (WDT enabled)
#pragma config PWRTE = ON       // Power-up Timer Enable (PWRT enabled)
#pragma config MCLRE = OFF      // MCLR/VPP pin function is GPIO
#pragma config CP = ON          // Flash Program Memory Code Protection enabled
#pragma config CPD = ON         // Data memory code protection is enabled
#pragma config BOREN = ON       // Brown-out Reset enabled
#pragma config CLKOUTEN = OFF   // CLKOUT function is disabled.
                                // I/O or oscillator function on the CLKOUT pin
#pragma config IESO = ON        // Internal/External Switch-over mode is enabled
#pragma config FCMEN = ON       // Fail-Safe Clock Monitor is enabled

// CONFIG2
#pragma config WRT = OFF        // Flash Memory Self-Write Protection off
//#pragma config VCAPEN = OFF     // All VCAP pin functionality is disabled
#pragma config PLLEN = ON       // PLL Enable (4x PLL enabled)
#pragma config STVREN = ON      // Stack Overflow/Underflow will cause a Reset
#pragma config BORV = LO        // Brown-out Reset Voltage (Vbor),
                                // low trip point selected.
#pragma config LVP = OFF         // Low-voltage programming enabled

#define RX_ELMNTS	6

// array for master to read from
volatile unsigned char I2C_Array_Tx[RX_ELMNTS] =
  {

    0x01,0x02,0x03,0x04};

// array for master to write to
volatile unsigned char I2C_Array_Rx[RX_ELMNTS] = 0xcd;	

unsigned char index_i2c = 0;     // index pointer
unsigned char junk = 0;          // used to place unnecessary data
unsigned char clear = 0x00;      // used to clear array location once
                                 // it has been read

void initialize(void);           // initialize routine

/**************************** MAIN ROUTINE ************************************/
#define SWAPPED 0

#if SWAPPED
#define GAUGEDATA     0b00000001
#define GAUGECLK      0b00001000
#define GAUGECLK_DRV  0b00100000
#define GAUGEDATA_DRV 0b00010000
#else
#define GAUGECLK      0b00000001
#define GAUGEDATA     0b00001000
#define GAUGEDATA_DRV 0b00100000
#define GAUGECLK_DRV  0b00010000
#endif
typedef enum _GAUGETYPE
  {
    GAUGE_TYPE_24_BIT=0,
    GAUGE_TYPE_FIXED=1,
    GAUGE_TYPE_2X24_BIT=2,
    GAUGE_TYPE_TMR1=3,
  } GAUGETYPE;

//int gauge_type = GAUGE_TYPE_TMR1;
int gauge_type = GAUGE_TYPE_24_BIT;

void gauge_type_proc_clock(void)
{
  int tmrval;

  tmrval = TMR1;

  // Reads timer
  I2C_Array_Tx[0] = tmrval & 0xff;;
  I2C_Array_Tx[1] = tmrval >> 8;;
  I2C_Array_Tx[2] = 0xda;
  I2C_Array_Tx[3] = gauge_type & 0xff;

}

// Standard machine-dro gauge two 24 bit data packets, we want the second one

unsigned char gb[48];
unsigned char gaugedata[6];

void gauge_type_proc_2x24_bit(void)
{
  int i;
  int sync = 0;

  // Wait for a long period of high clock so we sync to start of packet
  while(!sync)
    {
      // Reset timer
      TMR1 = 0x8000;
      TMR1IF = 0;
      
      // Wait for clock to go low
      while((PORTA & GAUGECLK)!=0)
	{
	}

      // See how long we were high for
      if( TMR1IF )
	{
	  // We have sync
	  sync = 1;
	  break;
	}
    }

  GIE = 0; 					// Disable global interrupts

  // Run as fast as we can to capture the data packets
  for(i=0; i<48; i++)
    {
      // Wait for clock to go high
      while((PORTA & GAUGECLK)==0)
	{
	}

      gb[i] = PORTA;

      // Wait for clock to go low
      while((PORTA & GAUGECLK)!=0)
	{
	}
      //     t2 = TMR1;

    }

  GIE = 1;

  // We have time to build the data now, so scan the bit array and build the
  // data bytes
  gaugedata[0] = 0;
  gaugedata[1] = 0;
  gaugedata[2] = 0;
  gaugedata[3] = 0;
  gaugedata[4] = 0;
  gaugedata[5] = 0;

  for(i=0; i<48; i++)
    {
      // Data is inverted
      if((gb[i] & GAUGEDATA)) 
	{
	  gaugedata[i/8] |= 1 << (i %8);
	}
    }

  // Put data into I2C Tx buffer
  GIE = 0; 					// Disable global interrupts
  I2C_Array_Tx[0] = gaugedata[3];
  I2C_Array_Tx[1] = gaugedata[4];
  I2C_Array_Tx[2] = gaugedata[5];
  I2C_Array_Tx[3] = I2C_Array_Tx[0]+I2C_Array_Tx[1]+I2C_Array_Tx[2];
  GIE = 1; 					// Enable global interrupts
}

void gauge_type_proc_24_bit(void)
{
  int i;

  
  gaugedata[0] = 0;
  gaugedata[1] = 0;
  gaugedata[2] = 0;
  int sync = 0;

  // Wait for a long period of high clock so we sync to start of packet
  while(!sync)
    {
      // Reset timer
      TMR1 = 0x0001;
      TMR1IF = 0;
      
      // Wait for clock to go high
      while((PORTA & GAUGECLK)==0)
	{
	}

      // See how long we were high for
      if( TMR1IF )
	{
	  // We have sync
	  sync = 1;
	  break;
	}
    }

  LATA = 0;
  LATA = GAUGEDATA_DRV | GAUGECLK_DRV;

  // Wait for clock to go high
  //while((PORTA & GAUGECLK)==0)
    //{
    //}
  
  for(i=0; i<24; i++)
    {
      
      // Wait for clock to go low
      while((PORTA & GAUGECLK)!=0)
	{
	}

      // Sample data
      if(!(PORTA & GAUGEDATA)) 
	{
	  gaugedata[i/8] |= 1 << (i %8);
	}
      
      // Wait for clock to go high (but not on last loop)
      if( i != 23 )
      {
      while((PORTA & GAUGECLK)==0)
	{
	}
    }
  }
  
  // Put data into I2C Tx buffer
  GIE = 0; 					// Disable global interrupts
#if 1
  I2C_Array_Tx[0] = gaugedata[0];
  I2C_Array_Tx[1] = gaugedata[1];
  I2C_Array_Tx[2] = gaugedata[2];
  I2C_Array_Tx[3] = I2C_Array_Tx[0]+I2C_Array_Tx[1]+I2C_Array_Tx[2];

#else
  I2C_Array_Tx[0] = I2C_Array_Rx[0];
  I2C_Array_Tx[1] = I2C_Array_Rx[1];
  I2C_Array_Tx[2] = 0x67;
#endif  
  GIE = 1; 					// Enable global interrupts

    LATA = 0;
    LATA = GAUGEDATA_DRV | GAUGECLK_DRV;
    LATA = 0;
    LATA = GAUGEDATA_DRV | GAUGECLK_DRV;

}

void gauge_type_proc_fixed(void)
{
  int i;

  gaugedata[0] = I2C_Array_Rx[0];
  gaugedata[1] = I2C_Array_Rx[1];
  gaugedata[2] = I2C_Array_Rx[2];

  // Put data into I2C Tx buffer
  GIE = 0; 					// Disable global interrupts
  I2C_Array_Tx[0] = gaugedata[0];
  I2C_Array_Tx[1] = gaugedata[1];
  I2C_Array_Tx[2] = gaugedata[2];
  
  GIE = 1; 					// Enable global interrupts
}

//
// RA0 and RA3 are gauge inputs
// RA1 and RA2 are gauge drives for hold etc 
//

void main(void) 
{ 
  int b;

  // Set up data directions appropriately
  // All inputs except gauge drive pins
  TRISA = 0b11111111 & (~GAUGEDATA_DRV) & (~GAUGECLK_DRV);

  LATA = GAUGEDATA_DRV | GAUGECLK_DRV;

#if 1
  // Set fast mode
  for(b=0;b<20000;b++)
    {

    }

  LATA = GAUGEDATA_DRV;
  for(b=0;b<10000;b++)
    {

    }

  LATA = GAUGEDATA_DRV | GAUGECLK_DRV;
  for(b=0;b<10000;b++)
    {

    }

  LATA = GAUGEDATA_DRV;
  for(b=0;b<10000;b++)
    {

    }

  LATA = GAUGEDATA_DRV | GAUGECLK_DRV;
#endif

  // Set up timer so we can use it to measure timings of edges
  T1CON  = 0b00000101;               // FOSc/4 clock

  initialize();               // call initialize routine

  // Main loop
  // Reads gauge as main task
  // Listens for commands from master over I2C

  while(1)
    {                           // program will wait here for ISR to be called
    //LATA = GAUGEDATA_DRV | GAUGECLK_DRV;
    //LATA = 0;


      
      // Check for a command from the master
      if( I2C_Array_Rx[0] != 0 )
	{
	  // Command has appeared, process it 
	  switch(I2C_Array_Rx[0])
	    {
	    case 1:
	      // Command that sets which mode to run in
	      gauge_type = I2C_Array_Rx[1];
	      break;

	    }

	}

      asm("CLRWDT");          // clear WDT

#if DEMO
      I2C_Array_Tx[0]++;
      I2C_Array_Tx[1]=0;
      I2C_Array_Tx[2]=0;

      I2C_Array_Tx[3] = I2C_Array_Tx[0]+I2C_Array_Tx[1]+I2C_Array_Tx[2];
#else
 
      switch(gauge_type)
	{
	case GAUGE_TYPE_2X24_BIT:
	  gauge_type_proc_2x24_bit();
	  break;

	case GAUGE_TYPE_24_BIT:
	  gauge_type_proc_24_bit();
	  break;

	case GAUGE_TYPE_FIXED:
	  gauge_type_proc_fixed();
	  break;

	case GAUGE_TYPE_TMR1:
	  gauge_type_proc_clock();
	  break;
	}
#endif
    }
}

/******************************************************************************/

/**************************** INITIALIZE ROUTINE ******************************/
void initialize(void)
{

  //uC SET UP
  OSCCON = 0b11110000; 		// Internal OSC @ 16MHz
  //  OPTION_REG = 0b11010111;    // WPU disabled, INT on rising edge, FOSC/4
  // Prescaler assigned to TMR0, rate @ 1:256
  WDTCON = 0b00010111;        // Prescaler 1:65536
  // period is 2 sec (RESET value)
  //    PORTC = 0x00;               // Clear PORTC
  //LATC = 0x00;                // Clear PORTC latches
  //TRISC = 0b00011000;         // Set RC3, RC4 as inputs for I2C
  ANSELA = 0;

  //    PORTA = 0x00;
  //TRISA = 0b11001111;
  //LATA - 0b00100000;

  //I2C SLAVE MODULE SET UP 
  SSPSTAT = 0b10000000;       // Slew rate control disabled for standard
  // speed mode (100 kHz and 1 MHz)
  SSPCON1 = 0b00110110; 		// Enable serial port, I2C slave mode, 
  // 7-bit address

  SSPCON2bits.SEN = 1;        // Clock stretching is enabled
  SSPCON3bits.BOEN = 1;       // SSPBUF is updated and NACK is generated
  // for a received address/data byte,
  // ignoring the state of the SSPOV bit
  // only if the BF bit = 0
  SSPCON3bits.SDAHT = 1;		// Minimum of 300 ns hold time on SDA after
  // the falling edge of SCL
  SSPCON3bits.SBCDE = 1;		// Enable slave bus collision detect interrupts
  SSPADD = I2C_slave_address; // Load the slave address
  SSP1IF = 0;                  // Clear the serial port interrupt flag
  BCL1IF = 0;                  // Clear the bus collision interrupt flag
  BCL1IE = 1;                  // Enable bus collision interrupts
  SSP1IE = 1;                  // Enable serial port interrupts
  PEIE = 1;                   // Enable peripheral interrupts
  GIE = 1; 					// Enable global interrupts
}

/******************************************************************************/

/*************************** ISR ROUTINE **************************************/
void interrupt ISR(void)
{
  if(SSP1IF)                               // check to see if SSP interrupt
    {
      if(SSPSTATbits.R_nW)                // Master read (R_nW = 1)
        {
	  if(!SSPSTATbits.D_nA)         // last byte was an address (D_nA = 0)
            {
	      junk = SSPBUF;                  // dummy read to clear BF bit
	      index_i2c = 0;                     // clear index pointer
	      if(index_i2c < RX_ELMNTS)
                {               // Does data exceed number of allocated bytes?
		  SSPBUF = I2C_Array_Tx[index_i2c++];   // load SSPBUF with data
		  //		  I2C_Array_Tx[index_i2c++] = clear;  // clear that location
                }                                       // and increment index
	      else                       // trying to exceed array size
                {
		  junk = SSPBUF;          // dummy read to clear BF bit
                }
	      SSPCON1bits.CKP = 1;        // release CLK
            }
	  if(SSPSTATbits.D_nA)       // last byte was data
            {
	      if (index_i2c < RX_ELMNTS)
                {               // Does data exceed number of allocated bytes?
		  SSPBUF = I2C_Array_Tx[index_i2c++];   // load SSPBUF with data
		  //		  I2C_Array_Tx[index_i2c++] = clear;  // clear that location
                }                                       // and increment index
	      else
                {
		  junk = SSPBUF;      // dummy read to clear BF bit
                }
	      SSPCON1bits.CKP = 1;    // release CLK
            }
        }
        
      if(!SSPSTATbits.R_nW)			// master write (R_nW = 0)
        {
	  if(!SSPSTATbits.D_nA)        // last byte was an address (D_nA = 0)
            {
	      junk = SSPBUF;			// read buffer to clear BF
	      SSPCON1bits.CKP = 1;            // release CLK
	      index_i2c = 0;
            }
	  if(SSPSTATbits.D_nA)                // last byte was data (D_nA = 1)
            {
	      if (index_i2c < RX_ELMNTS)	
                {               // Does data exceed number of allocated bytes?
		  I2C_Array_Rx[index_i2c++] = SSPBUF;
                }                                       // load array with data
	      else
                {
		  index_i2c = 0;                   // reset the index pointer
                }
	      if(SSPCON1bits.WCOL)		// Did a write collision occur?
		{
		  SSPCON1bits.WCOL = 0;       // clear WCOL bit
		  junk = SSPBUF;              // clear SSPBUF
		}
	      SSPCON1bits.CKP = 1;    		// release CLK
            }
    	}
    }
  if(BCL1IF)                       // Did a bus collision occur?
    {
      junk = SSPBUF;              // clear SSPBUF
      BCL1IF = 0;                  // clear BCL1IF
      SSPCON1bits.CKP = 1;        // Release CLK
    }
  SSP1IF = 0;                      // clear SSP1IF
}
