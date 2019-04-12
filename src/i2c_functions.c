/*
 * i2c_functions.c
 *
 *  Created on: 23 Jul 2016
 *      Author: menadue
 */
#include "chip.h"
#include "board.h"
#include "i2c_functions.h"


// I2C functions

// Release the bus
void i2c_release(I2C_PORT_DESC *port) {
  // All inputs
  Chip_GPIO_SetPinDIRInput(LPC_GPIO, port->sdaport, port->sdabit);
  Chip_GPIO_SetPinDIRInput(LPC_GPIO, port->sclport, port->sclbit);
}

// Delay to slow down to I2C bus rates
void i2c_delay(I2C_PORT_DESC *port) {
  volatile int i;
  for(i=0; i<1; i++)
    {
    }
}

void i2c_sda_low(I2C_PORT_DESC *port) {
  // Take SCL low by driving a 0 on to the bus
  Chip_GPIO_SetPinState(LPC_GPIO, port->sdaport, port->sdabit, 0);
  
  // Make sure bit is an output
  Chip_GPIO_SetPinDIROutput(LPC_GPIO, port->sdaport, port->sdabit);
}

void i2c_sda_high(I2C_PORT_DESC *port) {
  // Make sure bit is an intput
  Chip_GPIO_SetPinDIRInput(LPC_GPIO, port->sdaport, port->sdabit);
}

void i2c_scl_low(I2C_PORT_DESC *port) {
  Chip_GPIO_SetPinState(LPC_GPIO, port->sclport, port->sclbit, 0);

  // Make sure bit is an output
  Chip_GPIO_SetPinDIROutput(LPC_GPIO, port->sclport, port->sclbit);
  }

void i2c_scl_high(I2C_PORT_DESC *port)
{
  // Make sure bit is an input
  Chip_GPIO_SetPinDIRInput(LPC_GPIO, port->sclport, port->sclbit);
}

// Read ACK bit

int i2c_read_sda(I2C_PORT_DESC *port)
{
  return(Chip_GPIO_GetPinState(LPC_GPIO, port->sdaport, port->sdabit));

  //  return( ((*(port->port)) & (port->sda_mask)) == port->sda_mask);
}

// I2C start condition

void i2c_start(I2C_PORT_DESC *port)
{
  //
  i2c_sda_low(port);

  i2c_delay(port);
  i2c_scl_low(port);
  i2c_delay(port);
}

void i2c_stop(I2C_PORT_DESC *port)
{
  i2c_scl_high(port);
  i2c_delay(port);
  i2c_sda_high(port);
  i2c_delay(port);

}

// Send 8 bits and read ACK
// Returns number of acks received

int i2c_send_byte(I2C_PORT_DESC *port, BYTE b)
{
  int i;
  int ack=0;
  int retries = 100;
  int rc =1;

  for (i = 0; i < 8; i++)
    {
      // Set up data
      if ((b & 0x80) == 0x80)
	{
	  i2c_sda_high(port);
	} else
	{
	  i2c_sda_low(port);
	}

      // Delay
      i2c_delay(port);

      // Take clock high and then low
      i2c_scl_high(port);

      // Delay
      i2c_delay(port);

      // clock low again
      i2c_scl_low(port);

      // Delay
      i2c_delay(port);

      // Shift next data bit in
      b <<= 1;
    }

  // release data line
  i2c_sda_high(port);

  // Now get ACK
  i2c_scl_high(port);

  i2c_delay(port);

  // read ACK

  while( ack = i2c_read_sda(port) )
    {
      retries--;

      if ( retries == 0 )
	{
	  rc = 0;
	  break;
	}
    }

  i2c_scl_low(port);

  i2c_delay(port);
  return (rc);
}


// Receive 8 bits and set ACK
// Ack as specified
void i2c_recv_byte(I2C_PORT_DESC *port, BYTE *data, int ack)
{
  int i, b;

  b = 0;

  // Make data an input
  i2c_sda_high(port);

  for (i = 0; i < 8; i++)
    {
      // Delay
      i2c_delay(port);

      // Take clock high and then low
      i2c_scl_high(port);

      // Delay
      i2c_delay(port);

      // Shift next data bit in
      b <<= 1;
      b += (i2c_read_sda(port) & 0x1);

      // clock low again
      i2c_scl_low(port);

      // Delay
      i2c_delay(port);

    }

  // ACK is as we are told 
  if ( ack )
    {
      // Data low for ACK
      i2c_sda_low(port);
    }
  else
    {
      i2c_sda_high(port);
    }

  // Now send ACK
  i2c_scl_high(port);

  i2c_delay(port);

  i2c_scl_low(port);

  i2c_delay(port);

  *data = b;
  return (0);
}

// Reads a block of bytes from a slave

int i2c_read_bytes(I2C_SLAVE_DESC *slave, int n, BYTE *data)
{
  int i;
  BYTE byte;

  i2c_start(slave->port);

  // Send slave address with read bit
  if ( !i2c_send_byte(slave->port, ((slave->slave_7bit_addr) << 1) | 1) )
    {
      i2c_stop(slave->port);
      return(0);
    }


  //
  for (i = 0; i < n; i++)
    {
      i2c_recv_byte(slave->port, &byte, (i==(n-1))? 0 : 1);
      *(data++) = byte;
    }

  i2c_stop(slave->port);

  return(1);
}

// Sends a block of data to I2C slave
void i2c_send_bytes(I2C_SLAVE_DESC *slave, int n, BYTE *data)
{
  int i;

  i2c_start(slave->port);

  // Send slave address with read bit
  i2c_send_byte(slave->port, ((slave->slave_7bit_addr) << 1) | 0);

  //
  for (i = 0; i < n; i++)
    {
      i2c_send_byte(slave->port, *(data++));
    }

  i2c_stop(slave->port);
}

void i2c_init(I2C_PORT_DESC *port)
{
  Chip_GPIO_SetPinDIROutput(LPC_GPIO, 0, 19);
  Chip_GPIO_SetPinDIROutput(LPC_GPIO, 0, 2);
  Chip_GPIO_SetPinDIROutput(LPC_GPIO, 0, 18);
  Chip_GPIO_SetPinDIROutput(LPC_GPIO, 0, 7);
  Chip_GPIO_SetPinDIROutput(LPC_GPIO, 1, 19);
  Chip_GPIO_SetPinDIROutput(LPC_GPIO, 1, 20);
}

