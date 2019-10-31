#include <wiringOli.h>
#include <wiringOliI2C.h>
#include <oliExt.h>
#include <i2cLcd.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <mysql.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>

MYSQL *connection;
time_t t = time(NULL);
struct tm tm;

int readDB();
void clearDB();
void smsProcessedDB(int index);
void sendSMSDB(const char *number, char *message);
void doCommand(char *str);
char *decodeSMS(const char *number, const char *text);
int numCommand(const char *str);
char *GetCommand(const char *str, int index);
int GetTokenOpenBraces(char *input, int index);
int GetTokenCloseBraces(char *input, int index);
char *concat(int count, ...);
char *substring(char *string, int position, int length);
void sendSMSAlarm(char *message);
void roundDisplay(int value);

unsigned long prevMDHouse;    // Variable used for knowing time since last sent SMS
int powerFail = 0;            // 0 -> No power, 1 -> Power ok
long timeBetweenSMS = 50000;  // Waiting time before sending another SMS in ms
const char *PASCAL = "+33624323080";
const char *SEBASTIEN = "+33635526656";
const char *DENIS = "+33626281284";

// 40s to open or close a portail
const int PORTAIL_TIMER = 40000;

// Portail motors are on port B
int portailMotorUp[] = {4, 6, 8, 10, 12};
int portailMotorDown[] = {5, 7, 9, 11, 13};

// Global variable for status
int portailMaisonUpChanged = 0;
int portailMaisonDownChanged = 0;

int portail1UpChanged = 0;
int portail1DownChanged = 0;

int portail2UpChanged = 0;
int portail2DownChanged = 0;

int portail3UpChanged = 0;
int portail3DownChanged = 0;

int portailSARLUpChanged = 0;
int portailSARLDownChanged = 0;

OLI_THREAD(ri)
{
  int i;
  int cnt = 0;
  int dsp = 0;
  int clearForToday = 0;
  MYSQL mysql;
  oliHiPri(10);
  printf("RI thread starts...\n"); fflush(stdout);
  // Loop
  while(1)
  {
    // Check if RI status changed
    if (digitalReadSIM900_RI() == 0)
    {
      oliLock(0);
      printf("RI detected...\n");
      // connect to the MySQL database at localhost
      mysql_init(&mysql);
      connection = mysql_real_connect(&mysql,"localhost", "morind79", "DMO12lip6", "sms", 0, 0, 0);
      // check for a connection error
      if (connection == NULL)
      {
        printf(mysql_error(&mysql));
      }
      // Wait time Gammu store SMS in database
      delay(10000);
      // Something happened
      for (i=0; i<20; i++)
      {
        if (readDB() > 0) break;
        delay(2000);
      }
      // close the connection
      mysql_close(connection);
      oliUnlock(0);
    }
    // Get current time
    t = time(NULL);
    tm = *localtime(&t);
    // If time is 1:00 then clean DB
    if ((tm.tm_hour == 1) && (tm.tm_min == 00) && (clearForToday == 0))
    {
      oliLock(0);
      // connect to the MySQL database at localhost
      mysql_init(&mysql);
      connection = mysql_real_connect(&mysql,"localhost", "morind79", "DMO12lip6", "sms", 0, 0, 0);
      // check for a connection error
      if (connection == NULL)
      {
        printf(mysql_error(&mysql));
      }
      clearDB();
      clearForToday = 1;
      printf("Clean DB at : %d:%d\n", tm.tm_hour, tm.tm_min);
      // close the connection
      mysql_close(connection);
      oliUnlock(0);
    }
    if ((tm.tm_hour == 1) && (tm.tm_min == 05) && (clearForToday == 1))
    {
      clearForToday = 0;
    }
    // Display 7 Seg
    if (cnt > 10)
    {
      if (dsp > 5) dsp = 0;
      roundDisplay(dsp);
      dsp++;
      cnt = 0;
    }
    cnt++;
    delay(100);
  }
}

OLI_THREAD(alarmPortA)
{
  char *status;
  oliHiPri(20);
  printf("Alarm thread starts...\n"); fflush(stdout);
  // Loop
  while(1)
  {
    // Zone 1 motion detection PortA0 - Only when C7 = Zone 1 is active
    if ((digitalReadPortA(0) == 0) && (digitalReadPortC(7) == 1))
    {
      status = concat(0);
      status = concat(2, status, "Detection mouvement dans bureau SARL.");
      // Wait 10s before sending a new SMS
      if (millis() - prevMDHouse >= timeBetweenSMS)
      {
        prevMDHouse = millis();
        sendSMSAlarm(status);
      }
  	  free(status);
    }
    // Zone 2 motion detection PortA1 - Only when B0 = Zone 2 is active
    if ((digitalReadPortA(1) == 0) && (digitalReadPortB(0) == 1))
    {
      status = concat(0);
      status = concat(2, status, "Detection mouvement dans la cuisine.");
      // Wait 10s before sending a new SMS
      if (millis() - prevMDHouse >= timeBetweenSMS)
      {
        prevMDHouse = millis();
        sendSMSAlarm(status);
      }
  	  free(status);
    }
/*
    // Zone 3 motion detection PortA2 - Only when C0 = Zone 3 is active
    if ((digitalReadPortA(2) == 0) && (digitalReadPortC(0) == 1))
    {
      status = concat(0);
      status = concat(2, status, "Detection mouvement dans la maison.");
      // Wait 10s before sending a new SMS
      if (millis() - prevMDHouse >= timeBetweenSMS)
      {
        prevMDHouse = millis();
        sendSMSAlarm(status);
      }
  	  free(status);
    }
    // House IR barrier detection PortA3 - Only when B0 = Zone 2 is active
    if ((digitalReadPortA(3) == 0) && (digitalReadPortB(1) == 1))
    {
      status = concat(0);
      status = concat(2, status, "Detection barriere IR coté maison.");
      // Wait 10s before sending a new SMS
      if (millis() - prevMDHouse >= timeBetweenSMS)
      {
        prevMDHouse = millis();
        sendSMSAlarm(status);
      }
  	  free(status);
    }
    // SARL IR barrier detection PortA4 - Only when B0 = Zone 2 is active
    if ((digitalReadPortA(4) == 0) && (digitalReadPortB(2) == 1))
    {
      status = concat(0);
      status = concat(2, status, "Detection barriere IR coté SARL.");
      // Wait 10s before sending a new SMS
      if (millis() - prevMDHouse >= timeBetweenSMS)
      {
        prevMDHouse = millis();
        sendSMSAlarm(status);
      }
  	  free(status);
    }
    // Fire PortA4 - Always
    if (digitalReadPortA(5) == 0)
    {
      status = concat(0);
      status = concat(2, status, "Detection barriere IR coté SARL.");
      // Wait 10s before sending a new SMS
      if (millis() - prevMDHouse >= timeBetweenSMS)
      {
        prevMDHouse = millis();
        sendSMSAlarm(status);
      }
  	  free(status);
    }
    // Power failure PortA8
    if ((digitalReadPortA(8) == 0) && (powerFail == 0))
    {
      status = concat(0);
      status = concat(2, status, "Coupure de courant.");
      // Wait 10s before sending a new SMS
      if (millis() - prevMDHouse >= timeBetweenSMS)
      {
        prevMDHouse = millis();
        sendSMSAlarm(status);
      }
  	  free(status);
    }
    if (digitalReadPortA(10) == 1)
    {
      powerFail = 1;
    }
*/
    delay(100);
  }
}

OLI_THREAD(portailMaison)
{
  oliHiPri(10);
  printf("Portail maison thread starts...\n"); fflush(stdout);
  // Loop
  while(1)
  {
    // Check if button status changed
    if (portailMaisonUpChanged == 1)
    {
      portailMaisonUpChanged = 0;
      digitalWritePortB(portailMotorDown[0], LOW);
      digitalWritePortB(portailMotorUp[0], HIGH);
      printf("Portail maison up\n"); fflush(stdout);
      int timout = 0;
      while(timout < PORTAIL_TIMER)
      {
        delay(1);
        timout++;
      }
      // Motor off
      digitalWritePortB(portailMotorUp[0], LOW);
    }
    // Check if button status changed
    if (portailMaisonDownChanged == 1)
    {
      portailMaisonDownChanged = 0;
      digitalWritePortB(portailMotorUp[0], LOW);
      digitalWritePortB(portailMotorDown[0], HIGH);
      printf("Portail maison down\n"); fflush(stdout);
      int timout = 0;
      while(timout < PORTAIL_TIMER)
      {
        delay(1);
        timout++;
      }
      // Motor off
      digitalWritePortB(portailMotorDown[0], LOW);
    }
    delay(100);
  }
}

OLI_THREAD(portail1)
{
  oliHiPri(10);
  printf("Portail1 thread starts...\n"); fflush(stdout);
  // Loop
  while(1)
  {
    // Check if button status changed
    if (portail1UpChanged == 1)
    {
      portail1UpChanged = 0;
      digitalWritePortB(portailMotorDown[1], LOW);
      digitalWritePortB(portailMotorUp[1], HIGH);
      printf("Portail1 up\n"); fflush(stdout);
      int timout = 0;
      while(timout < PORTAIL_TIMER)
      {
        delay(1);
        timout++;
      }
      // Motor off
      digitalWritePortB(portailMotorUp[1], LOW);
    }
    // Check if button status changed
    if (portail1DownChanged == 1)
    {
      portail1DownChanged = 0;
      digitalWritePortB(portailMotorUp[1], LOW);
      digitalWritePortB(portailMotorDown[1], HIGH);
      printf("Portail1 down\n"); fflush(stdout);
      int timout = 0;
      while(timout < PORTAIL_TIMER)
      {
        delay(1);
        timout++;
      }
      // Motor off
      digitalWritePortB(portailMotorDown[1], LOW);
    }
    delay(100);
  }
}

OLI_THREAD(portail2)
{
  oliHiPri(10);
  printf("Portail2 thread starts...\n"); fflush(stdout);
  // Loop
  while(1)
  {
    // Check if button status changed
    if (portail2UpChanged == 1)
    {
      portail2UpChanged = 0;
      digitalWritePortB(portailMotorDown[2], LOW);
      digitalWritePortB(portailMotorUp[2], HIGH);
      printf("Portail2 up\n"); fflush(stdout);
      int timout = 0;
      while(timout < PORTAIL_TIMER)
      {
        delay(1);
        timout++;
      }
      // Motor off
      digitalWritePortB(portailMotorUp[2], LOW);
    }
    // Check if button status changed
    if (portail2DownChanged == 1)
    {
      portail2DownChanged = 0;
      digitalWritePortB(portailMotorUp[2], LOW);
      digitalWritePortB(portailMotorDown[2], HIGH);
      printf("Portail2 down\n"); fflush(stdout);
      int timout = 0;
      while(timout < PORTAIL_TIMER)
      {
        delay(1);
        timout++;
      }
      // Motor off
      digitalWritePortB(portailMotorDown[2], LOW);
    }
    delay(100);
  }
}

OLI_THREAD(portail3)
{
  oliHiPri(10);
  printf("Portail3 thread starts...\n"); fflush(stdout);
  // Loop
  while(1)
  {
    // Check if button status changed
    if (portail3UpChanged == 1)
    {
      portail3UpChanged = 0;
      digitalWritePortB(portailMotorDown[3], LOW);
      digitalWritePortB(portailMotorUp[3], HIGH);
      printf("Portail3 up\n"); fflush(stdout);
      int timout = 0;
      while(timout < PORTAIL_TIMER)
      {
        delay(1);
        timout++;
      }
      // Motor off
      digitalWritePortB(portailMotorUp[3], LOW);
    }
    // Check if button status changed
    if (portail3DownChanged == 1)
    {
      portail3DownChanged = 0;
      digitalWritePortB(portailMotorUp[3], LOW);
      digitalWritePortB(portailMotorDown[3], HIGH);
      printf("Portail3 down\n"); fflush(stdout);
      int timout = 0;
      while(timout < PORTAIL_TIMER)
      {
        delay(1);
        timout++;
      }
      // Motor off
      digitalWritePortB(portailMotorDown[3], LOW);
    }
    delay(100);
  }
}

OLI_THREAD(portailSARL)
{
  oliHiPri(10);
  printf("Portail SARL thread starts...\n"); fflush(stdout);
  // Loop
  while(1)
  {
    // Check if button status changed
    if (portailSARLUpChanged == 1)
    {
      portailSARLUpChanged = 0;
      digitalWritePortB(portailMotorDown[4], LOW);
      digitalWritePortB(portailMotorUp[4], HIGH);
      printf("Portail SARL up\n"); fflush(stdout);
      int timout = 0;
      while(timout < PORTAIL_TIMER)
      {
        delay(1);
        timout++;
      }
      // Motor off
      digitalWritePortB(portailMotorUp[4], LOW);
    }
    // Check if button status changed
    if (portailSARLDownChanged == 1)
    {
      portailSARLDownChanged = 0;
      digitalWritePortB(portailMotorUp[4], LOW);
      digitalWritePortB(portailMotorDown[4], HIGH);
      printf("Portail SARL down\n"); fflush(stdout);
      int timout = 0;
      while(timout < PORTAIL_TIMER)
      {
        delay(1);
        timout++;
      }
      // Motor off
      digitalWritePortB(portailMotorDown[4], LOW);
    }
    delay(100);
  }
}

void clearDB()
{
  int state;
  state = mysql_query(connection, "DELETE FROM inbox");
  if (state != 0)
  {
    printf(mysql_error(connection));
    return;
  }
}

void smsProcessedDB(int index)
{
  int state;
  char qry[400] = "";
  sprintf(qry, "UPDATE inbox SET Processed='true' WHERE ID='%d'", index);
  state = mysql_query(connection, qry);
  if (state != 0)
  {
    printf(mysql_error(connection));
    return;
  }
}

int readDB()
{
  MYSQL_RES *result;
  MYSQL_ROW row;
  int state;
  int num;
  const char *smsText;
  const char *smsNumber;
  int index;
  char *status;
  state = mysql_query(connection, "SELECT TextDecoded, SenderNumber, ID FROM inbox WHERE Processed='false'");
  if (state != 0)
  {
    printf(mysql_error(connection));
    return(0);
  }
  // must call mysql_store_result( ) before you can issue any other query calls
  result = mysql_store_result(connection);
  num = mysql_num_rows(result);
  printf("Rows: %d\n", num);
  // process each row in the result set
  while ((row = mysql_fetch_row(result)) != NULL)
  {
    // Get SMS Text
    smsText = row[0] ? row[0] : "NULL";
    smsNumber = row[1] ? row[1] : "NULL";
    index = row[2] ? atoi(row[2]) : 0;
    status = decodeSMS(smsNumber, smsText);
    smsProcessedDB(index);
    if (strlen(status) > 0)
    {
      sendSMSDB(smsNumber, status);
    }
    printf("Text: %s, Number: %s, index: %d\n", smsText, smsNumber, index);
  }
  // free the result set
  mysql_free_result(result);
  return(num);
}

void sendSMSAlarm(char *message)
{
  MYSQL mysql;
  oliLock(0);
  // connect to the MySQL database at localhost
  mysql_init(&mysql);
  connection = mysql_real_connect(&mysql,"localhost", "morind79", "DMO12lip6", "sms", 0, 0, 0);
  // check for a connection error
  if (connection == NULL)
  {
    printf(mysql_error(&mysql));
  }
  sendSMSDB(PASCAL, message);
  sendSMSDB(SEBASTIEN, message);
  // close the connection
  mysql_close(connection);
  oliUnlock(0);
}

void sendSMSDB(const char *number, char *message)
{
  int state;
  char qry[400] = "";
  sprintf(qry, "INSERT INTO outbox (DestinationNumber, TextDecoded, SenderID, CreatorID, Coding) VALUES ('%s', '%s', 'SIM900', 'SIM900', 'Default_No_Compression')", number, message);
  state = mysql_query(connection, qry);
  if (state != 0)
  {
    printf(mysql_error(connection));
    return;
  }
}

int numCommand(const char *str)
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

char *GetCommand(const char *str, int index)
{
  int start = GetTokenOpenBraces(strdup(str), index);
  int end = GetTokenCloseBraces(strdup(str), index);
  printf("start=%d end=%d\n", start, end);
  if (start < end)
  {
    // Get what we need that in between braces
    char *res = substring(strdup(str), start+1, end-start-1);
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
   pointer = (char *)malloc(length+1);
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
    char *merged = (char *)calloc(sizeof(char),len);
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

char *decodeSMS(const char *number, const char *text)
{
  int i;
  int error;
  char *cmd;
  char *status;
  LCDPrintString(1, 0, "");
  LCDPrintString(1, 0, strdup(number));
  // Compare number to be sure this is an allowed message
  if ((strcmp(number, DENIS) == 0) || (strcmp(number, PASCAL) == 0) || (strcmp(number, SEBASTIEN) == 0))
  {
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
      return(status);

  	  free(status);
    }
  }
  return("");
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
          portailMaisonUpChanged = 1;
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
          portailMaisonDownChanged = 1;
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
          portail1UpChanged = 1;
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
          portail1DownChanged = 1;
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
          portail2UpChanged = 1;
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
          portail2DownChanged = 1;
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
          portail3UpChanged = 1;
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
          portail3DownChanged = 1;
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
          portailSARLUpChanged = 1;
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
          portailSARLDownChanged = 1;
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

void roundDisplay(int value)
{
  if (value == 0)
  {
    digitalWrite(pinOliDisplay(0), HIGH);
    digitalWrite(pinOliDisplay(1), LOW);
    digitalWrite(pinOliDisplay(2), LOW);
    digitalWrite(pinOliDisplay(3), LOW);
    digitalWrite(pinOliDisplay(4), LOW);
    digitalWrite(pinOliDisplay(5), LOW);
    digitalWrite(pinOliDisplay(6), LOW);
    digitalWrite(pinOliDisplay(7), LOW);
  }
  else if (value == 1)
  {
    digitalWrite(pinOliDisplay(0), LOW);
    digitalWrite(pinOliDisplay(1), HIGH);
    digitalWrite(pinOliDisplay(2), LOW);
    digitalWrite(pinOliDisplay(3), LOW);
    digitalWrite(pinOliDisplay(4), LOW);
    digitalWrite(pinOliDisplay(5), LOW);
    digitalWrite(pinOliDisplay(6), LOW);
    digitalWrite(pinOliDisplay(7), LOW);
  }
  else if (value == 2)
  {
    digitalWrite(pinOliDisplay(0), LOW);
    digitalWrite(pinOliDisplay(1), LOW);
    digitalWrite(pinOliDisplay(2), HIGH);
    digitalWrite(pinOliDisplay(3), LOW);
    digitalWrite(pinOliDisplay(4), LOW);
    digitalWrite(pinOliDisplay(5), LOW);
    digitalWrite(pinOliDisplay(6), LOW);
    digitalWrite(pinOliDisplay(7), LOW);
  }
  else if (value == 3)
  {
    digitalWrite(pinOliDisplay(0), LOW);
    digitalWrite(pinOliDisplay(1), LOW);
    digitalWrite(pinOliDisplay(2), LOW);
    digitalWrite(pinOliDisplay(3), HIGH);
    digitalWrite(pinOliDisplay(4), LOW);
    digitalWrite(pinOliDisplay(5), LOW);
    digitalWrite(pinOliDisplay(6), LOW);
    digitalWrite(pinOliDisplay(7), LOW);
  }
  else if (value == 4)
  {
    digitalWrite(pinOliDisplay(0), LOW);
    digitalWrite(pinOliDisplay(1), LOW);
    digitalWrite(pinOliDisplay(2), LOW);
    digitalWrite(pinOliDisplay(3), LOW);
    digitalWrite(pinOliDisplay(4), HIGH);
    digitalWrite(pinOliDisplay(5), LOW);
    digitalWrite(pinOliDisplay(6), LOW);
    digitalWrite(pinOliDisplay(7), LOW);
  }
  else if (value == 5)
  {
    digitalWrite(pinOliDisplay(0), LOW);
    digitalWrite(pinOliDisplay(1), LOW);
    digitalWrite(pinOliDisplay(2), LOW);
    digitalWrite(pinOliDisplay(3), LOW);
    digitalWrite(pinOliDisplay(4), LOW);
    digitalWrite(pinOliDisplay(5), HIGH);
    digitalWrite(pinOliDisplay(6), LOW);
    digitalWrite(pinOliDisplay(7), LOW);
  }
  else if (value == 6)
  {
    digitalWrite(pinOliDisplay(0), LOW);
    digitalWrite(pinOliDisplay(1), LOW);
    digitalWrite(pinOliDisplay(2), LOW);
    digitalWrite(pinOliDisplay(3), LOW);
    digitalWrite(pinOliDisplay(4), LOW);
    digitalWrite(pinOliDisplay(5), LOW);
    digitalWrite(pinOliDisplay(6), HIGH);
    digitalWrite(pinOliDisplay(7), LOW);
  }
  else if (value == 7)
  {
    digitalWrite(pinOliDisplay(0), LOW);
    digitalWrite(pinOliDisplay(1), LOW);
    digitalWrite(pinOliDisplay(2), LOW);
    digitalWrite(pinOliDisplay(3), LOW);
    digitalWrite(pinOliDisplay(4), LOW);
    digitalWrite(pinOliDisplay(5), LOW);
    digitalWrite(pinOliDisplay(6), LOW);
    digitalWrite(pinOliDisplay(7), HIGH);
  }
  else
  {
    digitalWrite(pinOliDisplay(0), LOW);
    digitalWrite(pinOliDisplay(1), LOW);
    digitalWrite(pinOliDisplay(2), LOW);
    digitalWrite(pinOliDisplay(3), LOW);
    digitalWrite(pinOliDisplay(4), LOW);
    digitalWrite(pinOliDisplay(5), LOW);
    digitalWrite(pinOliDisplay(6), LOW);
    digitalWrite(pinOliDisplay(7), LOW);
  }
}

int main(char **args)
{
  int i;
  int fd;
  MYSQL mysql;
  char ch;
  printf("GSM Testing V0.2.1\n");
  tm = *localtime(&t);
  printf("Current time : %d:%d\n", tm.tm_hour, tm.tm_min);
  // connect to the MySQL database at localhost
  mysql_init(&mysql);
  connection = mysql_real_connect(&mysql,"localhost", "morind79", "DMO12lip6", "sms", 0, 0, 0);
  // check for a connection error
  if (connection == NULL)
  {
    printf(mysql_error(&mysql));
    return 1;
  }
  // Start with a clean database
  clearDB();
  // close the connection
  mysql_close(connection);
  // Setup GPIO
  wiringOliSetup();
  oliExtSetup();
  // I2C communication
  fd = wiringOliI2CSetup(0, 0x02);
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
    digitalWritePortB(i, LOW);
  }
  // Port C as output
  for (i=0; i<8; i++)
  {
    pinModePortC(i, OUTPUT);
    digitalWritePortC(i, LOW);
  }
  // Port D as output
  for (i=0; i<8; i++)
  {
    pinModePortD(i, OUTPUT);
    digitalWritePortD(i, LOW);
  }
  // We are ready, tell this to Arduino send 1 byte 'S' ASCII=0x53
  wiringOliI2CWriteReg8(fd, 0x00, 0x53);
  // Init of timers
  prevMDHouse = millis();
  // Start thread for RI pin
  int x1 = oliThreadCreate(ri);
  if (x1 != 0) printf ("RI didn't start\n");
  // Start alarm thread
  int x2 = oliThreadCreate(alarmPortA);
  if (x2 != 0) printf ("Alarm didn't start\n");
  int x4 = oliThreadCreate(portailMaison);
  if (x4 != 0) printf ("Portail maison didn't start\n");
  int x5 = oliThreadCreate(portail1);
  if (x5 != 0) printf ("Portail1 didn't start\n");
  int x6 = oliThreadCreate(portail2);
  if (x6 != 0) printf ("Portail2 didn't start\n");
  int x7 = oliThreadCreate(portail3);
  if (x7 != 0) printf ("Portail3 didn't start\n");
  int x8 = oliThreadCreate(portailSARL);
  if (x8 != 0) printf ("Portail SARL didn't start\n");
  // Infinite loop
  for(;;)
  {
    ch=getchar();
    if(ch=='0')
    {
      // We stop, tell this to Arduino send 1 byte 's' ASCII=0x73
      wiringOliI2CWriteReg8(fd, 0x00, 0x73);
      break;
    }
    else if(ch=='A') digitalWritePortC(0, HIGH);
    else if(ch=='a') digitalWritePortC(0, LOW);
    else if(ch=='B') digitalWritePortC(1, HIGH);
    else if(ch=='b') digitalWritePortC(1, LOW);
    else if(ch=='C') digitalWritePortC(2, HIGH);
    else if(ch=='c') digitalWritePortC(2, LOW);
    else if(ch=='D') digitalWritePortC(3, HIGH);
    else if(ch=='d') digitalWritePortC(3, LOW);
    else if(ch=='E') digitalWritePortC(4, HIGH);
    else if(ch=='e') digitalWritePortC(4, LOW);
    else if(ch=='F') digitalWritePortC(5, HIGH);
    else if(ch=='f') digitalWritePortC(5, LOW);
    else if(ch=='G') digitalWritePortC(6, HIGH);
    else if(ch=='g') digitalWritePortC(6, LOW);
    else if(ch=='H') digitalWritePortC(7, HIGH);
    else if(ch=='h') digitalWritePortC(7, LOW);
    else if(ch=='I') digitalWritePortB(0, HIGH);
    else if(ch=='i') digitalWritePortB(0, LOW);
    else if(ch=='J') digitalWritePortB(1, HIGH);
    else if(ch=='j') digitalWritePortB(1, LOW);
    else if(ch=='K') digitalWritePortB(2, HIGH);
    else if(ch=='k') digitalWritePortB(2, LOW);
    else if(ch=='L') digitalWritePortB(3, HIGH);
    else if(ch=='l') digitalWritePortB(3, LOW);
    else if(ch=='M') portailMaisonUpChanged = 1;
    else if(ch=='N') portailMaisonDownChanged = 1;
    else if(ch=='O') portail1UpChanged = 1;
    else if(ch=='P') portail1DownChanged = 1;
    else if(ch=='Q') portail2UpChanged = 1;
    else if(ch=='R') portail2DownChanged = 1;
    else if(ch=='S') portail3UpChanged = 1;
    else if(ch=='T') portail3DownChanged = 1;
    else if(ch=='U') portailSARLUpChanged = 1;
    else if(ch=='V') portailSARLDownChanged = 1;

    delay(100);
  }
  printf("Done.\n");
}
