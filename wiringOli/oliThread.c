/*
 * oliThread.c:
 *	Provide a simplified interface to pthreads
 *
 *	Copyright (c) 2014
 ***********************************************************************
 * This file is part of wiringOli:
 *	https://
 *
 *    wiringOli is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU Lesser General Public License as
 *    published by the Free Software Foundation, either version 3 of the
 *    License, or (at your option) any later version.
 *
 *    wiringOli is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with wiringOli.
 *    If not, see <http://www.gnu.org/licenses/>.
 ***********************************************************************
 */

#include <pthread.h>
#include "wiringOli.h"

static pthread_mutex_t oliMutexes [4] ;



/*
 * oliThreadCreate:
 *	Create and start a thread
 *********************************************************************************
 */

int oliThreadCreate(void *(*fn)(void *))
{
  pthread_t myThread;

  return pthread_create(&myThread, NULL, fn, NULL);
}

/*
 * oliLock: oliUnlock:
 *	Activate/Deactivate a mutex.
 *	We're keeping things simple here and only tracking 4 mutexes which
 *	is more than enough for out entry-level pthread programming
 *********************************************************************************
 */

void oliLock(int key)
{
  pthread_mutex_lock(&oliMutexes[key]);
}

void oliUnlock(int key)
{
  pthread_mutex_unlock(&oliMutexes[key]);
}

