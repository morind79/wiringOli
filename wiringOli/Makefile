# ;
# Makefile:
#        wiringOli - Wiring Compatable library for the Olinuxino A20
#
#        Copyright (c) 2013
#################################################################################
# This file is part of wiringOli:
#        http://
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

DYN_VERS_MAJ=1
DYN_VERS_MIN=0

VERSION=$(DYN_VERS_MAJ).$(DYN_VERS_MIN)
DESTDIR=/usr
PREFIX=/local

STATIC=libwiringOli.a
DYNAMIC=libwiringOli.so.$(VERSION)

#DEBUG        = -g -O0
DEBUG        = -O2
CC        = gcc
INCLUDE        = -I.
CFLAGS        = $(DEBUG) -Wall $(INCLUDE) -Winline -pipe -fPIC

LIBS    =

# Should not alter anything below this line
###############################################################################

SRC        =        wiringOli.c gpio_lib.c oliHiPri.c oliExt.c interrupt.c wiringSerial.c	\
		softPwm.c oliThread.c

SRC_I2C        =        wiringOliI2C.c

OBJ        =        $(SRC:.c=.o)

OBJ_I2C        =        $(SRC_I2C:.c=.o)

all:                $(DYNAMIC)

static:                $(STATIC)

$(STATIC):        $(OBJ)
	@echo "[Link (Static)]"
	@ar rcs $(STATIC) $(OBJ)
	@ranlib $(STATIC)
#        @size   $(STATIC)

$(DYNAMIC):        $(OBJ)
	@echo "[Link (Dynamic)]"
	@$(CC) -shared -Wl,-soname,libwiringOli.so.1 -o libwiringOli.so.1.0 -lpthread $(OBJ)

i2c:        $(OBJ) $(OBJ_I2C)
	@echo "[Link (Dynamic + I2C)]"
	@$(CC) -shared -Wl,-soname,libwiringOli.so.1 -o libwiringOli.so.1.0 -lpthread $(OBJ) $(OBJ_I2C)

.c.o:
	@echo [Compile] $<
	@$(CC) -c $(CFLAGS) $< -o $@

.PHONEY:        clean
clean:
	rm -f $(OBJ) $(OBJ_I2C) *~ core tags Makefile.bak libwiringOli.*

.PHONEY:        tags
tags:        $(SRC)
	@echo [ctags]
	@ctags $(SRC)

.PHONEY:        install
install:        $(DYNAMIC)
	@echo "[Install]"
	@install -m 0755 -d                        $(DESTDIR)$(PREFIX)/lib
	@install -m 0755 -d                        $(DESTDIR)$(PREFIX)/include
	@install -m 0644 wiringOli.h                $(DESTDIR)$(PREFIX)/include
	@install -m 0644 oliExt.h                $(DESTDIR)$(PREFIX)/include
	@install -m 0644 interrupt.h                $(DESTDIR)$(PREFIX)/include
	@install -m 0644 softPwm.h                $(DESTDIR)$(PREFIX)/include
	@install -m 0644 wiringSerial.h                $(DESTDIR)$(PREFIX)/include
	@install -m 0644 wiringOliI2C.h                $(DESTDIR)$(PREFIX)/include
	@install -m 0755 libwiringOli.so.$(VERSION)                  $(DESTDIR)$(PREFIX)/lib
	@ln -sf $(DESTDIR)$(PREFIX)/lib/libwiringOli.so.$(VERSION) $(DESTDIR)/lib/libwiringOli.so
	@ln -sf $(DESTDIR)$(PREFIX)/lib/libwiringOli.so.$(VERSION) $(DESTDIR)/lib/libwiringOli.so.1
	@ldconfig

.PHONEY:        install-static
install-static:        $(STATIC)
	@echo "[Install Static]"
	@install -m 0755 libwiringOli.a                $(DESTDIR)$(PREFIX)/lib

.PHONEY:        uninstall
uninstall:
	@echo "[UnInstall]"
	@rm -f $(DESTDIR)$(PREFIX)/include/wiringOli.h
	@rm -f $(DESTDIR)$(PREFIX)/include/oliExt.h
	@rm -f $(DESTDIR)$(PREFIX)/include/interrupt.h
	@rm -f $(DESTDIR)$(PREFIX)/include/softPwm.h
	@rm -f $(DESTDIR)$(PREFIX)/include/wiringSerial.h
	@rm -f $(DESTDIR)$(PREFIX)/include/wiringOliI2C.h
	@rm -f $(DESTDIR)$(PREFIX)/lib/libwiringOli.*
	@ldconfig


.PHONEY:        depend
depend:
	makedepend -Y $(SRC) $(SRC_I2C)

# DO NOT DELETE

wiringOli.o: wiringOli.h
oliExt.o: oliExt.h
interrupt.o: interrupt.h
softPwm.o: softPwm.h
wiringSerial.o: wiringSerial.h
oliHiPri.o: wiringOli.h
wiringOliI2C.o: wiringOli.h wiringOliI2C.h
