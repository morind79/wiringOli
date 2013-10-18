/*
 * wiringOliI2C.h:
 *	Simplified I2C access routines
 *	Copyright (c) 2013 Gordon Henderson
 ***********************************************************************
 * This file is part of wiringOli:
 *	http://
 *
 *    wiringOli is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU Lesser General Public License as
 *    published by the Free Software Foundation, either version 3 of the
 *    License, or (at your option) any later version.
 *
 *    wiringOli is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with wiringOli.
 *    If not, see <http://www.gnu.org/licenses/>.
 ***********************************************************************
 */

#ifdef __cplusplus
extern "C" {
#endif

extern void i2c_init(int speed, int slaveaddr);
extern int i2c_probe(uchar chip);               
static int i2c_wait_ctl(int mask, int state);
static void i2c_clear_irq(void);
static int i2c_wait_irq(void);
static int i2c_wait_status(int status);
static int i2c_wait_irq_status(int status);
static int i2c_wait_bus_idle(void);
static int i2c_stop(void);
static int i2c_send_data(u8 data, u8 status);
static int i2c_start(int status);
extern int i2c_do_read(uchar chip, uint addr, int alen, uchar *buffer, int len);
extern int i2c_read(uchar chip, uint addr, int alen, uchar *buffer, int len);
static int i2c_do_write(uchar chip, uint addr, int alen, uchar *buffer, int len);
extern int i2c_write(uchar chip, uint addr, int alen, uchar *buffer, int len);

#ifdef __cplusplus
}
#endif