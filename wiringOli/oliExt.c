/*
 * oliExt.c
 *
 *  Created on: 14 oct. 2013
 *      Author: denm
 */

#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <sys/time.h>
#include <wiringOli.h>

#include "oliExt.h"

// Pin definition
static int pinPortA[16] = {41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56};  // PH0 to PH21
static int pinPortB[16] = {63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 57, 58};  // PB3 to PB17 and PH22, PH23
static int pinPortC[8] = {77, 78, 79, 80, 27, 28, 29, 30};  // PH24 to PH27 and PE0 to PE3
static int pinPortD[8] = {31, 32, 33, 34, 35, 36, 37, 38};  // PE4 to PE11
static int pinPortGeneral[2] = {18, 19}; // PI14 and PI15
static int pinSIM900[4] = {20, 21, 17, 22}; // PC3 = RST, PC7 = LED, PI11 = RI, PC16 = ON/OFF
static int pinDisplay[8] = {0, 1, 2, 3, 4, 5, 12, 13}; // PG0 to PG5, PI0 and PI1

/*
 * pinOliPortA :
 *	Get board pin corresponding to PortA pin numbering
 *********************************************************************************
 */
int pinOliPortA(int pin)
{
  return pinPortA[pin];
}

/*
 * pinOliPortB :
 *	Get board pin corresponding to PortB pin numbering
 *********************************************************************************
 */
int pinOliPortB(int pin)
{
  return pinPortB[pin];
}

/*
 * pinOliPortC :
 *	Get board pin corresponding to PortC pin numbering
 *********************************************************************************
 */
int pinOliPortC(int pin)
{
  return pinPortC[pin];
}

/*
 * pinOliPortD :
 *	Get board pin corresponding to PortD pin numbering
 *********************************************************************************
 */
int pinOliPortD(int pin)
{
  return pinPortD[pin];
}

/*
 * pinOliPortGeneral :
 *	Get board pin corresponding to Portgeneral pin numbering
 *********************************************************************************
 */
int pinOliPortGeneral(int pin)
{
  return pinPortGeneral[pin];
}

/*
 * pinOliDisplay :
 *	Get board pin corresponding to Display pin numbering
 *********************************************************************************
 */
int pinOliDisplay(int pin)
{
  return pinDisplay[pin];
}

/*
 * pinModePortA :
 *	Sets the mode of a pin to be INPUT, OUTPUT
 *********************************************************************************
 */
void pinModePortA(int pin, int mode)
{
  pinMode(pinOliPortA(pin), mode);
}

/*
 * pinModePortB :
 *	Sets the mode of a pin to be INPUT, OUTPUT
 *********************************************************************************
 */
void pinModePortB(int pin, int mode)
{
  pinMode(pinOliPortB(pin), mode);
}

/*
 * pinModePortC :
 *	Sets the mode of a pin to be INPUT, OUTPUT
 *********************************************************************************
 */
void pinModePortC(int pin, int mode)
{
  pinMode(pinOliPortC(pin), mode);
}

/*
 * pinModePortD :
 *	Sets the mode of a pin to be INPUT, OUTPUT
 *********************************************************************************
 */
void pinModePortD(int pin, int mode)
{
  pinMode(pinOliPortD(pin), mode);
}

/*
 * pinModePortGeneral :
 *	Sets the mode of a pin to be INPUT, OUTPUT
 *********************************************************************************
 */
void pinModePortGeneral(int pin, int mode)
{
  pinMode(pinOliPortGeneral(pin), mode);
}

/*
 * pullUpDnCtrlPortA :
 *	Control the internal pull-up/down resistors on PortA pin
 *      pud=0 -> pull disable, 1 -> pull-up, 2 -> pull-down
 *********************************************************************************
 */
void pullUpDnCtrlPortA(int pin, int pud)
{
  pullUpDnControlGpio(pinOliPortA(pin), pud);
}

/*
 * pullUpDnCtrlPortB :
 *	Control the internal pull-up/down resistors on PortB pin
 *      pud=0 -> pull disable, 1 -> pull-up, 2 -> pull-down
 *********************************************************************************
 */
void pullUpDnCtrlPortB(int pin, int pud)
{
  pullUpDnControlGpio(pinOliPortB(pin), pud);
}

/*
 * pullUpDnCtrlPortC :
 *	Control the internal pull-up/down resistors on PortC pin
 *      pud=0 -> pull disable, 1 -> pull-up, 2 -> pull-down
 *********************************************************************************
 */
void pullUpDnCtrlPortC(int pin, int pud)
{
  pullUpDnControlGpio(pinOliPortC(pin), pud);
}

/*
 * pullUpDnCtrlPortD :
 *	Control the internal pull-up/down resistors on PortD pin
 *      pud=0 -> pull disable, 1 -> pull-up, 2 -> pull-down
 *********************************************************************************
 */
void pullUpDnCtrlPortD(int pin, int pud)
{
  pullUpDnControlGpio(pinOliPortD(pin), pud);
}

/*
 * pullUpDnCtrlPortGeneral :
 *	Control the internal pull-up/down resistors on PortGeneral pin
 *      pud=0 -> pull disable, 1 -> pull-up, 2 -> pull-down
 *********************************************************************************
 */
void pullUpDnCtrlPortGeneral(int pin, int pud)
{
  pullUpDnControlGpio(pinOliPortGeneral(pin), pud);
}

/*
 * digitalReadPortA :
 *	Read the value of a given PortA pin, returning HIGH or LOW
 *********************************************************************************
 */
int digitalReadPortA(int pin)
{
  return(digitalRead(pinOliPortA(pin)));
}

/*
 * digitalReadPortB :
 *	Read the value of a given PortB pin, returning HIGH or LOW
 *********************************************************************************
 */
int digitalReadPortB(int pin)
{
  return(digitalRead(pinOliPortB(pin)));
}

/*
 * digitalReadPortC :
 *	Read the value of a given PortC pin, returning HIGH or LOW
 *********************************************************************************
 */
int digitalReadPortC(int pin)
{
  return(digitalRead(pinOliPortC(pin)));
}

/*
 * digitalReadPortD :
 *	Read the value of a given PortD pin, returning HIGH or LOW
 *********************************************************************************
 */
int digitalReadPortD(int pin)
{
  return(digitalRead(pinOliPortD(pin)));
}

/*
 * digitalReadPortGeneral :
 *	Read the value of a given PortGeneral pin, returning HIGH or LOW
 *********************************************************************************
 */
int digitalReadPortGeneral(int pin)
{
  return(digitalRead(pinOliPortGeneral(pin)));
}

/*
 * digitalWritePortA :
 *	Set an output bit
 *********************************************************************************
 */
void digitalWritePortA(int pin, int value)
{
  digitalWrite(pinOliPortA(pin), value);
}

/*
 * digitalWritePortB :
 *	Set an output bit
 *********************************************************************************
 */
void digitalWritePortB(int pin, int value)
{
  digitalWrite(pinOliPortB(pin), value);
}

/*
 * digitalWritePortC :
 *	Set an output bit
 *********************************************************************************
 */
void digitalWritePortC(int pin, int value)
{
  digitalWrite(pinOliPortC(pin), value);
}

/*
 * digitalWritePortD :
 *	Set an output bit
 *********************************************************************************
 */
void digitalWritePortD(int pin, int value)
{
  digitalWrite(pinOliPortD(pin), value);
}

/*
 * digitalWritePortGeneral :
 *	Set an output bit
 *********************************************************************************
 */
void digitalWritePortGeneral(int pin, int value)
{
  digitalWrite(pinOliPortGeneral(pin), value);
}

/*
 * digitalWriteDisplaySegment :
 *	Set an output bit
 *********************************************************************************
 */
void digitalWriteDisplaySegment(int pin, int value)
{
  digitalWrite(pinOliDisplay(pin), value);
}

/*
 * digitalWriteDisplay :
 *	Set an output bit
 *********************************************************************************
 */
void digitalWriteDisplay(int value)
{
  if (value == 0)
  {
    digitalWrite(pinOliDisplay(0), HIGH);
    digitalWrite(pinOliDisplay(1), HIGH);
    digitalWrite(pinOliDisplay(2), HIGH);
    digitalWrite(pinOliDisplay(3), HIGH);
    digitalWrite(pinOliDisplay(4), HIGH);
    digitalWrite(pinOliDisplay(5), HIGH);
    digitalWrite(pinOliDisplay(6), LOW);
    digitalWrite(pinOliDisplay(7), LOW);  
  }
  else if (value == 1)
  {
    digitalWrite(pinOliDisplay(0), LOW);
    digitalWrite(pinOliDisplay(1), HIGH);
    digitalWrite(pinOliDisplay(2), HIGH);
    digitalWrite(pinOliDisplay(3), LOW);
    digitalWrite(pinOliDisplay(4), LOW);
    digitalWrite(pinOliDisplay(5), LOW);
    digitalWrite(pinOliDisplay(6), LOW);
    digitalWrite(pinOliDisplay(7), LOW);  
  }
  else if (value == 2)
  {
    digitalWrite(pinOliDisplay(0), HIGH);
    digitalWrite(pinOliDisplay(1), HIGH);
    digitalWrite(pinOliDisplay(2), LOW);
    digitalWrite(pinOliDisplay(3), HIGH);
    digitalWrite(pinOliDisplay(4), HIGH);
    digitalWrite(pinOliDisplay(5), LOW);
    digitalWrite(pinOliDisplay(6), HIGH);
    digitalWrite(pinOliDisplay(7), LOW);  
  }
  else if (value == 3)
  {
    digitalWrite(pinOliDisplay(0), HIGH);
    digitalWrite(pinOliDisplay(1), HIGH);
    digitalWrite(pinOliDisplay(2), HIGH);
    digitalWrite(pinOliDisplay(3), HIGH);
    digitalWrite(pinOliDisplay(4), LOW);
    digitalWrite(pinOliDisplay(5), LOW);
    digitalWrite(pinOliDisplay(6), HIGH);
    digitalWrite(pinOliDisplay(7), LOW);  
  }
  else if (value == 4)
  {
    digitalWrite(pinOliDisplay(0), LOW);
    digitalWrite(pinOliDisplay(1), HIGH);
    digitalWrite(pinOliDisplay(2), HIGH);
    digitalWrite(pinOliDisplay(3), LOW);
    digitalWrite(pinOliDisplay(4), LOW);
    digitalWrite(pinOliDisplay(5), HIGH);
    digitalWrite(pinOliDisplay(6), HIGH);
    digitalWrite(pinOliDisplay(7), LOW);  
  }
  else if (value == 5)
  {
    digitalWrite(pinOliDisplay(0), HIGH);
    digitalWrite(pinOliDisplay(1), LOW);
    digitalWrite(pinOliDisplay(2), HIGH);
    digitalWrite(pinOliDisplay(3), HIGH);
    digitalWrite(pinOliDisplay(4), LOW);
    digitalWrite(pinOliDisplay(5), HIGH);
    digitalWrite(pinOliDisplay(6), HIGH);
    digitalWrite(pinOliDisplay(7), LOW);  
  }
  else if (value == 6)
  {
    digitalWrite(pinOliDisplay(0), LOW);
    digitalWrite(pinOliDisplay(1), LOW);
    digitalWrite(pinOliDisplay(2), HIGH);
    digitalWrite(pinOliDisplay(3), HIGH);
    digitalWrite(pinOliDisplay(4), HIGH);
    digitalWrite(pinOliDisplay(5), HIGH);
    digitalWrite(pinOliDisplay(6), HIGH);
    digitalWrite(pinOliDisplay(7), LOW);  
  }
  else if (value == 7)
  {
    digitalWrite(pinOliDisplay(0), HIGH);
    digitalWrite(pinOliDisplay(1), HIGH);
    digitalWrite(pinOliDisplay(2), HIGH);
    digitalWrite(pinOliDisplay(3), LOW);
    digitalWrite(pinOliDisplay(4), LOW);
    digitalWrite(pinOliDisplay(5), LOW);
    digitalWrite(pinOliDisplay(6), LOW);
    digitalWrite(pinOliDisplay(7), LOW);  
  }
  else if (value == 8)
  {
    digitalWrite(pinOliDisplay(0), HIGH);
    digitalWrite(pinOliDisplay(1), HIGH);
    digitalWrite(pinOliDisplay(2), HIGH);
    digitalWrite(pinOliDisplay(3), HIGH);
    digitalWrite(pinOliDisplay(4), HIGH);
    digitalWrite(pinOliDisplay(5), HIGH);
    digitalWrite(pinOliDisplay(6), HIGH);
    digitalWrite(pinOliDisplay(7), LOW);  
  }
  else if (value == 9)
  {
    digitalWrite(pinOliDisplay(0), HIGH);
    digitalWrite(pinOliDisplay(1), HIGH);
    digitalWrite(pinOliDisplay(2), HIGH);
    digitalWrite(pinOliDisplay(3), LOW);
    digitalWrite(pinOliDisplay(4), LOW);
    digitalWrite(pinOliDisplay(5), HIGH);
    digitalWrite(pinOliDisplay(6), HIGH);
    digitalWrite(pinOliDisplay(7), LOW);  
  }
  else
  {
    digitalWrite(pinOliDisplay(0), LOW);
    digitalWrite(pinOliDisplay(1), LOW);
    digitalWrite(pinOliDisplay(2), LOW);
    digitalWrite(pinOliDisplay(3), LOW);
    digitalWrite(pinOliDisplay(4), LOW);
    digitalWrite(pinOliDisplay(5), LOW);
    digitalWrite(pinOliDisplay(6), LOW);
    digitalWrite(pinOliDisplay(7), LOW);  
  }
}

/*
 * digitalWriteSIM900_ON :
 *	Set ON/OFF of SIM900 module
 *********************************************************************************
 */
void digitalWriteSIM900_ON(int value)
{
  digitalWrite(pinSIM900[3], value);
}

/*
 * digitalReadSIM900_LED :
 *	Read LED value of SIM900 module, returning HIGH or LOW
 *********************************************************************************
 */
int digitalReadSIM900_LED()
{
  return(digitalRead(pinSIM900[1]));
}

/*
 * digitalWriteSIM900_RST :
 *	Set RST of SIM900 module
 *********************************************************************************
 */
void digitalWriteSIM900_RST(int value)
{
  digitalWrite(pinSIM900[0], value);
}

/*
 * digitalReadSIM900_RI :
 *	Read RI value of SIM900 module, returning HIGH or LOW
 *********************************************************************************
 */
int digitalReadSIM900_RI()
{
  return(digitalRead(pinSIM900[2]));
}

/*
 * oliextSetup:
 *	Must be called once at the start of your program execution.
 *
 * Default setup: All port (A, B, C, D, General) as input
 *	              Display as output
 *	              SIM900 PC7, PC3=output, PI11, PI10=input 
 *********************************************************************************
 */
void oliExtSetup()
{
  int i = 0;
  // Port A and B (16 bit) -> Input
  for (i=0; i<16; i++)
  {
    pinMode(pinOliPortA(i), INPUT);
    pinMode(pinOliPortB(i), INPUT);
  }
  // Port C and D (8 bit) -> Input
  for (i=0; i<8; i++)
  {
    pinMode(pinOliPortC(i), INPUT);
    pinMode(pinOliPortD(i), INPUT);
  }
  // Port General (2 bit) -> Input
  for (i=0; i<2; i++)
  {
    pinMode(pinPortGeneral[i], INPUT);
  }
  // Display (8 bit) -> Output
  for (i=0; i<8; i++)
  {
    pinMode(pinOliDisplay(i), OUTPUT);
  }
  // SIM900 (4 bit) -> Input and Output
  pinMode(pinSIM900[0], OUTPUT);
  pinMode(pinSIM900[1], INPUT);
  pinMode(pinSIM900[2], INPUT);
  pinMode(pinSIM900[3], OUTPUT);

}
