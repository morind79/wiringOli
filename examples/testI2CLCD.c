/*
 * testI2CLCD.c:
 *        Standard "blink" program in wiringOli. Blink the LEDs connected
 *        to port C of OliExt using GPIO pin on Olinuxino A20.
 *
 * Copyright (c) 2013-2014 
 ***********************************************************************
 * This file is part of wiringOli:
 *        http://
 *
 *    wiringOli is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU Lesser General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    wiringOli is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public License
 *    along with wiringOli.  If not, see <http://www.gnu.org/licenses/>.
 ***********************************************************************
 */
#include <stdio.h>
#include <wiringOli.h>
#include <wiringOliI2C.h>
#include <oliExt.h>
#include <i2cLcd.h>

int main()
{
  // Setup GPIO
  wiringOliSetup();
  printf("LCD Init...\n");
  // LCD at address 0x27, with 4 rows and 20 columns
  LCDInit(0x27, 4, 20);
  LCDPrintString(0, 0, "Test");
  LCDPrintString(1, 0, "This is second line");
  return(0);
}
