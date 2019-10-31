/*
 * Pascal
 *
 *  /home/denis/pascal/main.cpp
 *  gcc -Wall -o sms main.cpp -lwiringOli $(mysql_config --cflags) $(mysql_config --libs)
 *  ./sms
 *
 * @author denm
 */

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
void printDate();

unsigned long prevMDHouse;               // Variable used for knowing time since last sent SMS
int powerFail = 10;                      // 0 -> No power, 10 -> Power ok, and is decreased every 100ms till 0 to give 1s without power before sending SMS
unsigned long timeBetweenSMS = 10000;    // Waiting time before sending another SMS in ms
const char *PASCAL = "+33624323080";     // Phone numbers
const char *SEBASTIEN = "+33635526656";  //
const char *DENIS = "+33626281284";      //
const char *PIERRE = "+33695720863";     //
int fd;                                  // File descriptor for I2C slave Arduino

// Specific functions
int sendSMSEnablePF = 1;                 // Enable/disable SMS sent in case of power failure

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
      printf("RI detected... ");
      printDate();
      // Connect to the MySQL database at localhost
      mysql_init(&mysql);
      connection = mysql_real_connect(&mysql,"localhost", "root", "DMO12lip6", "smsd", 0, 0, 0);
      // Check for a connection error
      if (connection == NULL)
      {
        printf(mysql_error(&mysql));
      }
      // Wait time Gammu store SMS in database
      delay(10000);
      // Something happened
      for (i=0; i<30; i++)
      {
        if (readDB() > 0) break;
        delay(4000);
      }
      // Close the connection
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
      // Connect to the MySQL database at localhost
      mysql_init(&mysql);
      connection = mysql_real_connect(&mysql,"localhost", "root", "DMO12lip6", "smsd", 0, 0, 0);
      // Check for a connection error
      if (connection == NULL)
      {
        printf(mysql_error(&mysql));
      }
      clearDB();
      clearForToday = 1;
      printf("Clean DB at : %d:%d\n", tm.tm_hour, tm.tm_min);
      // Close the connection
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
  oliHiPri(1);
  printf("Alarm thread starts...\n"); fflush(stdout);
  // Loop
  while(1)
  {
    // Zone 1 motion detection PortA0 - Only when C7 = Zone 1 is active
    if ((digitalReadPortA(0) == 0) && (digitalReadPortC(7) == 1))
    {
      status = concat(0);
      status = concat(2, status, "Detection mouvement Zone 1 (Hangar)");
      // Wait some time before sending a new SMS
      if (millis() - prevMDHouse >= timeBetweenSMS)
      {
        printf("Detection mouvement Zone 1 (Hangar) ");
        sendSMSAlarm(status);
        printDate();
        prevMDHouse = millis();
      }
  	  free(status);
    }
    // Zone 2 motion detection PortA1 - Only when B0 = Zone 2 is active
    if ((digitalReadPortA(1) == 0) && (digitalReadPortB(0) == 1))
    {
      status = concat(0);
      status = concat(2, status, "Detection mouvement Zone 2 (tracteur)");
      // Wait some time before sending a new SMS
      if (millis() - prevMDHouse >= timeBetweenSMS)
      {
        printf("Detection mouvement Zone 2 (tracteur) ");
        sendSMSAlarm(status);
        printDate();
        prevMDHouse = millis();
      }
  	  free(status);
    }
    // Zone 3 motion detection PortA2 - Only when C0 = Zone 3 is active
    if ((digitalReadPortA(2) == 0) && (digitalReadPortC(0) == 1))
    {
      status = concat(0);
      status = concat(2, status, "Detection mouvement dans la maison.");
      // Wait some time before sending a new SMS
      if (millis() - prevMDHouse >= timeBetweenSMS)
      {
        printf("Detection mouvement dans la maison ");
        sendSMSAlarm(status);
        printDate();
        prevMDHouse = millis();
      }
  	  free(status);
    }
/*
    // House IR barrier detection PortA3 - Only when B0 = Zone 2 is active
    if ((digitalReadPortA(3) == 0) && (digitalReadPortB(1) == 1))
    {
      status = concat(0);
      status = concat(2, status, "Detection barriere IR coté maison.");
      // Wait some time before sending a new SMS
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
      // Wait some time before sending a new SMS
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
      // Wait some time before sending a new SMS
      if (millis() - prevMDHouse >= timeBetweenSMS)
      {
        prevMDHouse = millis();
        sendSMSAlarm(status);
      }
  	  free(status);
    }
*/
    // Power failure PortA8
    if (digitalReadPortA(8) == 0)
    {
      // Loop is 1000ms so count 10 time to have 10s power failure at least
      powerFail--;
      // Send message only when powerFail is lower or equals to 0
      if (powerFail <= 0)
      {
        powerFail = 0;
        status = concat(0);
        status = concat(2, status, "Coupure de courant.");
        // Wait some time before sending a new SMS
        if (millis() - prevMDHouse >= timeBetweenSMS)
        {
          printf("Coupure de courant ");
          prevMDHouse = millis();
          printDate();
          // Send SMS only when user enable it
          if (sendSMSEnablePF == 1) sendSMSAlarm(status);
        }
        free(status);
      }
    }
    if ((digitalReadPortA(8) == 1) && (powerFail == 0))
    {
      powerFail = 10;
      status = concat(0);
      status = concat(2, status, "Courant revenu.");
      // Wait some time before sending a new SMS
      if (millis() - prevMDHouse >= timeBetweenSMS)
      {
        printf("Courant revenu ");
        prevMDHouse = millis();
        printDate();
        sendSMSAlarm(status);
      }
      free(status);
    }
    if (digitalReadPortA(8) == 1) powerFail = 10;
    delay(1000);
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
      wiringOliI2CWriteReg8(fd, 0x00, 0x53);
      printf("Portail maison up "); printDate(); fflush(stdout);
      int timout = 0;
      while(timout < PORTAIL_TIMER)
      {
        delay(1);
        timout++;
      }
      // Motor off
      digitalWritePortB(portailMotorUp[0], LOW);
      wiringOliI2CWriteReg8(fd, 0x00, 0x53);
    }
    // Check if button status changed
    if (portailMaisonDownChanged == 1)
    {
      portailMaisonDownChanged = 0;
      digitalWritePortB(portailMotorUp[0], LOW);
      digitalWritePortB(portailMotorDown[0], HIGH);
      wiringOliI2CWriteReg8(fd, 0x00, 0x53);
      printf("Portail maison down "); printDate(); fflush(stdout);
      int timout = 0;
      while(timout < PORTAIL_TIMER)
      {
        delay(1);
        timout++;
      }
      // Motor off
      digitalWritePortB(portailMotorDown[0], LOW);
      wiringOliI2CWriteReg8(fd, 0x00, 0x53);
    }
    delay(100);
  }
}

OLI_THREAD(portail1)
{
  oliHiPri(10);
  printf("Portail1 thread starts (Cote route)...\n"); fflush(stdout);
  // Loop
  while(1)
  {
    // Check if button status changed
    if (portail1UpChanged == 1)
    {
      portail1UpChanged = 0;
      digitalWritePortB(portailMotorDown[1], LOW);
      digitalWritePortB(portailMotorUp[1], HIGH);
      wiringOliI2CWriteReg8(fd, 0x00, 0x53);
      printf("Portail1 up (cote route) "); printDate(); fflush(stdout);
      int timout = 0;
      while(timout < PORTAIL_TIMER)
      {
        delay(1);
        timout++;
      }
      // Motor off
      digitalWritePortB(portailMotorUp[1], LOW);
      wiringOliI2CWriteReg8(fd, 0x00, 0x53);
    }
    // Check if button status changed
    if (portail1DownChanged == 1)
    {
      portail1DownChanged = 0;
      digitalWritePortB(portailMotorUp[1], LOW);
      digitalWritePortB(portailMotorDown[1], HIGH);
      wiringOliI2CWriteReg8(fd, 0x00, 0x53);
      printf("Portail1 down (cote route) "); printDate(); fflush(stdout);
      int timout = 0;
      while(timout < PORTAIL_TIMER)
      {
        delay(1);
        timout++;
      }
      // Motor off
      digitalWritePortB(portailMotorDown[1], LOW);
      wiringOliI2CWriteReg8(fd, 0x00, 0x53);
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
      wiringOliI2CWriteReg8(fd, 0x00, 0x53);
      printf("Portail2 up "); printDate(); fflush(stdout);
      int timout = 0;
      while(timout < PORTAIL_TIMER)
      {
        delay(1);
        timout++;
      }
      // Motor off
      digitalWritePortB(portailMotorUp[2], LOW);
      wiringOliI2CWriteReg8(fd, 0x00, 0x53);
    }
    // Check if button status changed
    if (portail2DownChanged == 1)
    {
      portail2DownChanged = 0;
      digitalWritePortB(portailMotorUp[2], LOW);
      digitalWritePortB(portailMotorDown[2], HIGH);
      wiringOliI2CWriteReg8(fd, 0x00, 0x53);
      printf("Portail2 down "); printDate(); fflush(stdout);
      int timout = 0;
      while(timout < PORTAIL_TIMER)
      {
        delay(1);
        timout++;
      }
      // Motor off
      digitalWritePortB(portailMotorDown[2], LOW);
      wiringOliI2CWriteReg8(fd, 0x00, 0x53);
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
      wiringOliI2CWriteReg8(fd, 0x00, 0x53);
      printf("Portail3 up "); printDate(); fflush(stdout);
      int timout = 0;
      while(timout < PORTAIL_TIMER)
      {
        delay(1);
        timout++;
      }
      // Motor off
      digitalWritePortB(portailMotorUp[3], LOW);
      wiringOliI2CWriteReg8(fd, 0x00, 0x53);
    }
    // Check if button status changed
    if (portail3DownChanged == 1)
    {
      portail3DownChanged = 0;
      digitalWritePortB(portailMotorUp[3], LOW);
      digitalWritePortB(portailMotorDown[3], HIGH);
      wiringOliI2CWriteReg8(fd, 0x00, 0x53);
      printf("Portail3 down "); printDate(); fflush(stdout);
      int timout = 0;
      while(timout < PORTAIL_TIMER)
      {
        delay(1);
        timout++;
      }
      // Motor off
      digitalWritePortB(portailMotorDown[3], LOW);
      wiringOliI2CWriteReg8(fd, 0x00, 0x53);
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
      wiringOliI2CWriteReg8(fd, 0x00, 0x53);
      printf("Portail SARL up "); printDate(); fflush(stdout);
      int timout = 0;
      while(timout < PORTAIL_TIMER)
      {
        delay(1);
        timout++;
      }
      // Motor off
      digitalWritePortB(portailMotorUp[4], LOW);
      wiringOliI2CWriteReg8(fd, 0x00, 0x53);
    }
    // Check if button status changed
    if (portailSARLDownChanged == 1)
    {
      portailSARLDownChanged = 0;
      digitalWritePortB(portailMotorUp[4], LOW);
      digitalWritePortB(portailMotorDown[4], HIGH);
      wiringOliI2CWriteReg8(fd, 0x00, 0x53);
      printf("Portail SARL down "); printDate(); fflush(stdout);
      int timout = 0;
      while(timout < PORTAIL_TIMER)
      {
        delay(1);
        timout++;
      }
      // Motor off
      digitalWritePortB(portailMotorDown[4], LOW);
      wiringOliI2CWriteReg8(fd, 0x00, 0x53);
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
  state = mysql_query(connection, "DELETE FROM outbox");
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
  // Must call mysql_store_result( ) before you can issue any other query calls
  result = mysql_store_result(connection);
  num = mysql_num_rows(result);
  printf("Rows: %d ", num); printDate();
  // Process each row in the result set
  while ((row = mysql_fetch_row(result)) != NULL)
  {
    // Get SMS Text
    smsText = row[0] ? row[0] : "NULL";
    smsNumber = row[1] ? row[1] : "NULL";
    index = row[2] ? atoi(row[2]) : 0;
    status = decodeSMS(smsNumber, smsText);
    smsProcessedDB(index);
    if (status != '\0')
    {
      printf("Before sendSMSDB\n");
      sendSMSDB(smsNumber, status);
      printf("Before sendSMSDB\n");
    }
    printf("Before printf\n");
    printf("Text: %s, Number: %s, index: %d ", smsText, smsNumber, index);
    printf("After printf\n");
    printDate();
  }
  // Free the result set
  mysql_free_result(result);
  return(num);
}

void sendSMSAlarm(char *message)
{
  MYSQL mysql;
  oliLock(0);
  // Connect to the MySQL database at localhost
  mysql_init(&mysql);
  connection = mysql_real_connect(&mysql,"localhost", "root", "DMO12lip6", "smsd", 0, 0, 0);
  // Check for a connection error
  if (connection == NULL)
  {
    printf(mysql_error(&mysql));
  }
  sendSMSDB(PASCAL, message);
  sendSMSDB(SEBASTIEN, message);
  sendSMSDB(PIERRE, message);
  // Close the connection
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
  char *cmd;
  char *status;
  // Compare number to be sure this is an allowed message
  if ((strcmp(number, DENIS) == 0) || (strcmp(number, PASCAL) == 0) || (strcmp(number, SEBASTIEN) == 0) || (strcmp(number, PIERRE) == 0))
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
        printf("cmd='%s' ", cmd); printDate();
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
      printf("status=%s", status); printDate();
      return(status);
      free(status);
    }
  }
  return('\0');
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
        if (val == '1') {digitalWritePortC(0, HIGH); wiringOliI2CWriteReg8(fd, 0x00, 0x53);}
        else {digitalWritePortC(0, LOW); wiringOliI2CWriteReg8(fd, 0x00, 0x53);}
        break;
      case 'B' :  // C1
        if (val == '1') {digitalWritePortC(1, HIGH); wiringOliI2CWriteReg8(fd, 0x00, 0x53);}
        else {digitalWritePortC(1, LOW); wiringOliI2CWriteReg8(fd, 0x00, 0x53);}
        break;
      case 'C' :  // C2
        if (val == '1') {digitalWritePortC(2, HIGH); wiringOliI2CWriteReg8(fd, 0x00, 0x53);}
        else {digitalWritePortC(2, LOW); wiringOliI2CWriteReg8(fd, 0x00, 0x53);}
        break;
      case 'D' :  // C3
        if (val == '1') {digitalWritePortC(3, HIGH); wiringOliI2CWriteReg8(fd, 0x00, 0x53);}
        else {digitalWritePortC(3, LOW); wiringOliI2CWriteReg8(fd, 0x00, 0x53);}
        break;
      case 'E' :  // C4
        if (val == '1') {digitalWritePortC(4, HIGH); wiringOliI2CWriteReg8(fd, 0x00, 0x53);}
       else {digitalWritePortC(4, LOW); wiringOliI2CWriteReg8(fd, 0x00, 0x53);}
       break;
      case 'F' :  // C5
        if (val == '1') {digitalWritePortC(5, HIGH); wiringOliI2CWriteReg8(fd, 0x00, 0x53);}
        else {digitalWritePortC(5, LOW); wiringOliI2CWriteReg8(fd, 0x00, 0x53);}
        break;
      case 'G' :  // C6
        if (val == '1') {digitalWritePortC(6, HIGH); wiringOliI2CWriteReg8(fd, 0x00, 0x53);}
        else {digitalWritePortC(6, LOW); wiringOliI2CWriteReg8(fd, 0x00, 0x53);}
        break;
      case 'H' :  // C7
        if (val == '1') {digitalWritePortC(7, HIGH); wiringOliI2CWriteReg8(fd, 0x00, 0x53);}
        else {digitalWritePortC(7, LOW); wiringOliI2CWriteReg8(fd, 0x00, 0x53);}
        break;
      case 'I' :  // B0
        if (val == '1') {digitalWritePortB(0, HIGH); wiringOliI2CWriteReg8(fd, 0x00, 0x53);}
        else {digitalWritePortB(0, LOW); wiringOliI2CWriteReg8(fd, 0x00, 0x53);}
        break;
      case 'J' :  // B1
        if (val == '1') {digitalWritePortB(1, HIGH); wiringOliI2CWriteReg8(fd, 0x00, 0x53);}
        else {digitalWritePortB(1, LOW); wiringOliI2CWriteReg8(fd, 0x00, 0x53);}
        break;
      case 'K' :  // B2
        if (val == '1') {digitalWritePortB(2, HIGH); wiringOliI2CWriteReg8(fd, 0x00, 0x53);}
        else {digitalWritePortB(2, LOW); wiringOliI2CWriteReg8(fd, 0x00, 0x53);}
        break;
      case 'L' :  // B3
        if (val == '1') {digitalWritePortB(3, HIGH); wiringOliI2CWriteReg8(fd, 0x00, 0x53);}
        else {digitalWritePortB(3, LOW); wiringOliI2CWriteReg8(fd, 0x00, 0x53);}
        break;
      case 'M' :  // B4
        if (val == '1')
        {
          portailMaisonUpChanged = 1;
        }
        else
        {
          {digitalWritePortB(4, LOW); wiringOliI2CWriteReg8(fd, 0x00, 0x53);}
          {digitalWritePortB(5, LOW); wiringOliI2CWriteReg8(fd, 0x00, 0x53);}
        }
        break;
      case 'N' :  // B5
        if (val == '1')
        {
          portailMaisonDownChanged = 1;
        }
        else
        {
          {digitalWritePortB(5, LOW); wiringOliI2CWriteReg8(fd, 0x00, 0x53);}
          {digitalWritePortB(4, LOW); wiringOliI2CWriteReg8(fd, 0x00, 0x53);}
        }
        break;
      case 'O' :  // B6
        if (val == '1')
        {
          portail1UpChanged = 1;
        }
        else
        {
          {digitalWritePortB(6, LOW); wiringOliI2CWriteReg8(fd, 0x00, 0x53);}
          {digitalWritePortB(7, LOW); wiringOliI2CWriteReg8(fd, 0x00, 0x53);}
        }
        break;
      case 'P' :  // B7
        if (val == '1')
        {
          portail1DownChanged = 1;
        }
        else
        {
          {digitalWritePortB(7, LOW); wiringOliI2CWriteReg8(fd, 0x00, 0x53);}
          {digitalWritePortB(6, LOW); wiringOliI2CWriteReg8(fd, 0x00, 0x53);}
        }
        break;
      case 'Q' :  // B8
        if (val == '1')
        {
          portail2UpChanged = 1;
        }
        else
        {
          {digitalWritePortB(8, LOW); wiringOliI2CWriteReg8(fd, 0x00, 0x53);}
          {digitalWritePortB(9, LOW); wiringOliI2CWriteReg8(fd, 0x00, 0x53);}
        }
        break;
      case 'R' :  // B9
        if (val == '1')
        {
          portail2DownChanged = 1;
        }
        else
        {
          {digitalWritePortB(9, LOW); wiringOliI2CWriteReg8(fd, 0x00, 0x53);}
          {digitalWritePortB(8, LOW); wiringOliI2CWriteReg8(fd, 0x00, 0x53);}
        }
        break;
      case 'S' :  // B10
        if (val == '1')
        {
          portail3UpChanged = 1;
        }
        else
        {
          {digitalWritePortB(10, LOW); wiringOliI2CWriteReg8(fd, 0x00, 0x53);}
          {digitalWritePortB(11, LOW); wiringOliI2CWriteReg8(fd, 0x00, 0x53);}
        }
        break;
    case 'T' :  // B11
        if (val == '1')
        {
          portail3DownChanged = 1;
        }
        else
        {
          {digitalWritePortB(11, LOW); wiringOliI2CWriteReg8(fd, 0x00, 0x53);}
          {digitalWritePortB(10, LOW); wiringOliI2CWriteReg8(fd, 0x00, 0x53);}
        }
        break;
    case 'U' :  // B12
        if (val == '1')
        {
          portailSARLUpChanged = 1;
        }
        else
        {
          {digitalWritePortB(12, LOW); wiringOliI2CWriteReg8(fd, 0x00, 0x53);}
          {digitalWritePortB(13, LOW); wiringOliI2CWriteReg8(fd, 0x00, 0x53);}
        }
        break;
    case 'V' :  // B13
        if (val == '1')
        {
          portailSARLDownChanged = 1;
        }
        else
        {
          {digitalWritePortB(13, LOW); wiringOliI2CWriteReg8(fd, 0x00, 0x53);}
          {digitalWritePortB(12, LOW); wiringOliI2CWriteReg8(fd, 0x00, 0x53);}
        }
        break;
    case 'W' :  // sendSMSEnablePF
        if (val == '1')
        {
          sendSMSEnablePF = 1;
        }
        else
        {
          sendSMSEnablePF = 0;
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

void printDate()
{
  time_t now;
  time(&now);
  printf("%s", ctime(&now));
  fflush(stdout);
}

int main(void)
{
  int i;
  MYSQL mysql;
  char ch;
  printf("GSM Testing V0.2.5 171202\n");
  tm = *localtime(&t);
  printf("Current time : %d:%d\n", tm.tm_hour, tm.tm_min);
  // Connect to the MySQL database at localhost
  mysql_init(&mysql);
  connection = mysql_real_connect(&mysql,"localhost", "root", "DMO12lip6", "smsd", 0, 0, 0);
  // Check for a connection error
  if (connection == NULL)
  {
    printf(mysql_error(&mysql));
    return 1;
  }
  // Start with a clean database
  clearDB();
  // Close the connection
  mysql_close(connection);
  // Setup GPIO
  wiringOliSetup();
  oliExtSetup();
  // I2C communication
  fd = wiringOliI2CSetup(0, 0x02);
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
    else if (ch=='A') {digitalWritePortC(0, HIGH); wiringOliI2CWriteReg8(fd, 0x00, 0x53);}
    else if (ch=='a') {digitalWritePortC(0, LOW); wiringOliI2CWriteReg8(fd, 0x00, 0x53);}
    else if (ch=='B') {digitalWritePortC(1, HIGH); wiringOliI2CWriteReg8(fd, 0x00, 0x53);}
    else if (ch=='b') {digitalWritePortC(1, LOW); wiringOliI2CWriteReg8(fd, 0x00, 0x53);}
    else if (ch=='C') {digitalWritePortC(2, HIGH); wiringOliI2CWriteReg8(fd, 0x00, 0x53);}
    else if (ch=='c') {digitalWritePortC(2, LOW); wiringOliI2CWriteReg8(fd, 0x00, 0x53);}
    else if (ch=='D') {digitalWritePortC(3, HIGH); wiringOliI2CWriteReg8(fd, 0x00, 0x53);}
    else if (ch=='d') {digitalWritePortC(3, LOW); wiringOliI2CWriteReg8(fd, 0x00, 0x53);}
    else if (ch=='E') {digitalWritePortC(4, HIGH); wiringOliI2CWriteReg8(fd, 0x00, 0x53);}
    else if (ch=='e') {digitalWritePortC(4, LOW); wiringOliI2CWriteReg8(fd, 0x00, 0x53);}
    else if (ch=='F') {digitalWritePortC(5, HIGH); wiringOliI2CWriteReg8(fd, 0x00, 0x53);}
    else if (ch=='f') {digitalWritePortC(5, LOW); wiringOliI2CWriteReg8(fd, 0x00, 0x53);}
    else if (ch=='G') {digitalWritePortC(6, HIGH); wiringOliI2CWriteReg8(fd, 0x00, 0x53);}
    else if (ch=='g') {digitalWritePortC(6, LOW); wiringOliI2CWriteReg8(fd, 0x00, 0x53);}
    else if (ch=='H') {digitalWritePortC(7, HIGH); wiringOliI2CWriteReg8(fd, 0x00, 0x53);}
    else if (ch=='h') {digitalWritePortC(7, LOW); wiringOliI2CWriteReg8(fd, 0x00, 0x53);}
    else if (ch=='I') {digitalWritePortB(0, HIGH); wiringOliI2CWriteReg8(fd, 0x00, 0x53);}
    else if (ch=='i') {digitalWritePortB(0, LOW); wiringOliI2CWriteReg8(fd, 0x00, 0x53);}
    else if (ch=='J') {digitalWritePortB(1, HIGH); wiringOliI2CWriteReg8(fd, 0x00, 0x53);}
    else if (ch=='j') {digitalWritePortB(1, LOW); wiringOliI2CWriteReg8(fd, 0x00, 0x53);}
    else if (ch=='K') {digitalWritePortB(2, HIGH); wiringOliI2CWriteReg8(fd, 0x00, 0x53);}
    else if (ch=='k') {digitalWritePortB(2, LOW); wiringOliI2CWriteReg8(fd, 0x00, 0x53);}
    else if (ch=='L') {digitalWritePortB(3, HIGH); wiringOliI2CWriteReg8(fd, 0x00, 0x53);}
    else if (ch=='l') {digitalWritePortB(3, LOW); wiringOliI2CWriteReg8(fd, 0x00, 0x53);}
    else if (ch=='M') portailMaisonUpChanged = 1;
    else if (ch=='N') portailMaisonDownChanged = 1;
    else if (ch=='O') portail1UpChanged = 1;
    else if (ch=='P') portail1DownChanged = 1;
    else if (ch=='Q') portail2UpChanged = 1;
    else if (ch=='R') portail2DownChanged = 1;
    else if (ch=='S') portail3UpChanged = 1;
    else if (ch=='T') portail3DownChanged = 1;
    else if (ch=='U') portailSARLUpChanged = 1;
    else if (ch=='V') portailSARLDownChanged = 1;
    else if (ch=='W') sendSMSEnablePF = 1;
    else if (ch=='w') sendSMSEnablePF = 0;

    delay(100);
  }
  printf("Done.\n");
}
