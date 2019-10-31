#include <stdio.h>
#include <wiringOli.h>
#include <oliExt.h>

int main()
{
  // Setup GPIO
  wiringOliSetup();
  oliExtSetup();
  
  digitalWriteSIM900_ON(LOW);
  digitalWriteSIM900_RST(LOW);
  
  // generate turn on pulse
  printf("Power On\n");
	digitalWriteSIM900_ON(HIGH);
	delay(1200);
	//sleep(2);
	digitalWriteSIM900_ON(LOW);
	delay(5000);
  
  return(0);
}
