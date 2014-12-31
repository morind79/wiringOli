#include <wiringOli.h>
#include <oliExt.h>
#include <GSM.h>
#include <stdio.h>

OLI_THREAD(ri)
{
  int pos = 0;
  char number[20];
  char text[180];
  number[0] = '\0';
  text[0] = '\0';
  oliHiPri(10);
  printf("RI thread starts...\n"); fflush(stdout);
  // Loop
  while(1)
  {
    // Check if RI status changed
    if (digitalReadSIM900_RI() == 0)
    {
      // Something happened
      pos = (int)IsSMSPresent(SMS_UNREAD);
      printf("Position = %d\n", pos); 
      if ((pos > 0) && (pos <= 20))
      {
        text[0]='\0';
        GetSMS(pos, number, text, 180);
        printf("SMS : %s, from %s\n", text, number);
        DeleteSMS(pos);      
      }     
    }
    delay(100);
  }
}

int main()
{
  int i;
  char number[] = "+33626281284";  	// Destination number
  char text[] = "Hello world";  	  // SMS to send
  
  // Setup GPIO
  wiringOliSetup();
  oliExtSetup();
  // Start thread for RI pin
  int x1 = oliThreadCreate(ri);
  if (x1 != 0) printf ("RI didn't start\n");
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
    // Infinite loop
    while(1)
    {
      delay(5000);
    }
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