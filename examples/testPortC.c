/*
 * testPortC.c:
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
 
#include <wiringOli.h>
#include <oliExt.h>

int main()
{
  // Setup GPIO
  wiringOliSetup();
  int i;
  // Loop
  for (i=0; i<8; i++)
  {
    // Set pin LED as output
    pinModePortC(i, OUTPUT);
    digitalWritePortC(i, HIGH);
    delay(100);
    digitalWritePortC(i, LOW);
    delay(100);
  }
  return(0);
}
