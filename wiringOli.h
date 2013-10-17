/*
 * wiringOli.h
 *
 *  Created on: 14 oct. 2013
 *      Author: denm
 */

#ifndef WIRINGOLI_H_
#define WIRINGOLI_H_

#ifdef __cplusplus
extern "C" {
#endif

extern int wiringOliSetup(void);

extern void pinMode(int pin, int mode);
extern int digitalRead(int pin);
extern void digitalWrite(int pin, int value);

// Extras from arduino land
extern void delay(unsigned int howLong);
extern void delayMicroseconds(unsigned int howLong);
extern unsigned int millis(void);
extern unsigned int micros(void);

#ifdef __cplusplus
}
#endif

#endif /* WIRINGOLI_H_ */
