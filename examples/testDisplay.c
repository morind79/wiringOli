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

int main()
{
  // Setup GPIO
  wiringOliSetup();
  int pinA = 0;
  int pinB = 1;
  int pinC = 2;
  int pinD = 3;
  int pinE = 4;
  int pinF = 5;
  int pinG = 6;
  int pinDot = 7;

  // Set pin LED as output
  pinMode(pinA, OUTPUT);
  pinMode(pinB, OUTPUT);
  pinMode(pinC, OUTPUT);
  pinMode(pinD, OUTPUT);
  pinMode(pinE, OUTPUT);
  pinMode(pinF, OUTPUT);
  pinMode(pinG, OUTPUT);
  pinMode(pinDot, OUTPUT);
  // Set pull down
  pullUpDnControlGpio(pinA, 2);
  pullUpDnControlGpio(pinB, 2);
  pullUpDnControlGpio(pinC, 2);
  pullUpDnControlGpio(pinD, 2);
  pullUpDnControlGpio(pinE, 2);
  pullUpDnControlGpio(pinF, 2);
  pullUpDnControlGpio(pinG, 2);
  pullUpDnControlGpio(pinDot, 2);
  // Loop
  for (;;)
  {
    digitalWrite(pinA, HIGH);
    delay(100);
    digitalWrite(pinB, HIGH);
    delay(100);
    digitalWrite(pinC, HIGH);
    delay(100);
    digitalWrite(pinD, HIGH);
    delay(100);
    digitalWrite(pinE, HIGH);
    delay(100);
    digitalWrite(pinF, HIGH);
    delay(100);
    digitalWrite(pinG, HIGH);
    delay(100);
    digitalWrite(pinDot, HIGH);
    delay(100);
    digitalWrite(pinA, LOW);
    delay(100);
    digitalWrite(pinB, LOW);
    delay(100);
    digitalWrite(pinC, LOW);
    delay(100);
    digitalWrite(pinD, LOW);
    delay(100);
    digitalWrite(pinE, LOW);
    delay(100);
    digitalWrite(pinF, LOW);
    delay(100);
    digitalWrite(pinG, LOW);
    delay(100);
    digitalWrite(pinDot, LOW);
    delay(100);
  }
  return(0);
}
