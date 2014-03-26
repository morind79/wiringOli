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
static int pinSIM900[4] = {21, 17, 20, 16}; // PC7, PI11, PC3, PI10

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
 * digitalReadportA :
 *	Read the value of a given PortA pin, returning HIGH or LOW
 *********************************************************************************
 */
int  digitalReadPortA(int pin)
{
  return(digitalRead(pinOliPortA(pin)));
}

/*
 * digitalReadportB :
 *	Read the value of a given PortB pin, returning HIGH or LOW
 *********************************************************************************
 */
int  digitalReadPortB(int pin)
{
  return(digitalRead(pinOliPortB(pin)));
}

/*
 * digitalReadportC :
 *	Read the value of a given PortC pin, returning HIGH or LOW
 *********************************************************************************
 */
int  digitalReadPortC(int pin)
{
  return(digitalRead(pinOliPortC(pin)));
}

/*
 * digitalReadportD :
 *	Read the value of a given PortD pin, returning HIGH or LOW
 *********************************************************************************
 */
int  digitalReadPortD(int pin)
{
  return(digitalRead(pinOliPortD(pin)));
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
