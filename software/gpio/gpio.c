/*
 * gpio.c:
 *	Set command line interface to the OliExt board
 *	Copyright (c) 2015 Denis Morin
 ***********************************************************************
 * This file is part of wiringOli:
 *	https://
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
#include <stdlib.h>
#include <stdint.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <wiringOli.h>
#include <oliExt.h>

#ifndef TRUE
#  define	TRUE	(1==1)
#  define	FALSE	(1==2)
#endif

#define	VERSION		"1.00"
#define	I2CDETECT	"/usr/sbin/i2cdetect"

int wpMode ;

char *usage = "Usage: gpio -v\n"
              "       gpio -h\n"
              "       gpio -warranty\n"
              "       gpio mode <a, b, c, d> <pin> <input/output>\n"
              "       gpio read <a, b, c, d> <pin>\n"
              "       gpio write <a, b, c, d> <pin> <0/1>\n";

/*
 * changeOwner:
 *	Change the ownership of the file to the real userId of the calling
 *	program so we can access it.
 *********************************************************************************
 */

static void changeOwner (char *cmd, char *file)
{
  uid_t uid = getuid () ;
  uid_t gid = getgid () ;

  if (chown (file, uid, gid) != 0)
  {
    if (errno == ENOENT)	// Warn that it's not there
      fprintf (stderr, "%s: Warning: File not present: %s\n", cmd, file) ;
    else
    {
      fprintf (stderr, "%s: Unable to change ownership of %s: %s\n", cmd, file, strerror (errno)) ;
      exit (1) ;
    }
  }
}

/*
 * doExports:
 *	List all GPIO exports
 *********************************************************************************
 */

static void doExports (int argc, char *argv [])
{
  int fd ;
  int i, l, first ;
  char fName [128] ;
  char buf [16] ;

  for (first = 0, i = 0 ; i < 64 ; ++i)	// Crude, but effective
  {

// Try to read the direction

    sprintf (fName, "/sys/class/gpio/gpio%d/direction", i) ;
    if ((fd = open (fName, O_RDONLY)) == -1)
      continue ;

    if (first == 0)
    {
      ++first ;
      printf ("GPIO Pins exported:\n") ;
    }

    printf ("%4d: ", i) ;

    if ((l = read (fd, buf, 16)) == 0)
      sprintf (buf, "%s", "?") ;
 
    buf [l] = 0 ;
    if ((buf [strlen (buf) - 1]) == '\n')
      buf [strlen (buf) - 1] = 0 ;

    printf ("%-3s", buf) ;

    close (fd) ;

// Try to Read the value

    sprintf (fName, "/sys/class/gpio/gpio%d/value", i) ;
    if ((fd = open (fName, O_RDONLY)) == -1)
    {
      printf ("No Value file (huh?)\n") ;
      continue ;
    }

    if ((l = read (fd, buf, 16)) == 0)
      sprintf (buf, "%s", "?") ;

    buf [l] = 0 ;
    if ((buf [strlen (buf) - 1]) == '\n')
      buf [strlen (buf) - 1] = 0 ;

    printf ("  %s", buf) ;

// Read any edge trigger file

    sprintf (fName, "/sys/class/gpio/gpio%d/edge", i) ;
    if ((fd = open (fName, O_RDONLY)) == -1)
    {
      printf ("\n") ;
      continue ;
    }

    if ((l = read (fd, buf, 16)) == 0)
      sprintf (buf, "%s", "?") ;

    buf [l] = 0 ;
    if ((buf [strlen (buf) - 1]) == '\n')
      buf [strlen (buf) - 1] = 0 ;

    printf ("  %-8s\n", buf) ;

    close (fd) ;
  }
}


/*
 * doExport:
 *	gpio export pin mode
 *	This uses the /sys/class/gpio device interface.
 *********************************************************************************
 */

void doExport (int argc, char *argv [])
{
  FILE *fd ;
  int pin ;
  char *mode ;
  char fName [128] ;

  if (argc != 4)
  {
    fprintf (stderr, "Usage: %s export pin mode\n", argv [0]) ;
    exit (1) ;
  }

  pin = atoi (argv [2]) ;

  mode = argv [3] ;

  if ((fd = fopen ("/sys/class/gpio/export", "w")) == NULL)
  {
    fprintf (stderr, "%s: Unable to open GPIO export interface: %s\n", argv [0], strerror (errno)) ;
    exit (1) ;
  }

  fprintf (fd, "%d\n", pin) ;
  fclose (fd) ;

  sprintf (fName, "/sys/class/gpio/gpio%d/direction", pin) ;
  if ((fd = fopen (fName, "w")) == NULL)
  {
    fprintf (stderr, "%s: Unable to open GPIO direction interface for pin %d: %s\n", argv [0], pin, strerror (errno)) ;
    exit (1) ;
  }

  /**/ if ((strcasecmp (mode, "in")  == 0) || (strcasecmp (mode, "input")  == 0))
    fprintf (fd, "in\n") ;
  else if ((strcasecmp (mode, "out") == 0) || (strcasecmp (mode, "output") == 0))
    fprintf (fd, "out\n") ;
  else
  {
    fprintf (stderr, "%s: Invalid mode: %s. Should be in or out\n", argv [1], mode) ;
    exit (1) ;
  }

  fclose (fd) ;

// Change ownership so the current user can actually use it!

  sprintf (fName, "/sys/class/gpio/gpio%d/value", pin) ;
  changeOwner (argv [0], fName) ;

  sprintf (fName, "/sys/class/gpio/gpio%d/edge", pin) ;
  changeOwner (argv [0], fName) ;

}

/*
 * doUnexport:
 *	gpio unexport pin
 *	This uses the /sys/class/gpio device interface.
 *********************************************************************************
 */

void doUnexport (int argc, char *argv [])
{
  FILE *fd ;
  int pin ;

  if (argc != 3)
  {
    fprintf (stderr, "Usage: %s unexport pin\n", argv [0]) ;
    exit (1) ;
  }

  pin = atoi (argv [2]) ;

  if ((fd = fopen ("/sys/class/gpio/unexport", "w")) == NULL)
  {
    fprintf (stderr, "%s: Unable to open GPIO export interface\n", argv [0]) ;
    exit (1) ;
  }

  fprintf (fd, "%d\n", pin) ;
  fclose (fd) ;
}


/*
 * doUnexportAll:
 *	gpio unexportall
 *	Un-Export all the GPIO pins.
 *	This uses the /sys/class/gpio device interface.
 *********************************************************************************
 */

void doUnexportall(char *progName)
{
  FILE *fd ;
  int pin ;

  for (pin = 0 ; pin < 63 ; ++pin)
  {
    if ((fd = fopen ("/sys/class/gpio/unexport", "w")) == NULL)
    {
      fprintf (stderr, "%s: Unable to open GPIO export interface\n", progName) ;
      exit (1) ;
    }
    fprintf (fd, "%d\n", pin) ;
    fclose (fd) ;
  }
}

/*
 * doMode:
 *	gpio mode pin mode ...
 *	gpio mode <port> <pin> <input/output> 
 *********************************************************************************
 */

void doMode(int argc, char *argv [])
{
  int pin;
  char *mode;
  char *port;

  if (argc != 5)
  {
    fprintf(stderr, "Usage: gpio mode <port> <pin> <input/output>\n");
    exit(1);
  }

  port = argv[2];
  pin = atoi (argv[3]);
  mode = argv[4];
  
  // Port A
  if (strcasecmp(port, "a") == 0)
  {
    if ((pin < 0) || (pin > 15))
    {
      fprintf(stderr, "Pin range for port : %s is 0 to 15\n", port);
      exit(1);    
    }
    if (strcasecmp(mode, "input") == 0) pinModePortA(pin, INPUT);
    else if (strcasecmp(mode, "output") == 0) pinModePortA(pin, OUTPUT);
    else
    {
      fprintf(stderr, "%s: Invalid mode: %s. Should be input/output\n", argv[1], mode);
      exit(1);
    }
  }
  // Port B
  else if (strcasecmp(port, "b") == 0)
  {
    if ((pin < 0) || (pin > 15))
    {
      fprintf(stderr, "Pin range for port : %s is 0 to 15\n", port);
      exit(1);    
    }
    if (strcasecmp(mode, "input") == 0) pinModePortB(pin, INPUT);
    else if (strcasecmp(mode, "output") == 0) pinModePortB(pin, OUTPUT);
    else
    {
      fprintf (stderr, "%s: Invalid mode: %s. Should be input/output\n", argv [1], mode);
      exit(1);
    }
  }
  // Port C
  else if (strcasecmp(port, "c") == 0)
  {
    if ((pin < 0) || (pin > 15))
    {
      fprintf(stderr, "Pin range for port : %s is 0 to 7\n", port);
      exit(1);    
    }
    if (strcasecmp(mode, "input") == 0) pinModePortC(pin, INPUT);
    else if (strcasecmp(mode, "output") == 0) pinModePortC(pin, OUTPUT);
    else
    {
      fprintf (stderr, "%s: Invalid mode: %s. Should be input/output\n", argv [1], mode);
      exit(1);
    }
  }
  // Port D
  else if (strcasecmp(port, "d") == 0)
  {
    if ((pin < 0) || (pin > 15))
    {
      fprintf(stderr, "Pin range for port : %s is 0 to 7\n", port);
      exit(1);    
    }
    if (strcasecmp(mode, "input") == 0) pinModePortD(pin, INPUT);
    else if (strcasecmp(mode, "output") == 0) pinModePortD(pin, OUTPUT);
    else
    {
      fprintf (stderr, "%s: Invalid mode: %s. Should be input/output\n", argv [1], mode);
      exit(1);
    }
  }
  else
  {
    fprintf(stderr, "Port : %s does not exist\nUsage: gpio mode <port> <pin> <input/output>\n", port);
    exit(1);
  }
}

/*
 * doWrite:
 *	gpio write pin value
 *	gpio write <port> <pin> <0/1>  
 *********************************************************************************
 */

static void doWrite(int argc, char *argv[])
{
  int pin;
  char *port;

  if (argc != 5)
  {
    fprintf (stderr, "Usage: gpio write <port> <pin> <0/1> \n");
    exit(1);
  }
  
  port = argv[2];
  pin = atoi(argv[3]);
  
  // Port A
  if (strcasecmp(port, "a") == 0)
  {
    if ((pin < 0) || (pin > 15))
    {
      fprintf(stderr, "Pin range for port %s is 0 to 15\n", port);
      exit(1);    
    }
    if (strcasecmp(argv[4], "1") == 0) digitalWritePortA(pin, HIGH);
    else if (strcasecmp(argv[4], "0") == 0) digitalWritePortA(pin, LOW);
    else
    {
      fprintf(stderr, "%s: Invalid value: %s. Should be 0/1\n", argv[1], argv[4]);
      exit(1);
    }
  }
  // Port B
  else if (strcasecmp(port, "b") == 0)
  {
    if ((pin < 0) || (pin > 15))
    {
      fprintf(stderr, "Pin range for port %s is 0 to 15\n", port);
      exit(1);    
    }
    if (strcasecmp(argv[4], "1") == 0) digitalWritePortB(pin, HIGH);
    else if (strcasecmp(argv[4], "0") == 0) digitalWritePortB(pin, LOW);
    else
    {
      fprintf (stderr, "%s: Invalid mode: %s. Should be 0/1\n", argv [1], argv[4]);
      exit(1);
    }
  }
  // Port C
  else if (strcasecmp(port, "c") == 0)
  {
    if ((pin < 0) || (pin > 15))
    {
      fprintf(stderr, "Pin range for port %s is 0 to 7\n", port);
      exit(1);    
    }
    if (strcasecmp(argv[4], "1") == 0) digitalWritePortC(pin, HIGH);
    else if (strcasecmp(argv[4], "0") == 0) digitalWritePortC(pin, LOW);
    else
    {
      fprintf (stderr, "%s: Invalid mode: %s. Should be 0/1\n", argv [1], argv[4]);
      exit(1);
    }
  }
  // Port D
  else if (strcasecmp(port, "d") == 0)
  {
    if ((pin < 0) || (pin > 15))
    {
      fprintf(stderr, "Pin range for port %s is 0 to 7\n", port);
      exit(1);    
    }
    if (strcasecmp(argv[4], "1") == 0) digitalWritePortD(pin, HIGH);
    else if (strcasecmp(argv[4], "0") == 0) digitalWritePortD(pin, LOW);
    else
    {
      fprintf (stderr, "%s: Invalid mode: %s. Should be 0/1\n", argv [1], argv[4]);
      exit(1);
    }
  }
  else
  {
    fprintf(stderr, "Port : %s does not exist\nUsage: gpio write <port> <pin> <0/1>\n", port);
    exit(1);
  }
}

/*
 * doRead:
 *	Read a pin and return the value
 *	gpio read <port> <pin>   
 *********************************************************************************
 */

void doRead(int argc, char *argv[]) 
{
  int pin, val;
  char *port;

  if (argc != 4)
  {
    fprintf(stderr, "Usage: %s read pin\n", argv[0]);
    exit(1);
  }

  port = argv[2];
  pin = atoi(argv[3]);
  val = -1;

  // Port A
  if (strcasecmp(port, "a") == 0)
  {
    if ((pin < 0) || (pin > 15))
    {
      fprintf(stderr, "Pin range for port : %s is 0 to 15\n", port);
      exit(1);    
    }
    val = digitalReadPortA(pin);
  }
  // Port B
  else if (strcasecmp(port, "b") == 0)
  {
    if ((pin < 0) || (pin > 15))
    {
      fprintf(stderr, "Pin range for port : %s is 0 to 15\n", port);
      exit(1);    
    }
    val = digitalReadPortB(pin);
  }
  // Port C
  else if (strcasecmp(port, "c") == 0)
  {
    if ((pin < 0) || (pin > 15))
    {
      fprintf(stderr, "Pin range for port : %s is 0 to 7\n", port);
      exit(1);    
    }
    val = digitalReadPortC(pin);
  }
  // Port D
  else if (strcasecmp(port, "d") == 0)
  {
    if ((pin < 0) || (pin > 15))
    {
      fprintf(stderr, "Pin range for port : %s is 0 to 7\n", port);
      exit(1);    
    }
    val = digitalReadPortD(pin);
  }
  else
  {
    fprintf(stderr, "Port : %s does not exist\nUsage: gpio read <port> <pin>\n", port);
    exit(1);
  }  

  printf ("%s\n", val == 0 ? "0" : "1") ;
}

/*
 * doToggle:
 *	Toggle an IO pin
 *	gpio toggle <port> <pin>  
 *********************************************************************************
 */

void doToggle(int argc, char *argv[])
{
  int pin;
  char *port;

  if (argc != 4)
  {
    fprintf(stderr, "Usage: %s toggle pin\n", argv [0]);
    exit(1);
  }

  port = argv[2];
  pin = atoi (argv[3]);
  
  // Port A
  if (strcasecmp(port, "a") == 0)
  {
    if ((pin < 0) || (pin > 15))
    {
      fprintf(stderr, "Pin range for port : %s is 0 to 15\n", port);
      exit(1);    
    }
    digitalWritePortA(pin, !digitalReadPortA(pin));
  }
  // Port B
  else if (strcasecmp(port, "b") == 0)
  {
    if ((pin < 0) || (pin > 15))
    {
      fprintf(stderr, "Pin range for port : %s is 0 to 15\n", port);
      exit(1);    
    }
    digitalWritePortB(pin, !digitalReadPortB(pin));
  }
  // Port C
  else if (strcasecmp(port, "c") == 0)
  {
    if ((pin < 0) || (pin > 15))
    {
      fprintf(stderr, "Pin range for port : %s is 0 to 7\n", port);
      exit(1);    
    }
    digitalWritePortC(pin, !digitalReadPortC(pin));
  }
  // Port D
  else if (strcasecmp(port, "d") == 0)
  {
    if ((pin < 0) || (pin > 15))
    {
      fprintf(stderr, "Pin range for port : %s is 0 to 7\n", port);
      exit(1);    
    }
    digitalWritePortD(pin, !digitalReadPortD(pin));
  }
  else
  {
    fprintf(stderr, "Port : %s does not exist\nUsage: gpio toggle <port> <pin>\n", port);
    exit(1);
  }  
}

/*
 * main:
 *	Start here
 *********************************************************************************
 */

int main (int argc, char *argv[])
{

  if (argc == 1)
  {
    fprintf(stderr, "%s\n", usage);
    return 1;
  }

  // Help
  if (strcasecmp(argv [1], "-h") == 0)
  {
    printf("%s: %s\n", argv [0], usage);
    return 0;
  }

  // Version
  if (strcmp (argv [1], "-v") == 0)
  {
    printf("gpio version: %s\n", VERSION);
    printf("Copyright (c) 2015 Denis Morin\n");
    printf("This is free software with ABSOLUTELY NO WARRANTY.\n");
    printf("For details type: %s -warranty\n", argv [0]);
    printf("\n");
    return 0;
  }
  
  // Warranty
  if (strcasecmp (argv [1], "-warranty") == 0)
  {
    printf ("gpio version: %s\n", VERSION) ;
    printf ("Copyright (c) 2015 Denis Morin\n") ;
    printf ("\n") ;
    printf ("    This program is free software; you can redistribute it and/or modify\n") ;
    printf ("    it under the terms of the GNU Leser General Public License as published\n") ;
    printf ("    by the Free Software Foundation, either version 3 of the License, or\n") ;
    printf ("    (at your option) any later version.\n") ;
    printf ("\n") ;
    printf ("    This program is distributed in the hope that it will be useful,\n") ;
    printf ("    but WITHOUT ANY WARRANTY; without even the implied warranty of\n") ;
    printf ("    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n") ;
    printf ("    GNU Lesser General Public License for more details.\n") ;
    printf ("\n") ;
    printf ("    You should have received a copy of the GNU Lesser General Public License\n") ;
    printf ("    along with this program. If not, see <http://www.gnu.org/licenses/>.\n") ;
    printf ("\n") ;
    return 0 ;
  }

  if (geteuid () != 0)
  {
    fprintf (stderr, "%s: Must be root to run. Program should be suid root. This is an error.\n", argv [0]) ;
    return 1 ;
  }

  // Initial test for /sys/class/gpio operations:
  /**/ if (strcasecmp (argv [1], "exports"    ) == 0)	{ doExports     (argc, argv) ;	return 0 ; }
  else if (strcasecmp (argv [1], "export"     ) == 0)	{ doExport      (argc, argv) ;	return 0 ; }
  else if (strcasecmp (argv [1], "unexport"   ) == 0)	{ doUnexport    (argc, argv) ;	return 0 ; }
  else if (strcasecmp (argv [1], "unexportall") == 0)	{ doUnexportall (argv [0]) ;	return 0 ; }

  // Init board
  wiringOliSetup();
  //oliExtSetup();

  if (argc <= 1)
  {
    fprintf(stderr, "%s: no command given\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  // Core wiringPi functions
  if (strcasecmp(argv[1], "mode") == 0) doMode(argc, argv);
  else if (strcasecmp(argv[1], "read") == 0) doRead(argc, argv);
  else if (strcasecmp(argv[1], "write") == 0) doWrite(argc, argv);

  // GPIO toggle
  else if (strcasecmp(argv[1], "toggle") == 0) doToggle(argc, argv);
  else
  {
    fprintf(stderr, "%s: Unknown command: %s.\n", argv[0], argv[1]);
    exit(EXIT_FAILURE);
  }
  return 0;
}
