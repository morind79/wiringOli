#!/bin/bash
#
# test.sh:
#	Simple test: Assumes LEDs on port A pin 0-15 and lights them	in-turn.
#################################################################################
# This file is part of wiringOli:
#	Wiring Compatable library for the OliExt
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

# Simple test - assumes LEDs on port A pins 0-15.

for i in `seq 0 15`;
do
  gpio mode a $i output
done

while true;
do
  for i in `seq 0 15`;
  do
    gpio write a $i 1
    sleep 0.1
  done

  for i in `seq 0 15`;
  do
    gpio write a $i 0
    sleep 0.1
  done
done
