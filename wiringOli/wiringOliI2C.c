#include <stdint.h>
#include <unistd.h>
#include <linux/i2c-dev.h>
#include <fcntl.h>
#include <stdio.h>

#include "wiringOli.h"
#include "wiringOliI2C.h"


#ifndef BUSS
#define BUSS 2
#endif
#define ERR_I2C_OPEN -1
#define ERR_I2C_SETA -2

#ifndef PIO_RESET 
#define PIO_RESET PIN_PE0
#endif

/*
 * wiringPiI2CRead:
 *	Simple device read
 *********************************************************************************
 */

int wiringOliI2CRead (int fd)
{
  return i2c_smbus_read_byte (fd) ;
}


/*
 * wiringPiI2CReadReg8: wiringPiI2CReadReg16:
 *	Read an 8 or 16-bit value from a regsiter on the device
 *********************************************************************************
 */

int wiringOliI2CReadReg8 (int fd, int reg)
{
  return i2c_smbus_read_byte_data (fd, reg) ;
}

int wiringPiI2CReadReg16 (int fd, int reg)
{
  return i2c_smbus_read_word_data (fd, reg) ;
}


/*
 * wiringPiI2CWrite:
 *	Simple device write
 *********************************************************************************
 */

int wiringOliI2CWrite (int fd, int data)
{
  return i2c_smbus_write_byte (fd, data) ;
}


/*
 * wiringPiI2CWriteReg8: wiringPiI2CWriteReg16:
 *	Write an 8 or 16-bit value to the given register
 *********************************************************************************
 */

int wiringOliI2CWriteReg8 (int fd, int reg, int data)
{
  return i2c_smbus_write_byte_data (fd, reg, data) ;
}

int wiringPiI2CWriteReg16 (int fd, int reg, int data)
{
  return i2c_smbus_write_word_data (fd, reg, data) ;
}


/*
 * wiringPiI2CSetup:
 *	Open the I2C device, and regsiter the target device
 *********************************************************************************
 */

int wiringOliI2CSetup (int devId)
{
  int fd ;
  char *device ;

  device = "/dev/i2c-0" ;

  if ((fd = open (device, O_RDWR)) < 0)
    return -1 ;

  if (ioctl (fd, I2C_SLAVE, devId) < 0)
    return -1 ;

  return fd ;
}
