/*
 * testDisplay.c:
 *        Standard 7 segments display program in wiringOli. Blinks the LED connected
 *        to the PG0 to PG7 GPIO pin on Olinuxino A20.
 *
 * Copyright (c) 2012-2013 
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
  oliExtSetup();
  // Loop
  int i;
  for (i=0; i<10; i++)
  {
    digitalWriteDisplay(i);
    printf("AB=%d\n", i);
    delay(1000);
  }
  return(0);
}
