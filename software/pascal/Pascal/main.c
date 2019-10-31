#include <wiringOli.h>
#include <oliExt.h>
#include <GSM.h>
#include <stdio.h>
#include <i2cLcd.h>
#include <string.h>

void output(int value);
void decodeSMS(char *number, char *text);
void doCommand(char *str);
int numCommand(char *str);
char *GetCommand(char *str, int index);
int GetTokenOpenBraces(char *input, int index);
int GetTokenCloseBraces(char *input, int index);
char *substring(char *string, int position, int length);
char *concat(int count, ...);
void sendSMSAlarm(char *message);

unsigned long prevMDHouse;
int powerFail = 1;
long timeBetweenSMS = 50000;  // In ms

OLI_THREAD(alarmPortA)
{
  char *status;
  oliHiPri(20);
  printf("Alarm thread starts...\n"); fflush(stdout);
  // Loop
  while(1)
  {
    // Camera motion detection A0 Only when B0 = Zone 2 is active
    if (digitalReadPortA(0) == 0)
    {
      printf("PortA0\n");
    }

    delay(100);
  }
}

void sendSMSAlarm(char *message)
{
  int error = 0;
  oliLock(0);
  error = SendSMS("+33626281284", message);
  oliUnlock(0);
  //Check status
  if (error == 0)	printf("SMS ERROR \n");

}

char *concat(int count, ...)
{
    va_list ap;
    int i;

    // Find required length to store merged string
    int len = 1; // room for NULL
    va_start(ap, count);
    for(i=0 ; i<count ; i++)
        len += strlen(va_arg(ap, char*));
    va_end(ap);

    // Allocate memory to concat strings
    char *merged = calloc(sizeof(char),len);
    int null_pos = 0;

    // Actually concatenate strings
    va_start(ap, count);
    for(i=0 ; i<count ; i++)
    {
        char *s = va_arg(ap, char*);
        strcpy(merged+null_pos, s);
        null_pos += strlen(s);
    }
    va_end(ap);

    return merged;
}

void decodeSMS(char *number, char *text)
{
  int i;
  int error;
  char *cmd;
  char *status;
  // Message is (A=x)(B=y)...
  int numCmd = numCommand(text);
  printf("numCmd = %d\n", numCmd);
  if (numCmd > 0)
  {
    // Get all commands
    for (i=1; i<numCmd+1; i++)
    {
      cmd = GetCommand(text, i);
      printf("cmd=%s\n", cmd);
      doCommand(cmd);
    }
  }
  else
  {
    status = concat(0);
    // Get status of camera
    if (digitalReadPortC(0) == 1) status = concat(2, status, "(A=1)");
    else status = concat(2, status, "(A=0)");
    // Get status of lights
    if (digitalReadPortC(1) == 1) status = concat(2, status, "(B=1)");
    else status = concat(2, status, "(B=0)");
    if (digitalReadPortC(2) == 1) status = concat(2, status, "(C=1)");
    else status = concat(2, status, "(C=0)");
    if (digitalReadPortC(3) == 1) status = concat(2, status, "(D=1)");
    else status = concat(2, status, "(D=0)");
    if (digitalReadPortC(4) == 1) status = concat(2, status, "(E=1)");
    else status = concat(2, status, "(E=0)");
    if (digitalReadPortC(5) == 1) status = concat(2, status, "(F=1)");
    else status = concat(2, status, "(F=0)");
    if (digitalReadPortC(6) == 1) status = concat(2, status, "(G=1)");
    else status = concat(2, status, "(G=0)");
    // Get status of motion detector
    if (digitalReadPortC(7) == 1) status = concat(2, status, "(H=1)");
    else status = concat(2, status, "(H=0)");
    if (digitalReadPortB(0) == 1) status = concat(2, status, "(I=1)");
    else status = concat(2, status, "(I=0)");
    if (digitalReadPortB(1) == 1) status = concat(2, status, "(J=1)");
    else status = concat(2, status, "(J=0)");
    if (digitalReadPortB(2) == 1) status = concat(2, status, "(K=1)");
    else status = concat(2, status, "(K=0)");
    if (digitalReadPortB(3) == 1) status = concat(2, status, "(L=1)");
    else status = concat(2, status, "(L=0)");
    // Portails

    // Send back status
    printf("status=%s\n", status);
    oliLock(0);
    error = SendSMS(number, status);
    oliUnlock(0);
    //Check status
    if (error == 0)
  	{
  		printf("SMS ERROR \n");
  	}
  	else
  	{
  		printf("SMS OK \n");
  	}
  	free(status);
  	free(cmd);
  }
}

void doCommand(char *str)
{
  char cmd = '0';
  char val = '\0';
  if (strlen(str) >= 3)
  {
    cmd = str[0];
    val = str[2];
    // Command is A=1 for example
    switch (cmd)
    {
      case 'A' :  // C0
        if (val == '1') digitalWritePortC(0, HIGH);
        else digitalWritePortC(0, LOW);
        break;
      case 'B' :  // C1
        if (val == '1') digitalWritePortC(1, HIGH);
        else digitalWritePortC(1, LOW);
        break;
      case 'C' :  // C2
        if (val == '1') digitalWritePortC(2, HIGH);
        else digitalWritePortC(2, LOW);
        break;
      case 'D' :  // C3
        if (val == '1') digitalWritePortC(3, HIGH);
        else digitalWritePortC(3, LOW);
        break;
      case 'E' :  // C4
        if (val == '1') digitalWritePortC(4, HIGH);
       else digitalWritePortC(4, LOW);
       break;
      case 'F' :  // C5
        if (val == '1') digitalWritePortC(5, HIGH);
        else digitalWritePortC(5, LOW);
        break;
      case 'G' :  // C6
        if (val == '1') digitalWritePortC(6, HIGH);
        else digitalWritePortC(6, LOW);
        break;
      case 'H' :  // C7
        if (val == '1') digitalWritePortC(7, HIGH);
        else digitalWritePortC(7, LOW);
        break;
      case 'I' :  // B0
        if (val == '1') digitalWritePortB(0, HIGH);
        else digitalWritePortB(0, LOW);
        break;
      case 'J' :  // B1
        if (val == '1') digitalWritePortB(1, HIGH);
        else digitalWritePortB(1, LOW);
        break;
      case 'K' :  // B2
        if (val == '1') digitalWritePortB(2, HIGH);
        else digitalWritePortB(2, LOW);
        break;
      case 'L' :  // B3
        if (val == '1') digitalWritePortB(3, HIGH);
        else digitalWritePortB(3, LOW);
        break;
      case 'M' :  // B4
        if (val == '1')
        {
          digitalWritePortB(5, LOW);
          digitalWritePortB(4, HIGH);
        }
        else
        {
          digitalWritePortB(4, LOW);
          digitalWritePortB(5, LOW);
        }
        break;
      case 'N' :  // B5
        if (val == '1')
        {
          digitalWritePortB(4, LOW);
          digitalWritePortB(5, HIGH);
        }
        else
        {
          digitalWritePortB(5, LOW);
          digitalWritePortB(4, LOW);
        }
        break;
      case 'O' :  // B6
        if (val == '1')
        {
          digitalWritePortB(7, LOW);
          digitalWritePortB(6, HIGH);
        }
        else
        {
          digitalWritePortB(6, LOW);
          digitalWritePortB(7, LOW);
        }
        break;
      case 'P' :  // B7
        if (val == '1')
        {
          digitalWritePortB(6, LOW);
          digitalWritePortB(7, HIGH);
        }
        else
        {
          digitalWritePortB(7, LOW);
          digitalWritePortB(6, LOW);
        }
        break;
      case 'Q' :  // B8
        if (val == '1')
        {
          digitalWritePortB(9, LOW);
          digitalWritePortB(8, HIGH);
        }
        else
        {
          digitalWritePortB(8, LOW);
          digitalWritePortB(9, LOW);
        }
        break;
      case 'R' :  // B9
        if (val == '1')
        {
          digitalWritePortB(8, LOW);
          digitalWritePortB(9, HIGH);
        }
        else
        {
          digitalWritePortB(9, LOW);
          digitalWritePortB(8, LOW);
        }
        break;
      case 'S' :  // B10
        if (val == '1')
        {
          digitalWritePortB(11, LOW);
          digitalWritePortB(10, HIGH);
        }
        else
        {
          digitalWritePortB(10, LOW);
          digitalWritePortB(11, LOW);
        }
        break;
    case 'T' :  // B11
        if (val == '1')
        {
          digitalWritePortB(10, LOW);
          digitalWritePortB(11, HIGH);
        }
        else
        {
          digitalWritePortB(11, LOW);
          digitalWritePortB(10, LOW);
        }
        break;
    case 'U' :  // B12
        if (val == '1')
        {
          digitalWritePortB(13, LOW);
          digitalWritePortB(12, HIGH);
        }
        else
        {
          digitalWritePortB(12, LOW);
          digitalWritePortB(13, LOW);
        }
        break;
    case 'V' :  // B13
        if (val == '1')
        {
          digitalWritePortB(12, LOW);
          digitalWritePortB(13, HIGH);
        }
        else
        {
          digitalWritePortB(13, LOW);
          digitalWritePortB(12, LOW);
        }
        break;
    }
  }
}

int numCommand(char *str)
{
  int i;
  int b=0;
  for(i=0; str[i]!='\0'; i++)
  {
    if (str[i] == '(')
    {
      b++;
    }
  }
  return(b);
}

char *GetCommand(char *str, int index)
{
  int start = GetTokenOpenBraces(str, index);
  int end = GetTokenCloseBraces(str, index);
  printf("start=%d end=%d\n", start, end);
  if (start < end)
  {
    // Get what we need that in between braces
    char *res = substring(str, start+1, end-start-1);
    return(res);
  }
  else
  {
    return('\0');
  }
}

char *substring(char *string, int position, int length)
{
   char *pointer;
   int c;
   pointer = malloc(length+1);
   if (pointer == NULL)
   {
      printf("Unable to allocate memory.\n");
      exit(EXIT_FAILURE);
   }
   for (c = 0; c < position; c++)
      string++;
   for (c = 0; c < length; c++)
   {
      *(pointer+c) = *string;
      string++;
   }
   *(pointer+c) = '\0';

   return pointer;
}

int GetTokenOpenBraces(char *input, int index)
{
  int i;
  int b=0;
  for(i=0; input[i]!='\0'; i++)
  {
    if (input[i] == '(')
    {
      b++;
      if (b == index) return(i);
    }
  }
  return(0);
}

int GetTokenCloseBraces(char *input, int index)
{
  int i;
  int b=0;
  for(i=0; input[i]!='\0'; i++)
  {
    if (input[i] == ')')
    {
      b++;
      if (b == index) return(i);
    }
  }
  return(0);
}

void output(int value)
{
  int i;
  if (value ==0)
  {
    for (i=0; i<16; i++)
    {
      digitalWritePortA(i, LOW);
    }
  }
  else if ((value > 0) && (value < 16))
  {
    for (i=0; i<16; i++)
    {
      if (i == value)
        digitalWritePortA(i, HIGH);
      else
        digitalWritePortA(i, LOW);
    }
  }
}

int main(char **args)
{
  int i;
  char ch;
  printf("GSM Testing V0.2.1\n");
  // Setup GPIO
  wiringOliSetup();
  oliExtSetup();
  LCDInit(0x27, 4, 20);
  LCDPrintString(0, 0, "GSM Testing V0.2.1");
  // Port A as input
  for (i=0; i<16; i++)
  {
    pinModePortA(i, INPUT);
    pullUpDnCtrlPortA(i, 2);
    digitalWritePortA(i, LOW);
  }
  // Port B as output
  for (i=0; i<16; i++)
  {
    pinModePortB(i, OUTPUT);
    digitalWritePortB(i, HIGH);
  }
  // Port C as output
  for (i=0; i<8; i++)
  {
    pinModePortC(i, OUTPUT);
    digitalWritePortC(i, HIGH);
  }
  // Port D as output
  for (i=0; i<8; i++)
  {
    pinModePortD(i, OUTPUT);
    digitalWritePortD(i, HIGH);
  }
  // Start alarm thread
  int x2 = oliThreadCreate(alarmPortA);
  if (x2 != 0) printf ("Alarm didn't start\n");

  // Infinite loop
  for(;;)
  {
    ch=getchar();
    if(ch=='0')
    {
      break;
    }
    delay(100);
  }
  printf("Done.\n");
}
