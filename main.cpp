/*
 * testLED.cpp
 *
 *  Created on: 14 oct. 2013
 *      Author: denm
 */
#include "gpio_lib.h"
#include "wiringOli.h"

int main()
{
  // Setup GPIO
  wiringOliSetup();
  // Set LED pin (PH2) as output
  pinMode(SUNXI_GPH(2), OUTPUT);
  // Loop
  for (int i=0; i<1000; i++)
  {
    digitalWrite(SUNXI_GPH(2), HIGH);
    delay(100);
    digitalWrite(SUNXI_GPH(2), LOW);
    delay(1000);
  }
  return(0);
}
