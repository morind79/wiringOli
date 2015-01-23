/*
 * i2cLcd.h:
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
 *    wiringOli is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public License
 *    along with wiringOli.  If not, see <http://www.gnu.org/licenses/>.
 ***********************************************************************
 */

#ifdef __cplusplus
extern "C" {
#endif

#define EN 4  // Enable bit
#define RW 2  // Read/Write bit
#define RS 1  // Register select bit

typedef struct LCD
{
  unsigned char devAddr;            // Device address
  int fd;                           // File descriptor of I2C stream
  unsigned char dispalyFunction;    // Display function configuration data
  unsigned char displayControl;     // Display control configuration data
  unsigned char displayMode;        // Display mode configuration data
  unsigned char numCols;            // Characters one line can display
  unsigned char numRows;            // Lines LCD can display
  unsigned char backLightEn;        // LCD back light configuration data
}LCDConf;

#define CLEARDISPLAY   0x01
#define RETURNHOME     0x02
#define ENTRYMODESET   0x04
#define DISPLAYCONTROL 0x08
#define CURSORSHIFT    0x10
#define FUNCTIONSET    0x20
#define SETCGRAMADDR   0x40
#define SETDDRAMADDR   0x80

#define ENTRYSHIFTINC  0x01
#define ENTRYSHIFTDEC  0x00
#define TWOLINE        0x08
#define ONELINE        0x00

#define BLINKON        0x01
#define CURSORON       0x02
#define DISPLAYON      0x04

#define DISPLAYMOVE    0x08
#define CURSORMOVE     0x00
#define MOVERIGHT      0x04
#define MOVELEFT       0x00

#define BACKLIGHTON    0x08
#define BACKLIGHTOFF   0x00

extern void LCDClear(void);
extern void LCDHome(void);
extern void LCDDisplayOff(void);
extern void LCDDisplayOn(void);
extern void LCDBlinkOff(void);
extern void LCDBlinkOn(void);
extern void LCDCursorOff(void);
extern void LCDCursorOn(void);
extern void LCDScrollDisplayLeft(void);
extern void LCDScrollDisplayRight(void);
extern void LCDBacklightOff(void);
extern void LCDBacklightOn(void);
extern void LCDAutoScrollOff(void);
extern void LCDAutoScrollOn(void);
extern void LCDCreateChar(int location, int charMap[]);
extern void LCDInit(int address, int row, int col);
extern void LCDSetCursor(int row, int col);
extern void LCDShowChar(int row, int col, char ch);
extern void LCDPrintString(int row, int col, char *pStr);

#ifdef __cplusplus
}
#endif
