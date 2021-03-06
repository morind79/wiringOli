#
# Makefile:
#################################################################################
# This file is part of wiringOli:
#	Wiring Compatable library for the OliExt board
#
#    wiringOli is free software: you can redistribute it and/or modify
#    it under the terms of the GNU Lesser General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    wiringOli is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU Lesser General Public License for more details.
#
#    You should have received a copy of the GNU Lesser General Public License
#    along with wiringOli.  If not, see <http://www.gnu.org/licenses/>.
#################################################################################

DESTDIR=/usr
PREFIX=/local

#DEBUG	= -g -O0
DEBUG	= -O2
CC	= gcc
INCLUDE	= -I$(DESTDIR)$(PREFIX)/include
CFLAGS	= $(DEBUG) -Wall $(INCLUDE) -Winline -pipe

LDFLAGS	= -L$(DESTDIR)$(PREFIX)/lib
LIBS    = -lwiringOli -lpthread -lm

# May not need to  alter anything below this line
###############################################################################

SRC	=	gpio.c

OBJ	=	$(SRC:.c=.o)

all:		gpio

gpio:	$(OBJ)
	@echo [Link]
	@$(CC) -o $@ $(OBJ) $(LDFLAGS) $(LIBS)
	
.c.o:
	@echo [Compile] $<
	@$(CC) -c $(CFLAGS) $< -o $@

.PHONEY:	clean
clean:
	@echo "[Clean]"
	@rm -f $(OBJ) gpio *~ core tags *.bak

.PHONEY:	tags
tags:	$(SRC)
	@echo [ctags]
	@ctags $(SRC)

.PHONEY:	install
install:
	@echo "[Install]"
	@cp gpio		$(DESTDIR)$(PREFIX)/bin
	@chown root.root	$(DESTDIR)$(PREFIX)/bin/gpio
	@chmod 4755		$(DESTDIR)$(PREFIX)/bin/gpio
	@mkdir -p		$(DESTDIR)$(PREFIX)/man/man1
	@cp gpio.1		$(DESTDIR)$(PREFIX)/man/man1

.PHONEY:	uninstall
uninstall:
	@echo "[UnInstall]"
	@rm -f $(DESTDIR)$(PREFIX)/bin/gpio
	@rm -f $(DESTDIR)$(PREFIX)/man/man1/gpio.1

.PHONEY:	depend
depend:
	makedepend -Y $(SRC)

# DO NOT DELETE

