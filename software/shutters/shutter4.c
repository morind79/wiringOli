/*
 * Shutter2 program
 *
 * Output pin on Port B
 *  B0 Salon up        B1 Salon down
 *  B2 Bathroom up     B3 Bathroom down
 *  B4 Erine up        B5 Erine down
 *  B6 Eva up          B7 Eva down
 *  B8 Laura up        B9 Laura down
 *  B10 sam up         B11 sam down
 *  B12 kitchen up     B13 Kitchen down
 *  B14 Cellier up     B15 Cellier down
 *
 * Input pin on Port A
 *  A1 Salon up        A0 Salon down
 *  A2 Bathroom up     A3 Bathroom down
 *  A4 Erine up        A5 Erine down
 *  A6 Eva up          A7 Eva down
 *  A8 Laura up        A9 Laura down
 *  A10 sam up         A11 sam down
 *  A12 kitchen up     A13 Kitchen down
 *  A14 Cellier up     A15 Cellier down
 *
 * Input pin on Port General
 *  PG0 General up     PG1 General down
 *
 * Input pin on Port C
 *  C0 Motion detector service door
 *  C1 Motion detector cars
 *  C2 Unused
 *  C3 Water meter
 *  C4 garage door Cecile
 *  C5 garage doot Denis
 *  C6 IR Barrier
 *  C7 Push button in bathroom
 *  run : gpio load i2c
 *
 * Input pin on Port D
 *  D0 Grid power failure
 *  gcc -Wall -o shutter shutter4.c teleinfo.c -lwiringOli $(mysql_config --cflags) $(mysql_config --libs) -lncurses
 *  ./shutter
 *
 * @author denm
 */
#include <wiringOli.h>
#include <wiringOliI2C.h>
#include <oliExt.h>
#include <i2cLcd.h>
#include <mcp23008.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <mysql/mysql.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <ncurses.h>
#include "teleinfo.h"

MYSQL *connection;
time_t t;
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
void queryEnergyMeter();
void queryWaterMeter();
void processEnergyMeter(int channel);

int alarmStatus = 0;          // When 0 set alarm off
unsigned long prevAlarm1;     // Variable used for knowing time since last alarm1
unsigned long prevAlarm2;     // Variable used for knowing time since last alarm2
int powerFail = 0;            // 0 -> No power, 1 -> Power ok
long timeBetweenSMS = 50000;  // Waiting time before sending another SMS in ms
int alarmActive = 0;          // 0 -> Alarm not active, 1 -> Alarme active
int fdMCPEnergy = 0;          // MCP file descriptor for energy meter board
int water = 0;                // Water volume measured
const char *DENIS = "+33626281284";

// Timing for shutters
const int SMALL = 15000;
const int MEDIUM = 20000;
const int BIG = 45000;

int motorUp[] = {1, 3, 5, 7, 9, 11, 12, 14};
int motorDown[] = {0, 2, 4, 6, 8, 10, 13, 15};

int buttonUp[] = {1, 3, 5, 7, 9, 11, 12, 14};
int buttonDown[] = {0, 2, 4, 6, 8, 10, 13, 15};

int buttonCentralizedUp = 0;
int buttonCentralizedDown = 1;

// Global variable for button status
int salonUp = 0;
int salonDown = 0;
int salonUpChanged = 0;
int salonDownChanged = 0;
int salonForceUp = 0;
int salonForceDown = 0;

int sdbUp = 0;
int sdbDown = 0;
int sdbUpChanged = 0;
int sdbDownChanged = 0;
int sdbForceUp = 0;
int sdbForceDown = 0;

int erineUp = 0;
int erineDown = 0;
int erineUpChanged = 0;
int erineDownChanged = 0;
int erineForceUp = 0;
int erineForceDown = 0;

int evaUp = 0;
int evaDown = 0;
int evaUpChanged = 0;
int evaDownChanged = 0;
int evaForceUp = 0;
int evaForceDown = 0;

int lauraUp = 0;
int lauraDown = 0;
int lauraUpChanged = 0;
int lauraDownChanged = 0;
int lauraForceUp = 0;
int lauraForceDown = 0;

int samUp = 0;
int samDown = 0;
int samUpChanged = 0;
int samDownChanged = 0;
int samForceUp = 0;
int samForceDown = 0;

int cuisineUp = 0;
int cuisineDown = 0;
int cuisineUpChanged = 0;
int cuisineDownChanged = 0;
int cuisineForceUp = 0;
int cuisineForceDown = 0;

int cellierUp = 0;
int cellierDown = 0;
int cellierUpChanged = 0;
int cellierDownChanged = 0;
int cellierForceUp = 0;
int cellierForceDown = 0;

int genUp = 0;
int genDown = 0;
int genUpChanged = 0;
int genDownChanged = 0;
int genForceUp = 0;
int genForceDown = 0;

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
      connection = mysql_real_connect(&mysql, "localhost", "gammu", "gammu", "SMS", 0, 0, 0);
      // check for a connection error
      if (connection == NULL)
      {
        printf(mysql_error(&mysql));
      }
      // Wait time Gammu store SMS in database
      delay(10000);
      // Something happened
      for (i=0; i<50; i++)
      {
        if (readDB() > 0) break;
        delay(5000);
      }
      // close the connection
      mysql_close(connection);
      oliUnlock(0);
    }
    // Get current time
    t = time(NULL);
    tm = *localtime(&t);
    // If time is 1:00 then clean DB
    if ((tm.tm_hour == 1) && (tm.tm_min == 0) && (clearForToday == 0))
    {
      oliLock(0);
      // connect to the MySQL database at localhost
      mysql_init(&mysql);
      connection = mysql_real_connect(&mysql,"localhost", "gammu", "gammu", "SMS", 0, 0, 0);
      // check for a connection error
      if (connection == NULL)
      {
        printf(mysql_error(&mysql));
      }
      clearDB();
      clearForToday = 1;
      queryEnergyMeter();
      queryWaterMeter();
      printf("Clean DB at : %d:%d\n", tm.tm_hour, tm.tm_min);
      // close the connection
      mysql_close(connection);
      oliUnlock(0);
    }
    if ((tm.tm_hour == 1) && (tm.tm_min == 5) && (clearForToday == 1))
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

OLI_THREAD(alarm)
{
  char *status;
  oliHiPri(1);
  int waterDone = 0;
  printf("Alarm thread starts...\n"); fflush(stdout);
  while(1)
  {
    // Port C0 = motion detector service door **********************************
    if ((digitalReadPortC(0) == 0) && (alarmActive == 1))
    {
      status = concat(0);
      status = concat(2, status, "Detection mouvement porte de service");
      if (millis() - prevAlarm1 >= timeBetweenSMS)
      {
        printf("Detecteur porte service ");
        sendSMSAlarm(status);
        alarmStatus = 1;
        printDate();
        prevAlarm1 = millis();
      }
      free(status);
    }
    // Port C1 = motion detector cars ******************************************
    if ((digitalReadPortC(1) == 0) && (alarmActive == 1))
    {
      status = concat(0);
      status = concat(2, status, "Detection mouvement voiture");
      if (millis() - prevAlarm2 >= timeBetweenSMS)
      {
        printf("Detecteur voiture ");
        sendSMSAlarm(status);
        alarmStatus = 1;
        printDate();
        prevAlarm2 = millis();
      }
      free(status);
    }

    // port C2 = detecteur zone 3 **********************************************
    //printf("%d %d %d %d %d %d %d %d\n", digitalReadPortC(0), digitalReadPortC(1), digitalReadPortC(2),
    //                                    digitalReadPortC(3), digitalReadPortC(4), digitalReadPortC(5),
    //                                    digitalReadPortC(6), digitalReadPortC(7));
    // Water meter
    // Port C3 = water meter ***************************************************
    if ((digitalReadPortC(3) == 0) && (waterDone == 0))
    {
      water++;
      printf("Water : %d\n", water);
      waterDone = 1;
    }
    else if ((digitalReadPortC(3) == 1) && (waterDone == 1))
    {
      waterDone = 0;
    }
    // Port C4 garage door *****************************************************
    if ((digitalReadPortC(4) == 1) && (alarmActive == 1))
    {
      status = concat(0);
      status = concat(2, status, "Portail Cecile ouvert");
      if (millis() - prevAlarm2 >= timeBetweenSMS)
      {
        printf("Portail Cecile ouvert ");
        sendSMSAlarm(status);
        alarmStatus = 1;
        printDate();
        prevAlarm2 = millis();
      }
      free(status);
    }
    // Port C5 garage door *****************************************************
    if ((digitalReadPortC(5) == 1) && (alarmActive == 1))
    {
      status = concat(0);
      status = concat(2, status, "Portail Denis ouvert");
      if (millis() - prevAlarm2 >= timeBetweenSMS)
      {
        printf("Portail Denis ouvert ");
        sendSMSAlarm(status);
        alarmStatus = 1;
        printDate();
        prevAlarm2 = millis();
      }
      free(status);
    }
    // Port C6 = IR Barrier detector *******************************************
    if ((digitalReadPortC(6) == 1) && (alarmActive == 1))
    {
      status = concat(0);
      status = concat(2, status, "IR Barrier");
      if (millis() - prevAlarm2 >= timeBetweenSMS)
      {
        printf("IR Barrier ");
        sendSMSAlarm(status);
        alarmStatus = 1;
        printDate();
        prevAlarm2 = millis();
      }
      free(status);
    }
    // Port D0 = Grid power failure
    if (digitalReadPortD(0) == 0)
    {
      status = concat(0);
      status = concat(2, status, "Grid power failure");
      if (millis() - prevAlarm2 >= timeBetweenSMS)
      {
        printf("Grid Poser failure ");
        sendSMSAlarm(status);
        alarmStatus = 1;
        printDate();
        prevAlarm2 = millis();
      }
      free(status);
    }
    delay(100);
  }
}

OLI_THREAD(alarmOff)
{
  oliHiPri(1);
  while(1)
  {
    if (alarmStatus == 1)
    {
      delay(30000);

    }  
    delay(1000);
  }
}

// Task testing button changes
OLI_THREAD(buttons)
{
  oliHiPri(5);
  // Loop
  for (;;)
  {
    // Read button only when power grid is present
    if (digitalReadPortD(0) == 1)
    {
      // Salon
      if (salonUp != digitalReadPortA(buttonUp[0]))
      {
        salonUp = digitalReadPortA(buttonUp[0]);
        salonForceUp = 0;
        salonForceDown = 0;
        if (salonUp != 0) {salonUpChanged = 1; printf("Salon button up = 1 "); fflush(stdout); printDate();}
        else {salonUpChanged = 0;  printf("Salon button up = 0 "); fflush(stdout); printDate();}
      }
      if (salonDown != digitalReadPortA(buttonDown[0]))
      {
        salonDown = digitalReadPortA(buttonDown[0]);
        salonForceUp = 0;
        salonForceDown = 0;
        if (salonDown != 0) {salonDownChanged = 1; printf("Salon button down = 1 "); fflush(stdout); printDate();}
        else {salonDownChanged = 0;  printf("Salon button down = 0 "); fflush(stdout); printDate();}
      }
      // Sdb
      if (sdbUp != digitalReadPortA(buttonUp[1]))
      {
        sdbUp = digitalReadPortA(buttonUp[1]);
        sdbForceUp = 0;
        sdbForceDown = 0;
        if (sdbUp != 0) { sdbUpChanged = 1; printf("Sdb button up = 1 "); fflush(stdout); printDate();}
        else { sdbUpChanged = 0;  printf("Sdb button up = 0 "); fflush(stdout); printDate();}
      }
      if (sdbDown != digitalReadPortA(buttonDown[1]))
      {
        sdbDown = digitalReadPortA(buttonDown[1]);
        sdbForceUp = 0;
        sdbForceDown = 0;
        if (sdbDown != 0) {sdbDownChanged = 1;  printf("Sdb button down = 1 "); fflush(stdout); printDate();}
        else {sdbDownChanged = 0;  printf("Sdb button down = 0 "); fflush(stdout); printDate();}
      }
      // Erine
      if (erineUp != digitalReadPortA(buttonUp[2]))
      {
        erineUp = digitalReadPortA(buttonUp[2]);
        erineForceUp = 0;
        erineForceDown = 0;
        if (erineUp != 0) {erineUpChanged = 1;  printf("Erine button up = 1 "); fflush(stdout); printDate();}
        else {erineUpChanged = 0;  printf("Erine button up = 0 "); fflush(stdout); printDate();}
      }
      if (erineDown != digitalReadPortA(buttonDown[2]))
      {
        erineDown = digitalReadPortA(buttonDown[2]);
        erineForceUp = 0;
        erineForceDown = 0;
        if (erineDown != 0) {erineDownChanged = 1;  printf("Erine button down = 1 "); fflush(stdout); printDate();}
        else {erineDownChanged = 0;  printf("Erine button down = 0 "); fflush(stdout); printDate();}
      }
      // Eva
      if (evaUp != digitalReadPortA(buttonUp[3]))
      {
        evaUp = digitalReadPortA(buttonUp[3]);
        evaForceUp = 0;
        evaForceDown = 0;
        if (evaUp != 0) {evaUpChanged = 1;  printf("Eva button up = 1 "); fflush(stdout); printDate();}
        else {evaUpChanged = 0;  printf("Eva button up = 0 "); fflush(stdout); printDate();}
      }
      if (evaDown != digitalReadPortA(buttonDown[3]))
      {
        evaDown = digitalReadPortA(buttonDown[3]);
        evaForceUp = 0;
        evaForceDown = 0;
        if (evaDown != 0) {evaDownChanged = 1;  printf("Eva button down = 1 "); fflush(stdout); printDate();}
        else {evaDownChanged = 0;  printf("Eva button down = 0 "); fflush(stdout); printDate();}
      }
      // Laura
      if (lauraUp != digitalReadPortA(buttonUp[4]))
      {
        lauraUp = digitalReadPortA(buttonUp[4]);
        lauraForceUp = 0;
        lauraForceDown = 0;
        if (lauraUp != 0) {lauraUpChanged = 1;  printf("Laura button up = 1 "); fflush(stdout); printDate();}
        else {lauraUpChanged = 0;  printf("Laura button up = 0 "); fflush(stdout); printDate();}
      }
      if (lauraDown != digitalReadPortA(buttonDown[4]))
      {
        lauraDown = digitalReadPortA(buttonDown[4]);
        lauraForceUp = 0;
        lauraForceDown = 0;
        if (lauraDown != 0) {lauraDownChanged = 1;  printf("Laura button down = 1 "); fflush(stdout); printDate();}
        else {lauraDownChanged = 0;  printf("Laura button down = 0 "); fflush(stdout); printDate();}
      }
      // Sam
      if (samUp != digitalReadPortA(buttonUp[5]))
      {
        samUp = digitalReadPortA(buttonUp[5]);
        samForceUp = 0;
        samForceDown = 0;
        if (samUp != 0) {samUpChanged = 1;  printf("Sam button up = 1 "); fflush(stdout); printDate();}
        else {samUpChanged = 0;  printf("Sam button up = 0 "); fflush(stdout); printDate();}
      }
      if (samDown != digitalReadPortA(buttonDown[5]))
      {
        samDown = digitalReadPortA(buttonDown[5]);
        samForceUp = 0;
        samForceDown = 0;
        if (samDown != 0) {samDownChanged = 1;  printf("Sam button down = 1 "); fflush(stdout); printDate();}
        else {samDownChanged = 0;  printf("Sam button down = 0 "); fflush(stdout); printDate();}
      }
      // Cuisine
      if (cuisineUp != digitalReadPortA(buttonUp[6]))
      {
        cuisineUp = digitalReadPortA(buttonUp[6]);
        cuisineForceUp = 0;
        cuisineForceDown = 0;
        if (cuisineUp != 0) {cuisineUpChanged = 1;  printf("Cuisine button up = 1 "); fflush(stdout); printDate();}
        else {cuisineUpChanged = 0;  printf("Cuisine  button up = 0 "); fflush(stdout); printDate();}
      }
      if (cuisineDown != digitalReadPortA(buttonDown[6]))
      {
        cuisineDown = digitalReadPortA(buttonDown[6]);
        cuisineForceUp = 0;
        cuisineForceDown = 0;
        if (cuisineDown != 0) {cuisineDownChanged = 1;  printf("Cuisine button down = 1 "); fflush(stdout); printDate();}
        else {cuisineDownChanged = 0;  printf("Cuisine button down = 0 "); fflush(stdout); printDate();}
      }
      // Cellier
      if (cellierUp != digitalReadPortA(buttonUp[7]))
      {
        cellierUp = digitalReadPortA(buttonUp[7]);
        cellierForceUp = 0;
        cellierForceDown = 0;
        if (cellierUp != 0) {cellierUpChanged = 1;  printf("Cellier button up = 1 "); fflush(stdout); printDate();}
        else {cellierUpChanged = 0;  printf("Cellier button up = 0 "); fflush(stdout); printDate();}
      }
      if (cellierDown != digitalReadPortA(buttonDown[7]))
      {
        cellierDown = digitalReadPortA(buttonDown[7]);
        cellierForceUp = 0;
        cellierForceDown = 0;
        if (cellierDown != 0) {cellierDownChanged = 1;  printf("Cellier button down = 1 "); fflush(stdout); printDate();}
        else {cellierDownChanged = 0;  printf("Cellier button down = 0 "); fflush(stdout); printDate();}
      }
      // Gen
      if (genUp != digitalReadPortGeneral(buttonCentralizedUp))
      {
        genUp = digitalReadPortGeneral(buttonCentralizedUp);
        genForceUp = 0;
        genForceDown = 0;
        if (genUp != 0) {genUpChanged = 1;  printf("Gen button up = 1 "); fflush(stdout); printDate();}
        else {genUpChanged = 0;  printf("Gen button up = 0 "); fflush(stdout); printDate();}
      }
      if (genDown != digitalReadPortGeneral(buttonCentralizedDown))
      {
        genDown = digitalReadPortGeneral(buttonCentralizedDown);
        genForceUp = 0;
        genForceDown = 0;
        if (genDown != 0) {genDownChanged = 1;  printf("Gen button down = 1 "); fflush(stdout); printDate();}
        else {genDownChanged = 0;  printf("Gen button down = 0 "); fflush(stdout); printDate();}
      }
    }
    delay(5);
  }
}

OLI_THREAD(salon)
{
  oliHiPri(10);
  printf("Shutter thread salon starts...\n"); fflush(stdout);
  // Loop
  while(1)
  {
    // Check if button status changed
    if (salonUpChanged == 1)
    {
      salonUpChanged = 0;
      digitalWritePortB(motorDown[0], LOW);
      digitalWritePortB(motorUp[0], HIGH);
      printf("Salon up\n"); fflush(stdout);
      int timout = 0;
      while((timout < MEDIUM) && ((digitalReadPortA(buttonUp[0]) == 1) || (salonForceUp == 1)))
      {
        delay(1);
        timout++;
      }
      // Motor off
      digitalWritePortB(motorUp[0], LOW);
      salonForceUp = 0;
    }
    // Check if button status changed
    if (salonDownChanged == 1)
    {
      salonDownChanged = 0;
      digitalWritePortB(motorUp[0], LOW);
      digitalWritePortB(motorDown[0], HIGH);
      printf("Salon down\n"); fflush(stdout);
      int timout = 0;
      while((timout < MEDIUM) && ((digitalReadPortA(buttonDown[0]) == 1) || (salonForceDown == 1)))
      {
        delay(1);
        timout++;
      }
      // Motor off
      digitalWritePortB(motorDown[0], LOW);
      salonForceDown = 0;
    }
    delay(100);
  }
}

OLI_THREAD(sdb)
{
  oliHiPri(20);
  printf("Shutter thread sdb starts...\n"); fflush(stdout);
  // Loop
  while(1)
  {
    // Check if button status changed
    if (sdbUpChanged == 1)
    {
      sdbUpChanged = 0;
      digitalWritePortB(motorDown[1], LOW);
      digitalWritePortB(motorUp[1], HIGH);
      printf("Sdb up\n"); fflush(stdout);
      int timout = 0;
      while((timout < SMALL) && ((digitalReadPortA(buttonUp[1]) == 1) || (sdbForceUp == 1)))
      {
        delay(1);
        timout++;
      }
      // Motor off
      digitalWritePortB(motorUp[1], LOW);
      sdbForceUp = 0;
    }
    // Check if button status changed
    if (sdbDownChanged == 1)
    {
      sdbDownChanged = 0;
      digitalWritePortB(motorUp[1], LOW);
      digitalWritePortB(motorDown[1], HIGH);
      printf("Sdb down\n"); fflush(stdout);
      int timout = 0;
      while((timout < SMALL) && ((digitalReadPortA(buttonDown[1]) == 1) || (sdbForceDown == 1)))
      {
        delay(1);
        timout++;
      }
      // Motor off
      digitalWritePortB(motorDown[1], LOW);
      sdbForceDown = 0;
    }
    delay(100);
  }
}

OLI_THREAD(erine)
{
  oliHiPri(30);
  printf("Shutter thread Erine starts...\n"); fflush(stdout);
  // Loop
  while(1)
  {
    // Check if button status changed
    if (erineUpChanged == 1)
    {
      erineUpChanged = 0;
      digitalWritePortB(motorDown[2], LOW);
      digitalWritePortB(motorUp[2], HIGH);
      printf("Erine up\n"); fflush(stdout);
      int timout = 0;
      while((timout < MEDIUM) && ((digitalReadPortA(buttonUp[2]) == 1) || (erineForceUp == 1)))
      {
        delay(1);
        timout++;
      }
      // Motor off
      digitalWritePortB(motorUp[2], LOW);
      erineForceUp = 0;
    }
    // Check if button status changed
    if (erineDownChanged == 1)
    {
      erineDownChanged = 0;
      digitalWritePortB(motorUp[2], LOW);
      digitalWritePortB(motorDown[2], HIGH);
      printf("Erine down\n"); fflush(stdout);
      int timout = 0;
      while((timout < MEDIUM) && ((digitalReadPortA(buttonDown[2]) == 1) || (erineForceDown == 1)))
      {
        delay(1);
        timout++;
      }
      // Motor off
      digitalWritePortB(motorDown[2], LOW);
      erineForceDown = 0;
    }
    delay(100);
  }
}

OLI_THREAD(eva)
{
  oliHiPri(40);
  printf("Shutter thread Eva starts...\n"); fflush(stdout);
  // Loop
  while(1)
  {
    // Check if button status changed
    if (evaUpChanged == 1)
    {
      evaUpChanged = 0;
      digitalWritePortB(motorDown[3], LOW);
      digitalWritePortB(motorUp[3], HIGH);
      printf("Eva up\n"); fflush(stdout);
      int timout = 0;
      while((timout < MEDIUM) && ((digitalReadPortA(buttonUp[3]) == 1) || (evaForceUp == 1)))
      {
        delay(1);
        timout++;
      }
      // Motor off
      digitalWritePortB(motorUp[3], LOW);
      evaForceUp = 0;
    }
    // Check if button status changed
    if (evaDownChanged == 1)
    {
      evaDownChanged = 0;
      digitalWritePortB(motorUp[3], LOW);
      digitalWritePortB(motorDown[3], HIGH);
      printf("Eva down\n"); fflush(stdout);
      int timout = 0;
      while((timout < MEDIUM) && ((digitalReadPortA(buttonDown[3]) == 1) || (evaForceDown == 1)))
      {
        delay(1);
        timout++;
      }
      // Motor off
      digitalWritePortB(motorDown[3], LOW);
      evaForceDown = 0;
    }
    delay(100);
  }
}

OLI_THREAD(laura)
{
  oliHiPri(50);
  printf("Shutter thread Laura starts...\n"); fflush(stdout);
  // Loop
  while(1)
  {
    // Check if button status changed
    if (lauraUpChanged == 1)
    {
      lauraUpChanged = 0;
      digitalWritePortB(motorDown[4], LOW);
      digitalWritePortB(motorUp[4], HIGH);
      printf("Laura up\n"); fflush(stdout);
      int timout = 0;
      while((timout < MEDIUM) && ((digitalReadPortA(buttonUp[4]) == 1) || (lauraForceUp == 1)))
      {
        delay(1);
        timout++;
      }
      // Motor off
      digitalWritePortB(motorUp[4], LOW);
      lauraForceUp = 0;
    }
    // Check if button status changed
    if (lauraDownChanged == 1)
    {
      lauraDownChanged = 0;
      digitalWritePortB(motorUp[4], LOW);
      digitalWritePortB(motorDown[4], HIGH);
      printf("Laura down\n"); fflush(stdout);
      int timout = 0;
      while((timout < MEDIUM) && ((digitalReadPortA(buttonDown[4]) == 1) || (lauraForceDown == 1)))
      {
        delay(1);
        timout++;
      }
      // Motor off
      digitalWritePortB(motorDown[4], LOW);
      lauraForceDown = 0;
    }
    delay(100);
  }
}

OLI_THREAD(sam)
{
  oliHiPri(60);
  printf("Shutter thread Sam starts...\n"); fflush(stdout);
  // Loop
  while(1)
  {
    // Check if button status changed
    if (samUpChanged == 1)
    {
      samUpChanged = 0;
      digitalWritePortB(motorDown[5], LOW);
      digitalWritePortB(motorUp[5], HIGH);
      printf("Sam up\n"); fflush(stdout);
      int timout = 0;
      while((timout < BIG) && ((digitalReadPortA(buttonUp[5]) == 1) || (samForceUp == 1)))
      {
        delay(1);
        timout++;
      }
      // Motor off
      digitalWritePortB(motorUp[5], LOW);
      samForceUp = 0;
    }
    // Check if button status changed
    if (samDownChanged == 1)
    {
      samDownChanged = 0;
      digitalWritePortB(motorUp[5], LOW);
      digitalWritePortB(motorDown[5], HIGH);
      printf("Sam down\n"); fflush(stdout);
      int timout = 0;
      while((timout < BIG) && ((digitalReadPortA(buttonDown[5]) == 1) || (samForceDown == 1)))
      {
        delay(1);
        timout++;
      }
      // Motor off
      digitalWritePortB(motorDown[5], LOW);
      samForceDown = 0;
    }
    delay(100);
  }
}

OLI_THREAD(cuisine)
{
  oliHiPri(70);
  printf("Shutter thread Cuisine starts...\n"); fflush(stdout);
  // Loop
  while(1)
  {
    // Check if button status changed
    if (cuisineUpChanged == 1)
    {
      cuisineUpChanged = 0;
      digitalWritePortB(motorDown[6], LOW);
      digitalWritePortB(motorUp[6], HIGH);
      printf("Cuisine up\n"); fflush(stdout);
      int timout = 0;
      while((timout < BIG) && ((digitalReadPortA(buttonUp[6]) == 1) || (cuisineForceUp == 1)))
      {
        delay(1);
        timout++;
      }
      // Motor off
      digitalWritePortB(motorUp[6], LOW);
      cuisineForceUp = 0;
    }
    // Check if button status changed
    if (cuisineDownChanged == 1)
    {
      cuisineDownChanged = 0;
      digitalWritePortB(motorUp[6], LOW);
      digitalWritePortB(motorDown[6], HIGH);
      printf("Cuisine down\n"); fflush(stdout);
      int timout = 0;
      while((timout < BIG) && ((digitalReadPortA(buttonDown[6]) == 1) || (cuisineForceDown == 1)))
      {
        delay(1);
        timout++;
      }
      // Motor off
      digitalWritePortB(motorDown[6], LOW);
      cuisineForceDown = 0;
    }
    delay(100);
  }
}

OLI_THREAD(cellier)
{
  oliHiPri(80);
  printf("Shutter thread Cellier starts...\n"); fflush(stdout);
  // Loop
  while(1)
  {
    // Check if button status changed
    if (cellierUpChanged == 1)
    {
      cellierUpChanged = 0;
      digitalWritePortB(motorDown[7], LOW);
      digitalWritePortB(motorUp[7], HIGH);
      printf("Cellier up\n"); fflush(stdout);
      int timout = 0;
      while((timout < SMALL) && ((digitalReadPortA(buttonUp[7]) == 1) || (cellierForceUp == 1)))
      {
        delay(1);
        timout++;
      }
      // Motor off
      digitalWritePortB(motorUp[7], LOW);
      cellierForceUp = 0;
    }
    // Check if button status changed
    if (cellierDownChanged == 1)
    {
      cellierDownChanged = 0;
      digitalWritePortB(motorUp[7], LOW);
      digitalWritePortB(motorDown[7], HIGH);
      printf("Cellier down\n"); fflush(stdout);
      int timout = 0;
      while((timout < SMALL) && ((digitalReadPortA(buttonDown[7]) == 1) || (cellierForceDown == 1)))
      {
        delay(1);
        timout++;
      }
      // Motor off
      digitalWritePortB(motorDown[7], LOW);
      cellierForceDown = 0;
    }
     delay(100);
  }
}

OLI_THREAD(gen)
{
  oliHiPri(90);
  printf("Shutter thread Gen starts...\n"); fflush(stdout);
  // Loop
  while(1)
  {
    // Check if button status changed
    if (genUpChanged == 1)
    {
      genUpChanged = 0;
      digitalWritePortB(motorDown[0], LOW);
      digitalWritePortB(motorDown[1], LOW);
      digitalWritePortB(motorDown[2], LOW);
      digitalWritePortB(motorDown[3], LOW);
      digitalWritePortB(motorDown[4], LOW);
      digitalWritePortB(motorDown[5], LOW);
      digitalWritePortB(motorDown[6], LOW);
      digitalWritePortB(motorDown[7], LOW);
      digitalWritePortB(motorUp[0], HIGH);
      digitalWritePortB(motorUp[1], HIGH);
      digitalWritePortB(motorUp[2], HIGH);
      digitalWritePortB(motorUp[3], HIGH);
      digitalWritePortB(motorUp[4], HIGH);
      digitalWritePortB(motorUp[5], HIGH);
      digitalWritePortB(motorUp[6], HIGH);
      digitalWritePortB(motorUp[7], HIGH);
      printf("Gen up\n"); fflush(stdout);
      int timout = 0;
      while((timout < BIG) && ((digitalReadPortGeneral(buttonCentralizedUp) == 1) || (genForceUp == 1)))
      {
        delay(1);
        timout++;
      }
      // Motor off
      digitalWritePortB(motorUp[0], LOW);
      digitalWritePortB(motorUp[1], LOW);
      digitalWritePortB(motorUp[2], LOW);
      digitalWritePortB(motorUp[3], LOW);
      digitalWritePortB(motorUp[4], LOW);
      digitalWritePortB(motorUp[5], LOW);
      digitalWritePortB(motorUp[6], LOW);
      digitalWritePortB(motorUp[7], LOW);
      genForceUp = 0;
    }
    // Check if button status changed
    if (genDownChanged == 1)
    {
      genDownChanged = 0;
      digitalWritePortB(motorUp[0], LOW);
      digitalWritePortB(motorUp[1], LOW);
      digitalWritePortB(motorUp[2], LOW);
      digitalWritePortB(motorUp[3], LOW);
      digitalWritePortB(motorUp[4], LOW);
      digitalWritePortB(motorUp[5], LOW);
      digitalWritePortB(motorUp[6], LOW);
      digitalWritePortB(motorUp[7], LOW);
      digitalWritePortB(motorDown[0], HIGH);
      digitalWritePortB(motorDown[1], HIGH);
      digitalWritePortB(motorDown[2], HIGH);
      digitalWritePortB(motorDown[3], HIGH);
      digitalWritePortB(motorDown[4], HIGH);
      digitalWritePortB(motorDown[5], HIGH);
      digitalWritePortB(motorDown[6], HIGH);
      digitalWritePortB(motorDown[7], HIGH);
      printf("Gen down\n"); fflush(stdout);
      int timout = 0;
      while((timout < BIG) && ((digitalReadPortGeneral(buttonCentralizedDown) == 1) || (genForceDown == 1)))
      {
        delay(1);
        timout++;
      }
      // Motor off
      digitalWritePortB(motorDown[0], LOW);
      digitalWritePortB(motorDown[1], LOW);
      digitalWritePortB(motorDown[2], LOW);
      digitalWritePortB(motorDown[3], LOW);
      digitalWritePortB(motorDown[4], LOW);
      digitalWritePortB(motorDown[5], LOW);
      digitalWritePortB(motorDown[6], LOW);
      digitalWritePortB(motorDown[7], LOW);
      genForceDown = 0;
    }
     delay(100);
  }
}

// Read initial button state
void setup()
{
  salonUp = digitalReadPortA(buttonUp[0]);
  salonDown = digitalReadPortA(buttonDown[0]);
  sdbUp = digitalReadPortA(buttonUp[1]);
  sdbDown = digitalReadPortA(buttonDown[1]);
  erineUp = digitalReadPortA(buttonUp[2]);
  erineDown = digitalReadPortA(buttonDown[2]);
  evaUp = digitalReadPortA(buttonUp[3]);
  evaDown = digitalReadPortA(buttonDown[3]);
  lauraUp = digitalReadPortA(buttonUp[4]);
  lauraDown = digitalReadPortA(buttonDown[4]);
  samUp = digitalReadPortA(buttonUp[5]);
  samDown = digitalReadPortA(buttonDown[5]);
  cuisineUp = digitalReadPortA(buttonUp[6]);
  cuisineDown = digitalReadPortA(buttonDown[6]);
  cellierUp = digitalReadPortA(buttonUp[7]);
  cellierDown = digitalReadPortA(buttonDown[7]);
  genUp = digitalReadPortGeneral(buttonCentralizedUp);
  genDown = digitalReadPortGeneral(buttonCentralizedDown);
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
  connection = mysql_real_connect(&mysql,"localhost", "gammu", "gammu", "SMS", 0, 0, 0);
  // check for a connection error
  if (connection == NULL)
  {
    printf(mysql_error(&mysql));
  }
  sendSMSDB(DENIS, message);
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
  char *cmd;
  char *status;
  //LCDPrintString(1, 0, "");
  //LCDPrintString(1, 0, strdup(number));
  // Compare number to be sure this is an allowed message
  if (strcmp(number, DENIS) == 0)
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
      // Get status of motion detector
      if (digitalReadPortC(0) == 1) status = concat(2, status, "(A=1)");
      else status = concat(2, status, "(A=0)");
      // Get status of motion detector
      if (digitalReadPortC(1) == 1) status = concat(2, status, "(B=1)");
      else status = concat(2, status, "(B=0)");

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
      case 'A' :
        if (val == '0') {salonForceUp = 0; salonForceDown = 1; salonDownChanged = 1;}
        if (val == '1') {salonForceDown = 0; salonForceUp = 1; salonUpChanged = 1;}
        break;
      case 'B' :
        if (val == '0') {sdbForceUp = 0; sdbForceDown = 1; sdbDownChanged = 1;}
        if (val == '1') {sdbForceDown = 0; sdbForceUp = 1; sdbUpChanged = 1;}
        break;
      case 'C' :
        if (val == '0') {erineForceUp = 0; erineForceDown = 1; erineDownChanged = 1;}
        if (val == '1') {erineForceDown = 0; erineForceUp = 1; erineUpChanged = 1;}
        break;
      case 'D' :
        if (val == '0') {evaForceUp = 0; evaForceDown = 1; evaDownChanged = 1;}
        if (val == '1') {evaForceDown = 0; evaForceUp = 1; evaUpChanged = 1;}
        break;
      case 'E' :
        if (val == '0') {lauraForceUp = 0; lauraForceDown = 1; lauraDownChanged = 1;}
        if (val == '1') {lauraForceDown = 0; lauraForceUp = 1; lauraUpChanged = 1;}
       break;
      case 'F' :
        if (val == '0') {samForceUp = 0; samForceDown = 1; samDownChanged = 1;}
        if (val == '1') {samForceDown = 0; samForceUp = 1; samUpChanged = 1;}
        break;
      case 'G' :
        if (val == '0') {cuisineForceUp = 0; cuisineForceDown = 1; cuisineDownChanged = 1;}
        if (val == '1') {cuisineForceDown = 0; cuisineForceUp = 1; cuisineUpChanged = 1;}
        break;
      case 'H' :
        if (val == '0') {cellierForceUp = 0; cellierForceDown = 1; cellierDownChanged = 1;}
        if (val == '1') {cellierForceDown = 0; cellierForceUp = 1; cellierUpChanged = 1;}
        break;
      case 'I' :
        if (val == '0') {genForceUp = 0; genForceDown = 1; genDownChanged = 1;}
        if (val == '1') {genForceDown = 0; genForceUp = 1; genUpChanged = 1;}
        break;
      case 'J' :
        if (val == '0') {alarmActive = 0; digitalWrite(pinOliDisplay(7), LOW); printf("Alarm not active\n");}
        if (val == '1') {alarmActive = 1; digitalWrite(pinOliDisplay(7), HIGH); printf("Alarm active\n");}
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

int dbOpen( MYSQL * pmysql)
{
  MYSQL *mysql_connection;

  // Open MySQL Database and read timestamp of the last record written
  if(!mysql_init(pmysql))
  {
    log_syslog(stderr, "Cannot initialize MySQL");
  }
  else
  {
    // connect to database
    mysql_connection = mysql_real_connect(pmysql, "192.168.1.50", "morind79", "DMO12lip6", "Domotic", 3306, NULL, 0);

    if(mysql_connection == NULL)
    {
      printf("Cannot connect to database : %d: %s \n", mysql_errno(pmysql), mysql_error(pmysql));
      log_syslog(stderr, "%d: %s \n", mysql_errno(pmysql), mysql_error(pmysql));
      return(EXIT_FAILURE);
    }
  }

  return (EXIT_SUCCESS);
}

void dbClose( MYSQL * pmysql)
{
  mysql_close(pmysql);
}

void queryWaterMeter()
{
  static char mysql_field[ 512];
  static char mysql_value[ 512];
  static char mysql_job[1024];
  char buf[10];
  MYSQL mysql;

  strcpy(mysql_field, "DATE");
  strcpy(mysql_value, "NOW()");

  strcat(mysql_field, ",LITER");

  sprintf(buf, "%d", water);

  strcat(mysql_value, ",'");
  strcat(mysql_value, buf);
  strcat(mysql_value, "'");

  sprintf(mysql_job, "INSERT INTO DbiWaterMeter\n  (%s)\nVALUES\n  (%s);\n", mysql_field, mysql_value);

  fprintf(stdout, "%s", mysql_job);

  if ( dbOpen(&mysql) != EXIT_SUCCESS )
    log_syslog(stderr, "%d: %s \n", mysql_errno(&mysql), mysql_error(&mysql));
  else
  {
    // execute SQL query : INSERT INTO `DbiWaterMeter`(`id`, `DATE`, `LITER`) VALUES ([value-1],[value-2],[value-3])
    if (mysql_query(&mysql, mysql_job))
    {
      log_syslog(stderr, "%d: %s \n", mysql_errno(&mysql), mysql_error(&mysql));
    }

    db_close(&mysql);
  }

  water = 0;
  // close the connection
  dbClose(&mysql);
}

void queryEnergyMeter()
{
  processEnergyMeter(1);  // Maison
  processEnergyMeter(2);  // Chauffe Eau
  processEnergyMeter(3);  // Photovoltaique
  processEnergyMeter(8);  // Auto-conso
  processEnergyMeter(5);  // Non-conso
  processEnergyMeter(6);  // PAC
}

void processEnergyMeter(int channel)
{
  if (channel == 1) // Maison ADCO=700622010525
  {
    mcp23008DigitalWrite(fdMCPEnergy, 1, LOW); // Dir2 = 0 (S2)
    mcp23008DigitalWrite(fdMCPEnergy, 2, LOW); // Dir3 = 0 (S1)
    mcp23008DigitalWrite(fdMCPEnergy, 3, LOW); // Dir4 = 0 (S0)
    log_syslog(stderr, "Channel 1 >>>>>  MAISON  <<<<<\n");
  }
  else if (channel == 2) // Chauffe-eau ADCO=050622013932
  {
    mcp23008DigitalWrite(fdMCPEnergy, 1, LOW);
    mcp23008DigitalWrite(fdMCPEnergy, 2, LOW);
    mcp23008DigitalWrite(fdMCPEnergy, 3, HIGH);
    log_syslog(stderr, "Channel 2 >>>>>  Chauffe-eau  <<<<<<\n");
  }
  else if (channel == 3) // Photovoltaique ADCO=021028809892
  {
    mcp23008DigitalWrite(fdMCPEnergy, 1, LOW);
    mcp23008DigitalWrite(fdMCPEnergy, 2, HIGH);
    mcp23008DigitalWrite(fdMCPEnergy, 3, LOW);
    log_syslog(stderr, "Channel 3 >>>>>  Photovoltaique  <<<<<\n");
  }
  else if (channel == 4) // Does not work at all !!!
  {
    mcp23008DigitalWrite(fdMCPEnergy, 1, LOW);
    mcp23008DigitalWrite(fdMCPEnergy, 2, HIGH);
    mcp23008DigitalWrite(fdMCPEnergy, 3, HIGH);
    log_syslog(stderr, "Channel 4\n");
  }
  else if (channel == 5) // Non-conso ADCO=021028809922
  {
    mcp23008DigitalWrite(fdMCPEnergy, 1, HIGH);
    mcp23008DigitalWrite(fdMCPEnergy, 2, LOW);
    mcp23008DigitalWrite(fdMCPEnergy, 3, LOW);
    log_syslog(stderr, "Channel 5 >>>>>  Non-conso  <<<<<\n");
  }
  else if (channel == 6) // PAC ADCO=050522019453
  {
    mcp23008DigitalWrite(fdMCPEnergy, 1, HIGH);
    mcp23008DigitalWrite(fdMCPEnergy, 2, LOW);
    mcp23008DigitalWrite(fdMCPEnergy, 3, HIGH);
    log_syslog(stderr, "Channel 6 >>>>>  PAC  <<<<<\n");
  }
  else if (channel == 7)
  {
    mcp23008DigitalWrite(fdMCPEnergy, 1, HIGH);
    mcp23008DigitalWrite(fdMCPEnergy, 2, HIGH);
    mcp23008DigitalWrite(fdMCPEnergy, 3, LOW);
    log_syslog(stderr, "Channel 7\n");
  }
  else if (channel == 8) // Auto-conso ADCO=039801178663
  {
    mcp23008DigitalWrite(fdMCPEnergy, 1, HIGH);
    mcp23008DigitalWrite(fdMCPEnergy, 2, HIGH);
    mcp23008DigitalWrite(fdMCPEnergy, 3, HIGH);
    log_syslog(stderr, "Channel 8 >>>>>  Auto-consommation  <<<<<\n");
  }
  // Enable 74LS151
  mcp23008DigitalWrite(fdMCPEnergy, 0, LOW);
  log_syslog(stderr, "Channel enable\n");
  delay(1000);
  // Read Energy meter
  readEnergyMeter();
  delay(1000);
  // Disable 74LS151
  mcp23008DigitalWrite(fdMCPEnergy, 0, HIGH);
  delay(1000);
  log_syslog(stderr, "Channel disable\n");
}

int main(void)
{
  char ch;
  int i;
  int fd;
  MYSQL mysql;
  tm = *localtime(&t);
  // Connect to the MySQL database at localhost
  mysql_init(&mysql);
  connection = mysql_real_connect(&mysql,"localhost", "gammu", "gammu", "SMS", 0, 0, 0);
  // Check for a connection error
  if (connection == NULL)
  {
    printf(mysql_error(&mysql));
    return 1;
  }
  printf("\n");
  printDate();
  // Start with a clean database
  clearDB();
  // close the connection
  mysql_close(connection);
  // Setup GPIO
  wiringOliSetup();
  oliExtSetup();
  // I2C communication
  fd = wiringOliI2CSetup(0, 0x02);
  if (fd == -1)
  {
    printf("Can't setup the Arduino I2C slave device\n");
  }

  // MCP23008 energy meter
  fdMCPEnergy = mcp23008Init(0x24);
  if (fdMCPEnergy == -1)
  {
    printf("Can't setup the energy board I2C device\n");
  }
  mcp23008PinMode(fdMCPEnergy, 0, OUTPUT);
  mcp23008PinMode(fdMCPEnergy, 1, OUTPUT);
  mcp23008PinMode(fdMCPEnergy, 2, OUTPUT);
  mcp23008PinMode(fdMCPEnergy, 3, OUTPUT);
  mcp23008PinMode(fdMCPEnergy, 4, OUTPUT);
  mcp23008PinMode(fdMCPEnergy, 5, OUTPUT);
  mcp23008PinMode(fdMCPEnergy, 6, OUTPUT);
  mcp23008PinMode(fdMCPEnergy, 7, OUTPUT);

  mcp23008DigitalWrite(fdMCPEnergy, 0, HIGH);  // Disable 74LS151
  mcp23008DigitalWrite(fdMCPEnergy, 1, LOW);
  mcp23008DigitalWrite(fdMCPEnergy, 2, LOW);
  mcp23008DigitalWrite(fdMCPEnergy, 3, LOW);
  mcp23008DigitalWrite(fdMCPEnergy, 4, HIGH);
  mcp23008DigitalWrite(fdMCPEnergy, 5, HIGH);
  mcp23008DigitalWrite(fdMCPEnergy, 6, HIGH);
  mcp23008DigitalWrite(fdMCPEnergy, 7, HIGH);

  // Port A as input
  for (i=0; i<16; i++)
  {
    pinModePortA(i, INPUT);
    pullUpDnCtrlPortA(i, 2);
  }
  // Port B as output
  for (i=0; i<16; i++)
  {
    pinModePortB(i, OUTPUT);
    digitalWritePortB(i, LOW);
  }
  // Port C as input
  for (i=0; i<8; i++)
  {
    pinModePortC(i, INPUT);
    pullUpDnCtrlPortC(i, 2);
  }
  // Port D as input
  for (i=0; i<8; i++)
  {
    pinModePortD(i, INPUT);
    pullUpDnCtrlPortD(i, 2);
  }
  // Port General as input
  pinModePortGeneral(buttonCentralizedUp, INPUT);
  pullUpDnCtrlPortGeneral(buttonCentralizedUp, 2);
  pinModePortGeneral(buttonCentralizedDown, INPUT);
  pullUpDnCtrlPortGeneral(buttonCentralizedDown, 2);
  // We are ready, tell this to Arduino send 1 byte 'S' ASCII=0x53
  wiringOliI2CWriteReg8(fd, 0x00, 0x53);
  // Init of timers
  prevAlarm1 = millis();
  prevAlarm2 = millis();
  // Start
  printf("Start loop\n");
  // Read initial state
  setup();

  // Start thread for RI pin
  int xP = oliThreadCreate(ri);
  if (xP != 0) printf ("RI didn't start\n");

  int x0 = oliThreadCreate(alarm);
  if (x0 != 0) printf("Alarm didn't start\n");

  int x1 = oliThreadCreate(alarmOff);
  if (x1 != 0) printf("AlarmOff didn't start\n");

  int x2 = oliThreadCreate(buttons);
  if (x2 != 0) printf ("Buttons didn't start\n");

  int x3 = oliThreadCreate(salon);
  if (x3 != 0) printf ("Salon didn't start\n");

  int x4 = oliThreadCreate(sdb);
  if (x4 != 0) printf ("Sdb didn't start\n");

  int x5 = oliThreadCreate(erine);
  if (x5 != 0) printf ("Erine didn't start\n");

  int x6 = oliThreadCreate(eva);
  if (x6 != 0) printf ("Eva didn't start\n");

  int x7 = oliThreadCreate(laura);
  if (x7 != 0) printf ("Laura didn't start\n");

  int x8 = oliThreadCreate(sam);
  if (x8 != 0) printf ("Sam didn't start\n");

  int x9 = oliThreadCreate(cuisine);
  if (x9 != 0) printf ("Cuisine didn't start\n");

  int xA = oliThreadCreate(cellier);
  if (xA != 0) printf ("Cellier didn't start\n");

  int xB = oliThreadCreate(gen);
  if (xB != 0) printf ("Gen didn't start\n");

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
    else if (ch=='a') {salonForceUp = 0; salonForceDown = 1; salonDownChanged = 1;}
    else if (ch=='b') {salonForceDown = 0; salonForceUp = 1; salonUpChanged = 1;}
    else if (ch=='c') {sdbForceUp = 0; sdbForceDown = 1; sdbDownChanged = 1;}
    else if (ch=='d') {sdbForceDown = 0; sdbForceUp = 1; sdbUpChanged = 1;}
    else if (ch=='e') {erineForceUp = 0; erineForceDown = 1; erineDownChanged = 1;}
    else if (ch=='f') {erineForceDown = 0; erineForceUp = 1; erineUpChanged = 1;}
    else if (ch=='g') {evaForceUp = 0; evaForceDown = 1; evaDownChanged = 1;}
    else if (ch=='h') {evaForceDown = 0; evaForceUp = 1; evaUpChanged = 1;}
    else if (ch=='i') {lauraForceUp = 0; lauraForceDown = 1; lauraDownChanged = 1;}
    else if (ch=='j') {lauraForceDown = 0; lauraForceUp = 1; lauraUpChanged = 1;}
    else if (ch=='k') {samForceUp = 0; samForceDown = 1; samDownChanged = 1;}
    else if (ch=='l') {samForceDown = 0; samForceUp = 1; samUpChanged = 1;}
    else if (ch=='m') {cuisineForceUp = 0; cuisineForceDown = 1; cuisineDownChanged = 1;}
    else if (ch=='n') {cuisineForceDown = 0; cuisineForceUp = 1; cuisineUpChanged = 1;}
    else if (ch=='o') {cellierForceUp = 0; cellierForceDown = 1; cellierDownChanged = 1;}
    else if (ch=='p') {cellierForceDown = 0; cellierForceUp = 1; cellierUpChanged = 1;}
    else if (ch=='q') {genForceUp = 0; genForceDown = 1; genDownChanged = 1;}
    else if (ch=='r') {genForceDown = 0; genForceUp = 1; genUpChanged = 1;}
    else if (ch=='t') {queryEnergyMeter();}
    else if (ch=='w') {queryWaterMeter();}
    else if (ch=='1') {processEnergyMeter(1);}
    else if (ch=='2') {processEnergyMeter(2);}
    else if (ch=='3') {processEnergyMeter(3);}
    else if (ch=='4') {processEnergyMeter(4);}
    else if (ch=='5') {processEnergyMeter(5);}
    else if (ch=='6') {processEnergyMeter(6);}
    else if (ch=='7') {processEnergyMeter(7);}
    else if (ch=='8') {processEnergyMeter(8);}
    delay(100);
  }
  return(0);
}
