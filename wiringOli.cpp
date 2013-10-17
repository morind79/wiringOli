/*
 * wiringOli.cpp
 *
 *  Created on: 14 oct. 2013
 *      Author: denm
 */

#include <stdio.h>
#include <iostream>
#include <stdint.h>
#include <time.h>
#include <sys/time.h>

#include "wiringOli.h"
#include "gpio_lib.h"

// Time for easy calculations

static uint64_t epochMilli, epochMicro ;

/*
 * pinMode:
 *	Sets the mode of a pin to be INPUT, OUTPUT
 *********************************************************************************
 */
void pinMode(int pin, int mode)
{
  sunxi_gpio_set_cfgpin(pin, mode);
}

/*
 * digitalRead:
 *	Read the value of a given Pin, returning HIGH or LOW
 *********************************************************************************
 */
int  digitalRead(int pin)
{
  return(sunxi_gpio_input(pin));
}

/*
 * digitalWrite:
 *	Set an output bit
 *********************************************************************************
 */
void digitalWrite(int pin, int value)
{
  sunxi_gpio_output(pin, value);
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

unsigned int millis (void)
{
  struct timeval tv ;
  uint64_t now ;

  gettimeofday (&tv, NULL) ;
  now  = (uint64_t)tv.tv_sec * (uint64_t)1000 + (uint64_t)(tv.tv_usec / 1000) ;

  return (uint32_t)(now - epochMilli) ;
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
 * wiringPiSetup:
 *	Must be called once at the start of your program execution.
 *
 * Default setup: Initialises the system into wiringPi Pin mode and uses the
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
  else {
	  return SETUP_OK;
  }

  return SETUP_OK;
}

