#include <wiringOli.h>
#include <oliExt.h>
#include <GSM.h>

int main()
{
  int error;
  int i;
  char number[] = "+33626281284";  	// Destination number
  char text[] = "Hello world";  	  // SMS to send
  
  // Setup GPIO
  wiringOliSetup();
  oliExtSetup();
  
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
    // Send SMS
    error = SendSMS(number, text);
    printf("%d\n", error);
    // Make a call
    CallS(number);
    
  }
  else
  {
    printf("Module not started...\n");
    printf("End...\n");
  }

  serialEnd();
  return(0);
}
