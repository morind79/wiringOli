/*
 * wiringOliI2C.c:
 *	I2C access routines
 ***********************************************************************
 * This file is part of wiringOli:
 *	https://
 *
 *    wiringOli is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU Lesser General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    wiringoli is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public License
 *    along with wiringOli.  If not, see <http://www.gnu.org/licenses/>.
 ***********************************************************************
 */

#include <stdint.h>
#include <unistd.h>
#include <linux/i2c-dev.h>
#include <fcntl.h>
#include <stdio.h>

#include "wiringOli.h"
#include "wiringOliI2C.h"


/*
 * wiringOliI2CRead:
 *	Simple device read
 *********************************************************************************
 */
int wiringOliI2CRead(int fd)
{
  return i2c_smbus_read_byte(fd);
}


/*
 * wiringOliI2CReadReg8: wiringOliI2CReadReg16:
 *	Read an 8 or 16-bit value from a regsiter on the device
 *********************************************************************************
 */
int wiringOliI2CReadReg8(int fd, int reg)
{
  return i2c_smbus_read_byte_data(fd, reg);
}

int wiringOliI2CReadReg16(int fd, int reg)
{
  return i2c_smbus_read_word_data(fd, reg);
}


/*
 * wiringOliI2CWrite:
 *	Simple device write
 *********************************************************************************
 */
int wiringOliI2CWrite(int fd, int data)
{
  return i2c_smbus_write_byte(fd, data);
}


/*
 * wiringOliI2CWriteReg8: wiringOliI2CWriteReg16:
 *	Write an 8 or 16-bit value to the given register
 *********************************************************************************
 */

int wiringOliI2CWriteReg8(int fd, int reg, int data)
{
  return i2c_smbus_write_byte_data(fd, reg, data);
}

int wiringOliI2CWriteReg16(int fd, int reg, int data)
{
  return i2c_smbus_write_word_data(fd, reg, data);
}


/*
 * wiringOliI2CSetup:
 *	Open the I2C device (0, 1, 2 or 3 on A20), and regsiter the target device
 *      DevId is the I2C device address like 0x27
 *********************************************************************************
 */
int wiringOliI2CSetup(int device, int address)
{
  int fd=-1;
  // Open the I2C device
  switch(device)
  {
    case 0:	
      fd = open("/dev/i2c-0", O_RDWR);
      break;
    case 1:	
      fd = open("/dev/i2c-1", O_RDWR);
      break;
    case 2:	
      fd = open("/dev/i2c-2", O_RDWR);
      break;
    case 3:	
      fd = open("/dev/i2c-3", O_RDWR);
      break;
    default:
      break;
  }

  // Test if we can open I2C device
  if (fd  < 0)
    return -1;

  // Test if we can acquarie bus access at specified address
  if (ioctl(fd, I2C_SLAVE_FORCE, address) < 0)
    return -1;

  return fd;
}

/*
 * I2C_Close:
 *	Close I2C
 *********************************************************************************
 */
void I2C_Close(int fd)
{
  if(close(fd) < 0)
  {
    return;
  }
}

void I2C_Send(int fd, unsigned char *buffer, unsigned char num_bytes)
{
  int count = 0;
  count = write(fd, buffer, num_bytes);
  if(count != num_bytes)
  {
    return;
  }	
}

void I2C_Read(int fd, unsigned char *buffer, unsigned char num_bytes)
{

  int count = 0;
  count = read(fd, buffer, num_bytes);
  if(count != num_bytes)
  {
    return;
  }
}
