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
 *  C0 General up     C1 General down
 *
 *  run : gpio load i2c
 *
 *  gcc -Wall -o shutter shutter4.c -lwiringOli $(mysql_config --cflags) $(mysql_config --libs) -lncurses
 *  ./shutter
 *  
 * Screen 90x30 characters  
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

MYSQL *connection;
time_t t;
struct tm tm;
int now_sec, now_min, now_hour, now_day, now_wday, now_month, now_year;
time_t now;
struct tm *now_tm;

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
void printDate(int x, int y, int color);
void maketime();
void printString(int x, int y, int color, char *msg);

int alarmStatus = 0;          // When 0 set alarm off
unsigned long prevAlarm1;     // Variable used for knowing time since last alarm1
unsigned long prevAlarm2;     // Variable used for knowing time since last alarm2
int powerFail = 0;            // 0 -> No power, 1 -> Power ok
long timeBetweenSMS = 50000;  // Waiting time before sending another SMS in ms
int alarmActive = 0;          // 0 -> Alarm not active, 1 -> Alarme active
const char *DENIS = "+33626281284";

// Timing for shutters
const int SMALL = 15000;
const int MEDIUM = 20000;
const int BIG = 35000;

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
  printString(3, 0, 3, "RI thread starts...");
  //printf("RI thread starts...\n"); fflush(stdout);
  // Loop
  while(1)
  {
    // Check if RI status changed
    if (digitalReadSIM900_RI() == 0)
    {
      oliLock(0);
      printString(3, 30, 3, "RI detected...");
      printString(3, 60, 3, "                             "); // Erase MySQL errors
      //printf("RI detected...\n");
      // connect to the MySQL database at localhost
      mysql_init(&mysql);
      connection = mysql_real_connect(&mysql,"localhost", "gammu", "gammu", "SMS", 0, 0, 0);
      // check for a connection error
      if (connection == NULL)
      {
        //printf(mysql_error(&mysql));
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
      printString(3, 30, 3, "              ");
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
      connection = mysql_real_connect(&mysql,"localhost", "gammu", "gammu", "SMS", 0, 0, 0);
      // check for a connection error
      if (connection == NULL)
      {
        attron(COLOR_PAIR(3));
        mvprintw(3, 60, "%s", mysql_error(&mysql));
        refresh();
        //printf(mysql_error(&mysql));
      }
      clearDB();
      clearForToday = 1;
      attron(COLOR_PAIR(3));
      mvprintw(2, 0, "Clean DB at : %02d:%02d", tm.tm_hour, tm.tm_min);
      refresh();
      //printf("Clean DB at : %02d:%02d\n", tm.tm_hour, tm.tm_min);
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

OLI_THREAD(alarm)
{
  oliHiPri(1);
  while(1)
  {
    // Port C0 = motion detector service door
    if ((digitalReadPortC(0) == 0) && (alarmActive == 1))
    {
      if (millis() - prevAlarm1 >= timeBetweenSMS)
      {
        printString(6, 0, 3, "Detecteur porte service "); printDate(6, 25, 5);
        //printf("Detecteur porte service ");
        alarmStatus = 1;
        mcp23008DigitalWrite(0, HIGH);
        prevAlarm1 = millis();
      }
    }
    // Port C1 = motion detector cars
    if ((digitalReadPortC(1) == 0) && (alarmActive == 1))
    {
      if (millis() - prevAlarm2 >= timeBetweenSMS)
      {
        printString(7, 0, 3, "Detecteur voiture "); printDate(7, 25, 5);
        //printf("Detecteur voiture ");
        alarmStatus = 1;
        mcp23008DigitalWrite(1, HIGH);
        prevAlarm2 = millis();
      }
    }
    //printf("%d %d %d %d %d %d %d %d\n", digitalReadPortC(0), digitalReadPortC(1), digitalReadPortC(2), 
    //                                    digitalReadPortC(3), digitalReadPortC(4), digitalReadPortC(5),
    //                                    digitalReadPortC(6), digitalReadPortC(7));
    
    // Display clock
    maketime();
    attron(COLOR_PAIR(2));
    mvprintw(0, 80, "%02d:%02d:%02d", now_hour, now_min, now_sec);
    refresh();
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
      mcp23008DigitalWrite(0, LOW);
      mcp23008DigitalWrite(1, LOW);
      printString(6, 0, 3, "                        "); // Erase motion detector errors
      printString(7, 0, 3, "                  ");
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
    // Salon
    if (salonUp != digitalReadPortA(buttonUp[0]))
    {
      salonUp = digitalReadPortA(buttonUp[0]);
      salonForceUp = 0;
      salonForceDown = 0;
      if (salonUp != 0) {salonUpChanged = 1; printString(9, 0, 3, "Salon button up = 1 "); printDate(9, 25, 5);}
      else {salonUpChanged = 0;              printString(9, 0, 3, "Salon button up = 0 "); printDate(9, 25, 5);}
    }
    if (salonDown != digitalReadPortA(buttonDown[0]))
    {
      salonDown = digitalReadPortA(buttonDown[0]);
      salonForceUp = 0;
      salonForceDown = 0;
      if (salonDown != 0) {salonDownChanged = 1; printString(9, 40, 3, "Salon button down = 1 "); printDate(9, 65, 5);}
      else {salonDownChanged = 0;                printString(9, 40, 3, "Salon button down = 0 "); printDate(9, 65, 5);}
    }
    // Sdb
    if (sdbUp != digitalReadPortA(buttonUp[1]))
    {
      sdbUp = digitalReadPortA(buttonUp[1]);
      sdbForceUp = 0;
      sdbForceDown = 0;
      if (sdbUp != 0) { sdbUpChanged = 1; printString(10, 0, 3, "Sdb button up = 1 "); printDate(10, 25, 5);}
      else { sdbUpChanged = 0;            printString(10, 0, 3, "Sdb button up = 0 "); printDate(10, 25, 5);}
    }
    if (sdbDown != digitalReadPortA(buttonDown[1]))
    {
      sdbDown = digitalReadPortA(buttonDown[1]);
      sdbForceUp = 0;
      sdbForceDown = 0;
      if (sdbDown != 0) {sdbDownChanged = 1;  printString(10, 40, 3, "Sdb button down = 1 "); printDate(10, 65, 5);}
      else {sdbDownChanged = 0;               printString(10, 40, 3, "Sdb button down = 0 "); printDate(10, 65, 5);}
    }
    // Erine
    if (erineUp != digitalReadPortA(buttonUp[2]))
    {
      erineUp = digitalReadPortA(buttonUp[2]);
      erineForceUp = 0;
      erineForceDown = 0;
      if (erineUp != 0) {erineUpChanged = 1;  printString(11, 0, 3, "Erine button up = 1 "); printDate(11, 25, 5);}
      else {erineUpChanged = 0;               printString(11, 0, 3, "Erine button up = 0 "); printDate(11, 25, 5);}
    }
    if (erineDown != digitalReadPortA(buttonDown[2]))
    {
      erineDown = digitalReadPortA(buttonDown[2]);
      erineForceUp = 0;
      erineForceDown = 0;
      if (erineDown != 0) {erineDownChanged = 1;  printString(11, 40, 3, "Erine button down = 1 "); printDate(11, 65, 5);}
      else {erineDownChanged = 0;                 printString(11, 40, 3, "Erine button down = 0 "); printDate(11, 65, 5);}
    }
    // Eva
    if (evaUp != digitalReadPortA(buttonUp[3]))
    {
      evaUp = digitalReadPortA(buttonUp[3]);
      evaForceUp = 0;
      evaForceDown = 0;
      if (evaUp != 0) {evaUpChanged = 1;  printString(12, 0, 3, "Eva button up = 1 "); printDate(12, 25, 5);}
      else {evaUpChanged = 0;             printString(12, 0, 3, "Eva button up = 0 "); printDate(12, 25, 5);}
    }
    if (evaDown != digitalReadPortA(buttonDown[3]))
    {
      evaDown = digitalReadPortA(buttonDown[3]);
      evaForceUp = 0;
      evaForceDown = 0;
      if (evaDown != 0) {evaDownChanged = 1;  printString(12, 40, 3, "Eva button down = 1 "); printDate(12, 65, 5);}
      else {evaDownChanged = 0;               printString(12, 40, 3, "Eva button down = 0 "); printDate(12, 65, 5);}
    }
    // Laura
    if (lauraUp != digitalReadPortA(buttonUp[4]))
    {
      lauraUp = digitalReadPortA(buttonUp[4]);
      lauraForceUp = 0;
      lauraForceDown = 0;
      if (lauraUp != 0) {lauraUpChanged = 1;  printString(13, 0, 3, "Laura button up = 1 "); printDate(13, 25, 5);}
      else {lauraUpChanged = 0;               printString(13, 0, 3, "Laura button up = 0 "); printDate(13, 25, 5);}
    }
    if (lauraDown != digitalReadPortA(buttonDown[4]))
    {
      lauraDown = digitalReadPortA(buttonDown[4]);
      lauraForceUp = 0;
      lauraForceDown = 0;
      if (lauraDown != 0) {lauraDownChanged = 1;  printString(13, 40, 3, "Laura button down = 1 "); printDate(13, 65, 5);}
      else {lauraDownChanged = 0;                 printString(13, 40, 3, "Laura button down = 0 "); printDate(13, 65, 5);}
    }
    // Sam
    if (samUp != digitalReadPortA(buttonUp[5]))
    {
      samUp = digitalReadPortA(buttonUp[5]);
      samForceUp = 0;
      samForceDown = 0;
      if (samUp != 0) {samUpChanged = 1;  printString(14, 0, 3, "Sam button up = 1 "); printDate(14, 25, 5);}
      else {samUpChanged = 0;             printString(14, 0, 3, "Sam button up = 0 "); printDate(14, 25, 5);}
    }
    if (samDown != digitalReadPortA(buttonDown[5]))
    {
      samDown = digitalReadPortA(buttonDown[5]);
      samForceUp = 0;
      samForceDown = 0;
      if (samDown != 0) {samDownChanged = 1;  printString(14, 40, 3, "Sam button down = 1 "); printDate(14, 65, 5);}
      else {samDownChanged = 0;               printString(14, 40, 3, "Sam button down = 0 "); printDate(14, 65, 5);}
    }
    // Cuisine
    if (cuisineUp != digitalReadPortA(buttonUp[6]))
    {
      cuisineUp = digitalReadPortA(buttonUp[6]);
      cuisineForceUp = 0;
      cuisineForceDown = 0;
      if (cuisineUp != 0) {cuisineUpChanged = 1;  printString(15, 0, 3, "Cuisine button up = 1 "); printDate(15, 25, 5);}
      else {cuisineUpChanged = 0;                 printString(15, 0, 3, "Cuisine  button up = 0 "); printDate(15, 25, 5);}
    }
    if (cuisineDown != digitalReadPortA(buttonDown[6]))
    {
      cuisineDown = digitalReadPortA(buttonDown[6]);
      cuisineForceUp = 0;
      cuisineForceDown = 0;
      if (cuisineDown != 0) {cuisineDownChanged = 1;  printString(15, 40, 3, "Cuisine button down = 1 "); printDate(15, 65, 5);}
      else {cuisineDownChanged = 0;                   printString(15, 40, 3, "Cuisine button down = 0 "); printDate(15, 65, 5);}
    }
    // Cellier
    if (cellierUp != digitalReadPortA(buttonUp[7]))
    {
      cellierUp = digitalReadPortA(buttonUp[7]);
      cellierForceUp = 0;
      cellierForceDown = 0;
      if (cellierUp != 0) {cellierUpChanged = 1;  printString(16, 0, 3, "Cellier button up = 1 "); printDate(16, 25, 5);}
      else {cellierUpChanged = 0;                 printString(16, 0, 3, "Cellier button up = 0 "); printDate(16, 25, 5);}
    }
    if (cellierDown != digitalReadPortA(buttonDown[7]))
    {
      cellierDown = digitalReadPortA(buttonDown[7]);
      cellierForceUp = 0;
      cellierForceDown = 0;
      if (cellierDown != 0) {cellierDownChanged = 1;  printString(16, 40, 3, "Cellier button down = 1 "); printDate(16, 65, 5);}
      else {cellierDownChanged = 0;                   printString(16, 40, 3, "Cellier button down = 0 "); printDate(16, 65, 5);}
    }
    // Gen
    if (genUp != digitalReadPortGeneral(buttonCentralizedUp))
    {
      genUp = digitalReadPortGeneral(buttonCentralizedUp);
      genForceUp = 0;
      genForceDown = 0;
      if (genUp != 0) {genUpChanged = 1;  printString(17, 0, 3, "Gen button up = 1 "); printDate(17, 25, 5);}
      else {genUpChanged = 0;             printString(17, 0, 3, "Gen button up = 0 "); printDate(17, 25, 5);}
    }
    if (genDown != digitalReadPortGeneral(buttonCentralizedDown))
    {
      genDown = digitalReadPortGeneral(buttonCentralizedDown);
      genForceUp = 0;
      genForceDown = 0;
      if (genDown != 0) {genDownChanged = 1;  printString(17, 0, 3, "Gen button down = 1 "); printDate(17, 65, 5);}
      else {genDownChanged = 0;               printString(17, 0, 3, "Gen button down = 0 "); printDate(17, 65, 5);}
    }
    delay(5);
  }
}

OLI_THREAD(salon)
{
  oliHiPri(10);
  //printf("Shutter thread salon starts...\n"); fflush(stdout);
  // Loop
  while(1)
  {
    // Check if button status changed
    if (salonUpChanged == 1)
    {
      salonUpChanged = 0;
      digitalWritePortB(motorDown[0], LOW);
      digitalWritePortB(motorUp[0], HIGH);
      printString(9, 75, 4, "Salon up  ");
      //printf("Salon up\n"); fflush(stdout);
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
      printString(9, 75, 4, "Salon down");
      //printf("Salon down\n"); fflush(stdout);
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
  //printf("Shutter thread sdb starts...\n"); fflush(stdout);
  // Loop
  while(1)
  {
    // Check if button status changed
    if (sdbUpChanged == 1)
    {
      sdbUpChanged = 0;
      digitalWritePortB(motorDown[1], LOW);
      digitalWritePortB(motorUp[1], HIGH);
      printString(10, 75, 4, "Sdb up  ");
      //printf("Sdb up\n"); fflush(stdout);
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
      printString(10, 75, 4, "Sdb down");
      //printf("Sdb down\n"); fflush(stdout);
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
  //printf("Shutter thread Erine starts...\n"); fflush(stdout);
  // Loop
  while(1)
  {
    // Check if button status changed
    if (erineUpChanged == 1)
    {
      erineUpChanged = 0;
      digitalWritePortB(motorDown[2], LOW);
      digitalWritePortB(motorUp[2], HIGH);
      printString(11, 75, 4, "Erine up  ");
      //printf("Erine up\n"); fflush(stdout);
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
      printString(11, 75, 4, "Erine down");
      //printf("Erine down\n"); fflush(stdout);
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
  //printf("Shutter thread Eva starts...\n"); fflush(stdout);
  // Loop
  while(1)
  {
    // Check if button status changed
    if (evaUpChanged == 1)
    {
      evaUpChanged = 0;
      digitalWritePortB(motorDown[3], LOW);
      digitalWritePortB(motorUp[3], HIGH);
      printString(12, 75, 4, "Eva up  ");
      //printf("Eva up\n"); fflush(stdout);
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
      printString(12, 75, 4, "Eva down");
      //printf("Eva down\n"); fflush(stdout);
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
  //printf("Shutter thread Laura starts...\n"); fflush(stdout);
  // Loop
  while(1)
  {
    // Check if button status changed
    if (lauraUpChanged == 1)
    {
      lauraUpChanged = 0;
      digitalWritePortB(motorDown[4], LOW);
      digitalWritePortB(motorUp[4], HIGH);
      printString(13, 75, 4, "Laura up  ");
      //printf("Laura up\n"); fflush(stdout);
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
      printString(13, 75, 4, "Laura down");
      //printf("Laura down\n"); fflush(stdout);
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
  //printf("Shutter thread Sam starts...\n"); fflush(stdout);
  // Loop
  while(1)
  {
    // Check if button status changed
    if (samUpChanged == 1)
    {
      samUpChanged = 0;
      digitalWritePortB(motorDown[5], LOW);
      digitalWritePortB(motorUp[5], HIGH);
      printString(14, 75, 4, "Sam up  ");
      //printf("Sam up\n"); fflush(stdout);
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
      printString(14, 75, 4, "Sam down");
      //printf("Sam down\n"); fflush(stdout);
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
  //printf("Shutter thread Cuisine starts...\n"); fflush(stdout);
  // Loop
  while(1)
  {
    // Check if button status changed
    if (cuisineUpChanged == 1)
    {
      cuisineUpChanged = 0;
      digitalWritePortB(motorDown[6], LOW);
      digitalWritePortB(motorUp[6], HIGH);
      printString(15, 75, 4, "Cuisine up  ");
      //printf("Cuisine up\n"); fflush(stdout);
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
      printString(15, 75, 4, "Cuisine down");
      //printf("Cuisine down\n"); fflush(stdout);
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
  //printf("Shutter thread Cellier starts...\n"); fflush(stdout);
  // Loop
  while(1)
  {
    // Check if button status changed
    if (cellierUpChanged == 1)
    {
      cellierUpChanged = 0;
      digitalWritePortB(motorDown[7], LOW);
      digitalWritePortB(motorUp[7], HIGH);
      printString(16, 75, 4, "Cellier up  ");
      //printf("Cellier up\n"); fflush(stdout);
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
      printString(16, 75, 4, "Cellier down");
      //printf("Cellier down\n"); fflush(stdout);
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
  //printf("Shutter thread Gen starts...\n"); fflush(stdout);
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
      printString(17, 75, 4, "Gen up  ");
      //printf("Gen up\n"); fflush(stdout);
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
      printString(17, 75, 4, "Gen down");
      //printf("Gen down\n"); fflush(stdout);
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
    attron(COLOR_PAIR(3));
    mvprintw(3, 60, "%s", mysql_error(connection));
    refresh();
    //printf(mysql_error(connection));
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
    attron(COLOR_PAIR(3));
    mvprintw(3, 60, "%s", mysql_error(connection));
    refresh();
    //printf(mysql_error(connection));
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
    attron(COLOR_PAIR(3));
    mvprintw(3, 60, "%s", mysql_error(connection));
    refresh();
    //printf(mysql_error(connection));
    return(0);
  }
  // must call mysql_store_result( ) before you can issue any other query calls
  result = mysql_store_result(connection);
  num = mysql_num_rows(result);
  attron(COLOR_PAIR(3));
  mvprintw(19, 0, "Rows : %d", num);
  refresh();
  //printf("Rows: %d\n", num);
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
    attron(COLOR_PAIR(3));
    mvprintw(20, 19, "Text: %s, Number: %s, index: %d", smsText, smsNumber, index);
    refresh();
    //printf("Text: %s, Number: %s, index: %d\n", smsText, smsNumber, index);
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
    attron(COLOR_PAIR(3));
    mvprintw(3, 60, "%s", mysql_error(&mysql));
    refresh();
    //printf(mysql_error(&mysql));
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
    attron(COLOR_PAIR(3));
    mvprintw(3, 60, "%s", mysql_error(connection));
    refresh();
    //printf(mysql_error(connection));
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
  //printf("start=%d end=%d\n", start, end);
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
    attron(COLOR_PAIR(3));
    mvprintw(75, 19, "numCmd = %d", numCmd);
    refresh();
    //printf("numCmd = %d\n", numCmd);
    if (numCmd > 0)
    {
      // Get all commands
      for (i=1; i<numCmd+1; i++)
      {
        cmd = GetCommand(text, i);
        attron(COLOR_PAIR(3));
        mvprintw(80, 19, "cmd = %s", cmd);
        refresh();
        //printf("cmd = %s\n", cmd);
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
      attron(COLOR_PAIR(3));
      mvprintw(20, 0, "status = %s\n", status);
      refresh();
      //printf("status=%s\n", status);
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
        if (val == '0') {alarmActive = 0; digitalWrite(pinOliDisplay(7), LOW);  printString(22, 0, 4, "Alarm not active");}
        if (val == '1') {alarmActive = 1; digitalWrite(pinOliDisplay(7), HIGH); printString(22, 0, 4, "Alarm active    ");}
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

void printString(int x, int y, int color, char *msg)
{
  attron(COLOR_PAIR(color));
  mvprintw(x, y, "%s", msg);
  refresh();
}

void maketime()
{
  // Get the current date/time
  now = time (NULL);
  now_tm = localtime (&now);
  now_sec = now_tm->tm_sec;
  now_min = now_tm->tm_min;
  now_hour = now_tm->tm_hour;
  now_day = now_tm->tm_mday;
  now_wday = now_tm->tm_wday;
  now_month = now_tm->tm_mon + 1;
  now_year = now_tm->tm_year + 1900;
}

void printDate(int x, int y, int color)
{
  maketime();
  attron(COLOR_PAIR(color));
  mvprintw(x, y, "%02d:%02d:%02d", now_hour, now_min, now_sec);
  refresh();
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
  // Start with a clean database
  clearDB();
  // close the connection
  mysql_close(connection);
  // Setup GPIO
  wiringOliSetup();
  oliExtSetup();
  // I2C communication
  fd = wiringOliI2CSetup(0, 0x02);
  // MCP23008
  mcp23008Init(0x20);
  mcp23008PinMode(0, OUTPUT);
  mcp23008PinMode(1, OUTPUT);
  mcp23008PinMode(2, OUTPUT);
  mcp23008PinMode(3, OUTPUT);
  mcp23008PinMode(4, OUTPUT);
  mcp23008PinMode(5, OUTPUT);
  mcp23008PinMode(6, OUTPUT);
  mcp23008PinMode(7, OUTPUT);
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
  //printf("Start loop\n");  
  // Read initial state
  setup();
  
  // Curses
  initscr();
  start_color(); 
  noecho();
  curs_set(FALSE);
  
  init_pair(1, COLOR_RED, COLOR_BLACK);
  init_pair(2, COLOR_GREEN, COLOR_BLACK);
  init_pair(3, COLOR_CYAN, COLOR_BLACK);
  init_pair(4, COLOR_MAGENTA, COLOR_BLACK);
  init_pair(5, COLOR_YELLOW, COLOR_BLACK);
  
  attron(COLOR_PAIR(1));
  mvprintw(0, 0, "Shutter V0.1");
  
  // Start thread for RI pin
  int xP = oliThreadCreate(ri);
  if (xP != 0) mvprintw(3, 0, "RI didn't start");

  int x0 = oliThreadCreate(alarm);
  if (x0 != 0) mvprintw(3, 0, "Alarm didn't start");
  
  int x1 = oliThreadCreate(alarmOff);
  if (x1 != 0) mvprintw(3, 0, "AlarmOff didn't start");

  int x2 = oliThreadCreate(buttons);
  if (x2 != 0) mvprintw(3, 0, "Buttons didn't start");

  int x3 = oliThreadCreate(salon);
  if (x3 != 0) mvprintw(3, 0, "Salon didn't start");

  int x4 = oliThreadCreate(sdb);
  if (x4 != 0) mvprintw(3, 0, "Sdb didn't start");

  int x5 = oliThreadCreate(erine);
  if (x5 != 0) mvprintw(3, 0, "Erine didn't start");

  int x6 = oliThreadCreate(eva);
  if (x6 != 0) mvprintw(3, 0, "Eva didn't start");

  int x7 = oliThreadCreate(laura);
  if (x7 != 0) mvprintw(3, 0, "Laura didn't start");

  int x8 = oliThreadCreate(sam);
  if (x8 != 0) mvprintw(3, 0, "Sam didn't start");

  int x9 = oliThreadCreate(cuisine);
  if (x9 != 0) mvprintw(3, 0, "Cuisine didn't start");

  int xA = oliThreadCreate(cellier);
  if (xA != 0) mvprintw(3, 0, "Cellier didn't start");
  
  int xB = oliThreadCreate(gen);
  if (xB != 0) mvprintw(3, 0, "Gen didn't start");
  refresh();
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
    else if(ch=='a') {salonForceUp = 0; salonForceDown = 1; salonDownChanged = 1;}
    else if(ch=='b') {salonForceDown = 0; salonForceUp = 1; salonUpChanged = 1;}
    else if(ch=='c') {sdbForceUp = 0; sdbForceDown = 1; sdbDownChanged = 1;}
    else if(ch=='d') {sdbForceDown = 0; sdbForceUp = 1; sdbUpChanged = 1;}
    else if(ch=='e') {erineForceUp = 0; erineForceDown = 1; erineDownChanged = 1;}
    else if(ch=='f') {erineForceDown = 0; erineForceUp = 1; erineUpChanged = 1;}
    else if(ch=='g') {evaForceUp = 0; evaForceDown = 1; evaDownChanged = 1;}
    else if(ch=='h') {evaForceDown = 0; evaForceUp = 1; evaUpChanged = 1;}
    else if(ch=='i') {lauraForceUp = 0; lauraForceDown = 1; lauraDownChanged = 1;}
    else if(ch=='j') {lauraForceDown = 0; lauraForceUp = 1; lauraUpChanged = 1;}
    else if(ch=='k') {samForceUp = 0; samForceDown = 1; samDownChanged = 1;}
    else if(ch=='l') {samForceDown = 0; samForceUp = 1; samUpChanged = 1;}
    else if(ch=='m') {cuisineForceUp = 0; cuisineForceDown = 1; cuisineDownChanged = 1;}
    else if(ch=='n') {cuisineForceDown = 0; cuisineForceUp = 1; cuisineUpChanged = 1;}
    else if(ch=='o') {cellierForceUp = 0; cellierForceDown = 1; cellierDownChanged = 1;}
    else if(ch=='p') {cellierForceDown = 0; cellierForceUp = 1; cellierUpChanged = 1;}
    else if(ch=='q') {genForceUp = 0; genForceDown = 1; genDownChanged = 1;}
    else if(ch=='r') {genForceDown = 0; genForceUp = 1; genUpChanged = 1;}

    delay(100);
  }
  endwin();
  return(0);
}
