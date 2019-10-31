#include <wiringOli.h>
#include <oliExt.h>
#include <GSM.h>
#include <stdio.h>

int main()
{
  int i;
  char number[] = "+33626281284";  	// Destination number
  
  // Setup GPIO
  wiringOliSetup();
  oliExtSetup();
  // Open serial port
  printf("Start...\n");
  serialBegin(9600);
  // Test if module is on
  if (!isOn())
  {
    for (i = 0; i < 4; i++)
    {
      powerOn();
      if (isOn()) break;  
    }
  }
  if (isOn())
  {
    printf("Module is on...\n");
    init();
    // Make a call
    CallS(number);
    delay(10000);
    HangUp();
  }
  else
  {
    printf("Module not started...\n");
    printf("End...\n");
  }
  // Close serial port
  serialEnd();
  return(0);
}