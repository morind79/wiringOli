#include <stdint.h>
#include <unistd.h>
#include <linux/i2c-dev.h>
#include <fcntl.h>
#include <stdio.h>

#include "wiringOliI2C.h"


#ifndef BUSS
#define BUSS 2
#endif
#define ERR_I2C_OPEN -1
#define ERR_I2C_SETA -2

#ifndef PIO_RESET 
#define PIO_RESET PIN_PE0
#endif

int16_t g_i2c_fh;
uint8_t  g_i2c_addr;

int16_t i2c_init(uint8_t p_address, uint8_t p_buss)
{

  uint8_t l_filename[20];
  uint16_t l_adapter_nr = p_buss;

  snprintf(l_filename, 19, "/dev/i2c-%d", l_adapter_nr);
  g_i2c_fh = open(l_filename, O_RDWR);
  if(g_i2c_fh < 0)
    return  ERR_I2C_OPEN;
  if(ioctl(g_i2c_fh, I2C_SLAVE, p_address) < 0) 
    return ERR_I2C_SETA;
  return 1;
}
 
int16_t i2c_close()
{
  close(g_i2c_fh);
  return 1;
}

void i2c_reset()
{
}
	

uint16_t i2c_read(uint8_t p_reg, void * buff, uint16_t length)
{
  write(g_i2c_fh, &p_reg, 1);
  return read(g_i2c_fh, buff, length);
}

uint16_t i2c_write(uint8_t p_reg, void *buff, uint16_t length)
{
  write(g_i2c_fh, &p_reg, 1);
  return write(g_i2c_fh, buff, length);
}
