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
  wiringOliI2CWriteReg8(LCDDev.devAddr, 0x00, value | LCDDev.backLightEn);
  //unsigned char buff[1];
  //buff[0] = value | LCDDev.backLightEn;
  //i2c_write(0x00, buff, 1);
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
 * clear :
 *	Clear LCD
 *********************************************************************************
 */
void clear(void)
{
    writeCmd(CLEARDISPLAY);
    delayMicroseconds(50000);
}

/*
 * home :
 *	Set cursor to home position on LCD
 *********************************************************************************
 */
void home(void)
{
    writeCmd(RETURNHOME);
    delayMicroseconds(50000);
}

/*
 * setCursor :
 *	Set cursor to specified position on LCD
 *********************************************************************************
 */
void setCursor(unsigned char row, unsigned char col)
{
    unsigned char rowOffset[] = {0x00, 0x40, 0x14, 0x54};
    if (col<LCDDev.numCols && row<LCDDev.numRows)
    {
        writeCmd(SETDDRAMADDR | (col + rowOffset[row]));
    }
}

/*
 * displayOff :
 *	Turn off diplay
 *********************************************************************************
 */
void displayOff(void)
{
    LCDDev.displayControl &= ~DISPLAYON;
    writeCmd(DISPLAYCONTROL | LCDDev.displayControl);
}

/*
 * displayOn :
 *	Turn on diplay
 *********************************************************************************
 */
void displayOn(void)
{
    LCDDev.displayControl |= DISPLAYON;
    writeCmd(DISPLAYCONTROL | LCDDev.displayControl);
}

/*
 * cursorOff :
 *	Turn off cursor display
 *********************************************************************************
 */
void cursorOff(void)
{
    LCDDev.displayControl &= ~CURSORON;
    writeCmd(DISPLAYCONTROL | LCDDev.displayControl);
}

/*
 * cursorOn :
 *	Turn on cursor display
 *********************************************************************************
 */
void cursorOn(void)
{
    LCDDev.displayControl |= CURSORON;
    writeCmd(DISPLAYCONTROL | LCDDev.displayControl);
}

/*
 * blinkOff :
 *	Turn off blink
 *********************************************************************************
 */
void blinkOff(void)
{
    LCDDev.displayControl &= ~BLINKON;
    writeCmd(DISPLAYCONTROL | LCDDev.displayControl);
}

/*
 * blinkOn :
 *	Turn on blink
 *********************************************************************************
 */
void blinkOn(void)
{
    LCDDev.displayControl |= BLINKON;
    writeCmd(DISPLAYCONTROL | LCDDev.displayControl);
}

/*
 * scroolDisplayLeft :
 *	Scroll display content to left by one character
 *********************************************************************************
 */
void scrollDisplayLeft(void)
{
    writeCmd(DISPLAYMOVE | CURSORSHIFT | MOVELEFT);
}

/*
 * scroolDisplayRight :
 *	Scroll display content to right by one character
 *********************************************************************************
 */
void scrollDisplayRight(void)
{
    writeCmd(DISPLAYMOVE | CURSORSHIFT | MOVERIGHT);
}

/*
 * autoScrollOff :
 *	Turn off auto scroll
 *********************************************************************************
 */
void autoScrollOff(void)
{
    LCDDev.displayMode &= ~ENTRYSHIFTINC;
    writeCmd(ENTRYMODESET | LCDDev.displayMode);
}

/*
 * autoScrollOn :
 *	Turn on auto scroll
 *********************************************************************************
 */
void autoScrollOn(void)
{
    LCDDev.displayMode |= ENTRYSHIFTINC;
    writeCmd(ENTRYMODESET | LCDDev.displayMode);
}

/*
 * backlightOff :
 *	Turn off back light
 *********************************************************************************
 */
void backlightOff(void)
{
    LCDDev.backLightEn=BACKLIGHTOFF;
    expandWrite(0);
}

/*
 * backlightOn :
 *	Turn on back light
 *********************************************************************************
 */
void backlightOn(void)
{
    LCDDev.backLightEn=BACKLIGHTON;
    expandWrite(0);
}

/*
 * createChar :
 *	Create a user defined character
 *********************************************************************************
 */
void createChar(unsigned char location, unsigned char charMap[])
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
 * showChar :
 *	Show a character
 *********************************************************************************
 */
void showChar(unsigned char row, unsigned char col, char ch)
{
    if (col < LCDDev.numCols && row < LCDDev.numRows)
    {
        setCursor(row, col);
        writeData(ch);
    }
}

/*
 * printString :
 *	Show a string
 *********************************************************************************
 */
void printString(unsigned char row, unsigned char col, char *pStr)
{
    char *p;
    p = pStr;
    if (col < LCDDev.numCols && row < LCDDev.numRows)
    {
        while (*p)
        {
            showChar(row, col, *p);
            p++;
            col++;
            if (col >= 16)
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
 * lcdInit :
 *	Initialise the LCD, and return a handle to
 *	that LCD, or -1 if any error.
 *********************************************************************************
 */
void LCDinit()
{

    LCDDev.devAddr=0x27;
    //i2c_init(0x27, 0);
    wiringOliI2CSetup(0x27);
    LCDDev.numRows=4;
    LCDDev.numCols=16;
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
    displayOn();
    cursorOn();
    clear();
    autoScrollOff();
    home();
}
