/*
 * testLED.c:
 *        Standard "blink" program in wiringOli. Blinks the LED connected
 *        to the PH2 GPIO pin on Olinuxino A20.
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

int main()
{
  // Setup GPIO
  wiringOliSetup();
  // Set pin LED (PH2=40)
  int pinLED = 40;
  // Set pin LED as output
  pinMode(pinLED, OUTPUT);
  // Loop
  for (;;)
  {
    digitalWrite(pinLED, HIGH);
    delay(200);
    digitalWrite(pinLED, LOW);
    delay(200);
  }
  return(0);
}
