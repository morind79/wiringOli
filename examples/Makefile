#
# Makefile:
#        wiringOli - Wiring Compatable library for the Olinuxino A20
#        http://morind.free.fr
#
#        Copyright (c) 2013
#################################################################################
# This file is part of wiringOli:
#        Wiring Compatable library for the Olinuxino A20
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
#    along with wiringPi.  If not, see <http://www.gnu.org/licenses/>.
#################################################################################


#DEBUG        = -g -O0
DEBUG        = -O3
CC        = gcc
INCLUDE        = -I/usr/local/include
CFLAGS        = $(DEBUG) -Wall $(INCLUDE) -Winline -pipe

LDFLAGS        = -L/usr/local/lib
LDLIBS    = -lwiringOli -lpthread -lm

# Should not alter anything below this line
###############################################################################

SRC        =        testPortA.c testPortB.c testPortC.c testPortD.c testDisplay.c testI2CLCD.c testRS232.c testGSM.c testGSM_SendSMS.c	\
		testGSM_Call.c testGSM_ReceiveSMS.c

OBJ        =        $(SRC:.c=.o)

BINS        =        $(SRC:.c=)

all:        
	@cat README.TXT
	@echo "    $(BINS)" | fmt
	@echo ""

really-all:        $(BINS)

testPortA:        testPortA.o
	@echo [link]
	@$(CC) -o $@ testPortA.o $(LDFLAGS) $(LDLIBS)

testPortB:        testPortB.o
	@echo [link]
	@$(CC) -o $@ testPortB.o $(LDFLAGS) $(LDLIBS)

testPortC:        testPortC.o
	@echo [link]
	@$(CC) -o $@ testPortC.o $(LDFLAGS) $(LDLIBS)

testPortD:        testPortD.o
	@echo [link]
	@$(CC) -o $@ testPortD.o $(LDFLAGS) $(LDLIBS)

testDisplay:        testDisplay.o
	@echo [link]
	@$(CC) -o $@ testDisplay.o $(LDFLAGS) $(LDLIBS)

testI2CLCD:        testI2CLCD.o
	@echo [link]
	@$(CC) -o $@ testI2CLCD.o $(LDFLAGS) $(LDLIBS)

testRS232:        testRS232.o
	@echo [link]
	@$(CC) -o $@ testRS232.o $(LDFLAGS) $(LDLIBS)

testGSM:        testGSM.o
	@echo [link]
	@$(CC) -o $@ testGSM.o $(LDFLAGS) $(LDLIBS)

testGSM_SendSMS:        testGSM_SendSMS.o
	@echo [link]
	@$(CC) -o $@ testGSM_SendSMS.o $(LDFLAGS) $(LDLIBS)

testGSM_Call:        testGSM_Call.o
	@echo [link]
	@$(CC) -o $@ testGSM_Call.o $(LDFLAGS) $(LDLIBS)

testGSM_ReceiveSMS:        testGSM_ReceiveSMS.o
	@echo [link]
	@$(CC) -o $@ testGSM_ReceiveSMS.o $(LDFLAGS) $(LDLIBS)

.c.o:
	@echo [CC] $<
	@$(CC) -c $(CFLAGS) $< -o $@

clean:
	@echo "[Clean]"
	rm -f $(OBJ) *~ core tags $(BINS)

tags:        $(SRC)
	@echo [ctags]
	@ctags $(SRC)

depend:
	makedepend -Y $(SRC)

# DO NOT DELETE
