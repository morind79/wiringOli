/*
 * oliExt.h
 *
 *  Created on: 14 oct. 2013
 *      Author: denm
 */

#ifndef OLIEXT_H_
#define OLIEXT_H_

#ifdef __cplusplus
extern "C" {
#endif

extern int pinOliPortA(int pin);
extern int pinOliPortB(int pin);
extern int pinOliPortC(int pin);
extern int pinOliPortD(int pin);
extern void pinModePortA(int pin, int mode);
extern void pinModePortB(int pin, int mode);
extern void pinModePortC(int pin, int mode);
extern void pinModePortD(int pin, int mode);
extern void pullUpDnCtrlPortA(int pin, int pud);
extern void pullUpDnCtrlPortB(int pin, int pud);
extern void pullUpDnCtrlPortC(int pin, int pud);
extern void pullUpDnCtrlPortD(int pin, int pud);
extern int digitalReadPortA(int pin);
extern int digitalReadPortB(int pin);
extern int digitalReadPortC(int pin);
extern int digitalReadPortD(int pin);
extern void digitalWritePortA(int pin, int value);
extern void digitalWritePortB(int pin, int value);
extern void digitalWritePortC(int pin, int value);
extern void digitalWritePortD(int pin, int value);

#ifdef __cplusplus
}
#endif

extern unsigned int SUNXI_PIO_BASE;
#endif /* OLIEXT_H_ */
