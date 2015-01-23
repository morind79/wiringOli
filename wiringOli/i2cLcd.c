/*
 * i2cLcd.c:
 *	Handle an I2C LCD
 ***********************************************************************
 * This file is part of wiringOli:
 *	https://
 *
 *    wiringOli is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU Lesser General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    wiringoli is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public License
 *    along with wiringOli.  If not, see <http://www.gnu.org/licenses/>.
 ***********************************************************************
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <wiringOli.h>
#include <wiringOliI2C.h>

#include "i2cLcd.h"

static LCDConf LCDDev;

/*
 * expandWrite :
 *	Write data to LCD.
 *********************************************************************************
 */
static void expandWrite(unsigned char value)
{
  wiringOliI2CWriteReg8(LCDDev.fd, 0x00, value | LCDDev.backLightEn);
  //unsigned char buff[10];
  //buff[0] = 0x00;		//Pointer to the first register
  //buff[1] = value | LCDDev.backLightEn;
  //I2C_Send(LCDDev.fd, buff, 2);
}

/*
 * pulseEnable :
 *	This generate the real "E" signal on the LCD.
 *	According to the docs, data is latched on the falling edge.
 *********************************************************************************
 */
static void pulseEnable(unsigned char value)
{
  expandWrite(value | EN);
  delayMicroseconds(50);
  expandWrite(value & ~EN);
  delayMicroseconds(50);
}

/*
 * write4Bits :
 *	This function write 4 bits to LCD. Since we only use 4 bits data bus
 *      of LCD, each time of write can only write 4 bits data. The higher 4 bits
 *      are data bits to be written in LCD and the lower 4 bits are control bits
 *      to control the read/write time sequence and back light the real "E" signal on the LCD.
 *********************************************************************************
 */
static void write4Bits(unsigned char value)
{
  expandWrite(value);
  pulseEnable(value);
}

/*
 * writeCmd :
 *	This function is to write command on the LCD
 *********************************************************************************
 */
static void writeCmd(unsigned char value)
{
  unsigned char tmp;
  tmp = value & 0xF0;
  write4Bits(tmp);
  tmp = value << 4;
  write4Bits(tmp);
}

/*
 * writeData :
 *	This function is to write value to DDRAM or CGRAM on the LCD
 *********************************************************************************
 */
static void writeData(unsigned char value)
{
  unsigned char tmp;
  tmp = value & 0xF0;
  write4Bits(tmp | RS);
  tmp = value << 4;
  write4Bits(tmp | RS);
}

/*
 * LCDClear :
 *	Clear LCD
 *********************************************************************************
 */
void LCDClear(void)
{
  writeCmd(CLEARDISPLAY);
  delayMicroseconds(50000);
}

/*
 * LCDHome :
 *	Set cursor to home position on LCD
 *********************************************************************************
 */
void LCDHome(void)
{
  writeCmd(RETURNHOME);
  delayMicroseconds(50000);
}

/*
 * LCDSetCursor :
 *	Set cursor to specified position on LCD
 *********************************************************************************
 */
void LCDSetCursor(int row, int col)
{
  unsigned char rowOffset[] = {0x00, 0x40, 0x14, 0x54};
  if (col < LCDDev.numCols && row < LCDDev.numRows)
  {
    writeCmd(SETDDRAMADDR | (col + rowOffset[row]));
  }
}

/*
 * LCDDisplayOff :
 *	Turn off diplay
 *********************************************************************************
 */
void LCDDisplayOff(void)
{
  LCDDev.displayControl &= ~DISPLAYON;
  writeCmd(DISPLAYCONTROL | LCDDev.displayControl);
}

/*
 * LCDDisplayOn :
 *	Turn on diplay
 *********************************************************************************
 */
void LCDDisplayOn(void)
{
  LCDDev.displayControl |= DISPLAYON;
  writeCmd(DISPLAYCONTROL | LCDDev.displayControl);
}

/*
 * LCDCursorOff :
 *	Turn off cursor display
 *********************************************************************************
 */
void LCDCursorOff(void)
{
  LCDDev.displayControl &= ~CURSORON;
  writeCmd(DISPLAYCONTROL | LCDDev.displayControl);
}

/*
 * LCDCursorOn :
 *	Turn on cursor display
 *********************************************************************************
 */
void LCDCursorOn(void)
{
  LCDDev.displayControl |= CURSORON;
  writeCmd(DISPLAYCONTROL | LCDDev.displayControl);
}

/*
 * LCDBlinkOff :
 *	Turn off blink
 *********************************************************************************
 */
void LCDBlinkOff(void)
{
  LCDDev.displayControl &= ~BLINKON;
  writeCmd(DISPLAYCONTROL | LCDDev.displayControl);
}

/*
 * LCDBlinkOn :
 *	Turn on blink
 *********************************************************************************
 */
void LCDBlinkOn(void)
{
  LCDDev.displayControl |= BLINKON;
  writeCmd(DISPLAYCONTROL | LCDDev.displayControl);
}

/*
 * LCDScroolDisplayLeft :
 *	Scroll display content to left by one character
 *********************************************************************************
 */
void LCDScrollDisplayLeft(void)
{
  writeCmd(DISPLAYMOVE | CURSORSHIFT | MOVELEFT);
}

/*
 * LCDScroolDisplayRight :
 *	Scroll display content to right by one character
 *********************************************************************************
 */
void LCDScrollDisplayRight(void)
{
  writeCmd(DISPLAYMOVE | CURSORSHIFT | MOVERIGHT);
}

/*
 * LCDAutoScrollOff :
 *	Turn off auto scroll
 *********************************************************************************
 */
void LCDAutoScrollOff(void)
{
  LCDDev.displayMode &= ~ENTRYSHIFTINC;
  writeCmd(ENTRYMODESET | LCDDev.displayMode);
}

/*
 * LCDAutoScrollOn :
 *	Turn on auto scroll
 *********************************************************************************
 */
void LCDAutoScrollOn(void)
{
  LCDDev.displayMode |= ENTRYSHIFTINC;
  writeCmd(ENTRYMODESET | LCDDev.displayMode);
}

/*
 * LCDBacklightOff :
 *	Turn off back light
 *********************************************************************************
 */
void LCDBacklightOff(void)
{
  LCDDev.backLightEn=BACKLIGHTOFF;
  expandWrite(0);
}

/*
 * LCDBacklightOn :
 *	Turn on back light
 *********************************************************************************
 */
void LCDBacklightOn(void)
{
  LCDDev.backLightEn=BACKLIGHTON;
  expandWrite(0);
}

/*
 * LCDCreateChar :
 *	Create a user defined character
 *********************************************************************************
 */
void LCDCreateChar(int location, int charMap[])
{
  unsigned char i;
  location &= 0x07;
  writeCmd(SETCGRAMADDR | location << 3);
  for (i=0; i<8; i++)
  {
    writeData(charMap[i]);
  }
}

/*
 * LCDShowChar :
 *	Show a character
 *********************************************************************************
 */
void LCDShowChar(int row, int col, char ch)
{
  if (col < LCDDev.numCols && row < LCDDev.numRows)
  {
    LCDSetCursor(row, col);
    writeData(ch);
  }
}

/*
 * LCDPrintString :
 *	Show a string
 *********************************************************************************
 */
void LCDPrintString(int row, int col, char *pStr)
{
  char *p;
  p = pStr;
  if (col < LCDDev.numCols && row < LCDDev.numRows)
  {
    while (*p)
    {
      LCDShowChar(row, col, *p);
      p++;
      col++;
      if (col >= LCDDev.numCols)
      {
      	col=0;
      	row++;
      }
      if (row>1)
      {
        row=0;
      }
    }
  }
}

/*
 * lCDInit :
 *	Initialise the LCD
 *      Specify device address, row and columns
 *********************************************************************************
 */
void LCDInit(int address, int row, int col)
{
  int fd;
  LCDDev.devAddr=address;
  // Bus 0 at address specified
  fd = wiringOliI2CSetup(0, LCDDev.devAddr);
  if (fd < 0)
  {
    printf("Error initializing I2C bus");
    return;
  }
  LCDDev.fd = fd;
  LCDDev.numRows=row;
  LCDDev.numCols=col;
  delayMicroseconds(50000);
  expandWrite(LCDDev.backLightEn);
  LCDDev.backLightEn=BACKLIGHTON;
  LCDDev.dispalyFunction |= TWOLINE;
  delayMicroseconds(5000);
  //
  // we start in 8bit mode, try to set 4 bit mode
  //
  write4Bits(0x30);
  delayMicroseconds(50000);
  write4Bits(0x30);
  delayMicroseconds(50000);
  write4Bits(0x30);
  delayMicroseconds(50000);
  //
  // set 4bit mode
  //
  write4Bits(0x20);
  delayMicroseconds(50000);

  writeCmd(FUNCTIONSET | LCDDev.dispalyFunction);
  LCDDisplayOn();
  LCDCursorOn();
  LCDClear();
  LCDAutoScrollOff();
  LCDHome();
}
