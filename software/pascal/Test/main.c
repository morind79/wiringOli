#include <wiringOli.h>
#include <oliExt.h>
#include <wiringOliI2C.h>
#include <stdio.h>
#include <string.h>

int main(char **args)
{
  int i;
  char ch;
  int fd;
  printf("GSM Testing V0.2.1\n");
  // Setup GPIO
  wiringOliSetup();
  oliExtSetup();
  // I2C communication
  fd = wiringOliI2CSetup(0, 0x02);
  if (fd < 0)
  {
    printf("Error initializing I2C bus");
    return;
  }

  // Infinite loop
  for(;;)
  {
    ch=getchar();
    if(ch=='0')
    {
      break;
    }
    else if(ch=='A')
    {
      //write(fd, "S", 1);
      wiringOliI2CWriteReg8(fd, 0x00, 0x53);
    }
    else if(ch=='a')
    {
      wiringOliI2CWriteReg8(fd, 0x00, 0x73);

    }
    delay(100);
  }
  printf("Done.\n");
}

