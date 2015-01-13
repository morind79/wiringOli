/*
 * wiringOli.cpp
 *
 *  Created on: 14 oct. 2013
 *      Author: denm
 */

#include <stdio.h>
#include <stdint.h>
#include <poll.h>
#include <time.h>
#include <sys/time.h>

#include "wiringOli.h"
#include "interrupt.h"

// Pin definition
static int pinToGpio[128] =
{
  SUNXI_GPG(0), SUNXI_GPG(1), SUNXI_GPG(2), SUNXI_GPG(3), SUNXI_GPG(4), SUNXI_GPG(5),        // GPIO-1 (12 I/O pins)
  SUNXI_GPG(6), SUNXI_GPG(7), SUNXI_GPG(8), SUNXI_GPG(9), SUNXI_GPG(10), SUNXI_GPG(11),
  SUNXI_GPI(0), SUNXI_GPI(1), SUNXI_GPI(2), SUNXI_GPI(3), SUNXI_GPI(10), SUNXI_GPI(11),      // GPIO-2 (29 I/O pins)
  SUNXI_GPI(14), SUNXI_GPI(15),
  SUNXI_GPC(3), SUNXI_GPC(7), SUNXI_GPC(16), SUNXI_GPC(17), SUNXI_GPC(18), SUNXI_GPC(23),
  SUNXI_GPC(24), SUNXI_GPE(0), SUNXI_GPE(1), SUNXI_GPE(2), SUNXI_GPE(3), SUNXI_GPE(4),
  SUNXI_GPE(5), SUNXI_GPE(6), SUNXI_GPE(7), SUNXI_GPE(8), SUNXI_GPE(9), SUNXI_GPE(10),
  SUNXI_GPE(11), SUNXI_GPI(14), SUNXI_GPI(15),
  SUNXI_GPH(0), SUNXI_GPH(2), SUNXI_GPH(7), SUNXI_GPH(9), SUNXI_GPH(10), SUNXI_GPH(11),      // GPIO-3 (40 I/O pins)
  SUNXI_GPH(12), SUNXI_GPH(13), SUNXI_GPH(14), SUNXI_GPH(15), SUNXI_GPH(16), SUNXI_GPH(17),
  SUNXI_GPH(18), SUNXI_GPH(19), SUNXI_GPH(20), SUNXI_GPH(21), SUNXI_GPH(22), SUNXI_GPH(23),
  SUNXI_GPH(24), SUNXI_GPH(25), SUNXI_GPH(26), SUNXI_GPH(27),
  SUNXI_GPB(3), SUNXI_GPB(4), SUNXI_GPB(5), SUNXI_GPB(6), SUNXI_GPB(7), SUNXI_GPB(8),
  SUNXI_GPB(10), SUNXI_GPB(11), SUNXI_GPB(12), SUNXI_GPB(13), SUNXI_GPB(14), SUNXI_GPB(15),
  SUNXI_GPB(16), SUNXI_GPB(17), SUNXI_GPH(24), SUNXI_GPH(25), SUNXI_GPH(26), SUNXI_GPH(27),  
  SUNXI_GPB(1), SUNXI_GPB(0),                                                                // GPIO-2 I2C0 - SDA, SCL
  SUNXI_GPB(22), SUNXI_GPB(23),                                                              //        UART0 - Tx, Rx
  SUNXI_GPI(12), SUNXI_GPI(13),                                                              // UEXT1  UART6 - Tx, Rx
  SUNXI_GPB(21), SUNXI_GPB(20),                                                              //        I2C2 - SDA, SCL
  SUNXI_GPC(21), SUNXI_GPC(22), SUNXI_GPC(20), SUNXI_GPC(19),                                //        SPI2 - MOSI, MISO, CLK, CS0
  SUNXI_GPI(20), SUNXI_GPI(21),                                                              // UEXT2  UART7 - Tx, Rx
  SUNXI_GPB(19), SUNXI_GPB(18),                                                              //        I2C1 - SDA, SCL
  SUNXI_GPI(18), SUNXI_GPI(19), SUNXI_GPI(17), SUNXI_GPI(16),                                //        SPI1 - MOSI, MISO, CLK, CS0

// Padding:

  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,                            // ... 111
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,                                                    // ... 127
};

// Time for easy calculations
static uint64_t epochMilli, epochMicro;

/*
 * pinWiringOli:
 *	Get board pin corresponding to wiringOli pin numbering
 *********************************************************************************
 */
int pinWiringOli(int pin)
{
  // To be sure to have a correct pin number
  return pinToGpio[pin & 127];
}

/*
 * pinMode:
 *	Sets the mode of a pin to be INPUT, OUTPUT
 *********************************************************************************
 */
void pinMode(int pin, int mode)
{
  sunxi_gpio_set_cfgpin(pinWiringOli(pin), mode);
}

/*
 * pullUpDownCtrl:
 *	Control the internal pull-up/down resistors on a GPIO pin
 *      pud=0 -> pull disable 1 -> pull-up 2 -> pull-down
 *********************************************************************************
 */

void pullUpDnControlGpio(int pin, int pud)
{
  sunxi_gpio_set_pull(pinWiringOli(pin), pud);
}

/*
 * digitalRead:
 *	Read the value of a given Pin, returning HIGH or LOW
 *********************************************************************************
 */
int  digitalRead(int pin)
{
  return(sunxi_gpio_input(pinWiringOli(pin)));
}

/*
 * digitalWrite:
 *	Set an output bit
 *********************************************************************************
 */
void digitalWrite(int pin, int value)
{
  sunxi_gpio_output(pinWiringOli(pin), value);
}

/*
 * waitForInterrupt:
 *	Wait for interrupt on a GPIO pin
 *********************************************************************************
 */
int waitForInterrupt(int pin, int mS)
{
  struct pollfd polls;
  int gpio_fd, rc;
  uint8_t c ;

  // Get pin A20 number
  pin = pinWiringOli(pin);

  gpio_export(pin);
  gpio_set_dir(pin, 0);
  // Set interrupt edge, can be "none", "rising", "falling", or "both"
  gpio_set_edge(pin, "rising");
  gpio_fd = gpio_fd_open(pin);

	polls.fd = gpio_fd;
	polls.events = POLLPRI;

	rc = poll(&polls, 1, mS);      

	(void)read(polls.fd, &c, 1);

	gpio_fd_close(gpio_fd);
	return rc;
}

/*
 * initialiseEpoch:
 *	Initialise our start-of-time variable to be the current unix
 *	time in milliseconds and microseconds.
 *********************************************************************************
 */

static void initialiseEpoch (void)
{
  struct timeval tv ;

  gettimeofday (&tv, NULL) ;
  epochMilli = (uint64_t)tv.tv_sec * (uint64_t)1000    + (uint64_t)(tv.tv_usec / 1000) ;
  epochMicro = (uint64_t)tv.tv_sec * (uint64_t)1000000 + (uint64_t)(tv.tv_usec) ;
}

/*
 * delay:
 *	Wait for some number of milliseconds
 *********************************************************************************
 */

void delay (unsigned int howLong)
{
  struct timespec sleeper, dummy ;

  sleeper.tv_sec  = (time_t)(howLong / 1000) ;
  sleeper.tv_nsec = (long)(howLong % 1000) * 1000000 ;

  nanosleep (&sleeper, &dummy) ;
}

void delayMicrosecondsHard (unsigned int howLong)
{
  struct timeval tNow, tLong, tEnd ;

  gettimeofday (&tNow, NULL) ;
  tLong.tv_sec  = howLong / 1000000 ;
  tLong.tv_usec = howLong % 1000000 ;
  timeradd (&tNow, &tLong, &tEnd) ;

  while (timercmp (&tNow, &tEnd, <))
    gettimeofday (&tNow, NULL) ;
}

void delayMicroseconds (unsigned int howLong)
{
  struct timespec sleeper ;
  unsigned int uSecs = howLong % 1000000 ;
  unsigned int wSecs = howLong / 1000000 ;

  /**/ if (howLong ==   0)
    return ;
  else if (howLong  < 100)
    delayMicrosecondsHard (howLong) ;
  else
  {
    sleeper.tv_sec  = wSecs ;
    sleeper.tv_nsec = (long)(uSecs * 1000L) ;
    nanosleep (&sleeper, NULL) ;
  }
}

/*
 * millis:
 *	Return a number of milliseconds as an unsigned int.
 *********************************************************************************
 */

unsigned int millis(void)
{
  struct timeval tv;
  uint64_t now;

  gettimeofday(&tv, NULL);
  now  = (uint64_t)tv.tv_sec * (uint64_t)1000 + (uint64_t)(tv.tv_usec / 1000);

  return (uint32_t)(now - epochMilli);
}


/*
 * micros:
 *	Return a number of microseconds as an unsigned int.
 *********************************************************************************
 */

unsigned int micros(void)
{
  struct timeval tv ;
  uint64_t now ;

  gettimeofday (&tv, NULL) ;
  now  = (uint64_t)tv.tv_sec * (uint64_t)1000000 + (uint64_t)tv.tv_usec ;

  return (uint32_t)(now - epochMicro) ;
}

/*
 * wiringOliSetup:
 *	Must be called once at the start of your program execution.
 *
 * Default setup: Initialises the system into wiringOli Pin mode and uses the
 *	memory mapped hardware directly.
 *********************************************************************************
 */

int wiringOliSetup(void)
{

  initialiseEpoch() ;
  int result;
  result = sunxi_gpio_init();
  if(result == SETUP_DEVMEM_FAIL) {
	printf("No access to /dev/mem. Try running as root !");
	return SETUP_DEVMEM_FAIL;
  }
  else if(result == SETUP_MALLOC_FAIL) {
	printf("No memory !");
	return SETUP_MALLOC_FAIL;
  }
  else if(result == SETUP_MMAP_FAIL) {
	printf("Mmap failed on module import");
	return SETUP_MMAP_FAIL;
  }
  else 
  {
	return SETUP_OK;
  }

  return SETUP_OK;
}

