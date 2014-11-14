/*
 * testRS232.c:
 *        Test serial port communication
 *        This is the UART0 (RX0 TX0)
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
 
#include <stdio.h>
#include <wiringOli.h>
#include <string.h>
#include <wiringSerial.h>
#include <errno.h>

int main()
{
  int fd;
  int c;

  if ((fd = serialOpen("/dev/ttyS2", 9600, 8, 1)) < 0)
  {
    fprintf(stderr, "Unable to open serial device: %s\n", strerror(errno));
    return 1;
  }
  serialPuts(fd, "Test\n");
// Loop, getting and printing characters

  for (;;)
  {
    c = serialGetchar(fd);
    putchar(c);
    printf("%c", c);
    fflush(stdout);
  }
}
