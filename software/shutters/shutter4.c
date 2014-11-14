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
 *  gcc -Wall -o shutter shutter.c -lwiringOli
 *  sudo ./shutter
 *
 * @author denm
 */
#include <stdio.h>
#include <wiringOli.h>
#include <oliExt.h>
#include <time.h>

// Timing for shutters
const int SMALL = 15000;
const int MEDIUM = 20000;
const int BIG = 35000;


void printDate();

// Global variable for time
struct tm *timeinfo;
time_t rawtime;
char strResponse[128];

int motorUp[] = {0, 2, 4, 6, 8, 10, 12, 14};
int motorDown[] = {1, 3, 5, 7, 9, 11, 13, 15};

int buttonUp[] = {1, 3, 5, 7, 9, 11, 13, 15};
int buttonDown[] = {0, 2, 4, 6, 8, 10, 12, 14};

int buttonCentralizedUp = 0;
int buttonCentralizedDown = 1;

// Global variable for button status
int salonUp = 0;
int salonDown = 0;
int salonUpChanged = 0;
int salonDownChanged = 0;

int sdbUp = 0;
int sdbDown = 0;
int sdbUpChanged = 0;
int sdbDownChanged = 0;

int erineUp = 0;
int erineDown = 0;
int erineUpChanged = 0;
int erineDownChanged = 0;

int evaUp = 0;
int evaDown = 0;
int evaUpChanged = 0;
int evaDownChanged = 0;

int lauraUp = 0;
int lauraDown = 0;
int lauraUpChanged = 0;
int lauraDownChanged = 0;

int samUp = 0;
int samDown = 0;
int samUpChanged = 0;
int samDownChanged = 0;

int cuisineUp = 0;
int cuisineDown = 0;
int cuisineUpChanged = 0;
int cuisineDownChanged = 0;

int cellierUp = 0;
int cellierDown = 0;
int cellierUpChanged = 0;
int cellierDownChanged = 0;

int genUp = 0;
int genDown = 0;
int genUpChanged = 0;
int genDownChanged = 0;

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
      if (salonUp != 0) {salonUpChanged = 1;  printf("Salon button up = 1 "); fflush(stdout); printDate();}
      else {salonUpChanged = 0;  printf("Salon button up = 0 "); fflush(stdout); printDate();}
    }
    if (salonDown != digitalReadPortA(buttonDown[0]))
    {
      salonDown = digitalReadPortA(buttonDown[0]);
      if (salonDown != 0) {salonDownChanged = 1;  printf("Salon button down = 1 "); fflush(stdout); printDate();}
      else {salonDownChanged = 0;  printf("Salon button down = 0 "); fflush(stdout); printDate();}
    }
    // Sdb
    if (sdbUp != digitalReadPortA(buttonUp[1]))
    {
      sdbUp = digitalReadPortA(buttonUp[1]);
      if (sdbUp != 0) { sdbUpChanged = 1; printf("Sdb button up = 1 "); fflush(stdout); printDate();}
      else { sdbUpChanged = 0;  printf("Sdb button up = 0 "); fflush(stdout); printDate();}
    }
    if (sdbDown != digitalReadPortA(buttonDown[1]))
    {
      sdbDown = digitalReadPortA(buttonDown[1]);
      if (sdbDown != 0) {sdbDownChanged = 1;  printf("Sdb button down = 1 "); fflush(stdout); printDate();}
      else {sdbDownChanged = 0;  printf("Sdb button down = 0 "); fflush(stdout); printDate();}
    }
    // Erine
    if (erineUp != digitalReadPortA(buttonUp[2]))
    {
      erineUp = digitalReadPortA(buttonUp[2]);
      if (erineUp != 0) {erineUpChanged = 1;  printf("Erine button up = 1 "); fflush(stdout); printDate();}
      else {erineUpChanged = 0;  printf("Erine button up = 0 "); fflush(stdout); printDate();}
    }
    if (erineDown != digitalReadPortA(buttonDown[2]))
    {
      erineDown = digitalReadPortA(buttonDown[2]);
      if (erineDown != 0) {erineDownChanged = 1;  printf("Erine button down = 1 "); fflush(stdout); printDate();}
      else {erineDownChanged = 0;  printf("Erine button down = 0 "); fflush(stdout); printDate();}
    }
    // Eva
    if (evaUp != digitalReadPortA(buttonUp[3]))
    {
      evaUp = digitalReadPortA(buttonUp[3]);
      if (evaUp != 0) {evaUpChanged = 1;  printf("Eva button up = 1 "); fflush(stdout); printDate();}
      else {evaUpChanged = 0;  printf("Eva button up = 0 "); fflush(stdout); printDate();}
    }
    if (evaDown != digitalReadPortA(buttonDown[3]))
    {
      evaDown = digitalReadPortA(buttonDown[3]);
      if (evaDown != 0) {evaDownChanged = 1;  printf("Eva button down = 1 "); fflush(stdout); printDate();}
      else {evaDownChanged = 0;  printf("Eva button down = 0 "); fflush(stdout); printDate();}
    }
    // Laura
    if (lauraUp != digitalReadPortA(buttonUp[4]))
    {
      lauraUp = digitalReadPortA(buttonUp[4]);
      if (lauraUp != 0) {lauraUpChanged = 1;  printf("Laura button up = 1 "); fflush(stdout); printDate();}
      else {lauraUpChanged = 0;  printf("Laura button up = 0 "); fflush(stdout); printDate();}
    }
    if (lauraDown != digitalReadPortA(buttonDown[4]))
    {
      lauraDown = digitalReadPortA(buttonDown[4]);
      if (lauraDown != 0) {lauraDownChanged = 1;  printf("Laura button down = 1 "); fflush(stdout); printDate();}
      else {lauraDownChanged = 0;  printf("Laura button down = 0 "); fflush(stdout); printDate();}
    }
    // Sam
    if (samUp != digitalReadPortA(buttonUp[5]))
    {
      samUp = digitalReadPortA(buttonUp[5]);
      if (samUp != 0) {samUpChanged = 1;  printf("Sam button up = 1 "); fflush(stdout); printDate();}
      else {samUpChanged = 0;  printf("Sam button up = 0 "); fflush(stdout); printDate();}
    }
    if (samDown != digitalReadPortA(buttonDown[5]))
    {
      samDown = digitalReadPortA(buttonDown[5]);
      if (samDown != 0) {samDownChanged = 1;  printf("Sam button down = 1 "); fflush(stdout); printDate();}
      else {samDownChanged = 0;  printf("Sam button down = 0 "); fflush(stdout); printDate();}
    }
    // Cuisine
    if (cuisineUp != digitalReadPortA(buttonUp[6]))
    {
      cuisineUp = digitalReadPortA(buttonUp[6]);
      if (cuisineUp != 0) {cuisineUpChanged = 1;  printf("Cuisine button up = 1 "); fflush(stdout); printDate();}
      else {cuisineUpChanged = 0;  printf("Cuisine  button up = 0 "); fflush(stdout); printDate();}
    }
    if (cuisineDown != digitalReadPortA(buttonDown[6]))
    {
      cuisineDown = digitalReadPortA(buttonDown[6]);
      if (cuisineDown != 0) {cuisineDownChanged = 1;  printf("Cuisine button down = 1 "); fflush(stdout); printDate();}
      else {cuisineDownChanged = 0;  printf("Cuisine button down = 0 "); fflush(stdout); printDate();}
    }
    // Cellier
    if (cellierUp != digitalReadPortA(buttonUp[7]))
    {
      cellierUp = digitalReadPortA(buttonUp[7]);
      if (cellierUp != 0) {cellierUpChanged = 1;  printf("Cellier button up = 1 "); fflush(stdout); printDate();}
      else {cellierUpChanged = 0;  printf("Cellier button up = 0 "); fflush(stdout); printDate();}
    }
    if (cellierDown != digitalReadPortA(buttonDown[7]))
    {
      cellierDown = digitalReadPortA(buttonDown[7]);
      if (cellierDown != 0) {cellierDownChanged = 1;  printf("Cellier button down = 1 "); fflush(stdout); printDate();}
      else {cellierDownChanged = 0;  printf("Cellier button down = 0 "); fflush(stdout); printDate();}
    }
    // Gen
    if (genUp != digitalReadPortC(buttonCentralizedUp))
    {
      genUp = digitalReadPortC(buttonCentralizedUp);
      if (genUp != 0) {genUpChanged = 1;  printf("Gen button up = 1 "); fflush(stdout); printDate();}
      else {genUpChanged = 0;  printf("Gen button up = 0 "); fflush(stdout); printDate();}
    }
    if (genDown != digitalReadPortC(buttonCentralizedDown))
    {
      genDown = digitalReadPortC(buttonCentralizedDown);
      if (genDown != 0) {genDownChanged = 1;  printf("Gen button down = 1 "); fflush(stdout); printDate();}
      else {genDownChanged = 0;  printf("Gen button down = 0 "); fflush(stdout); printDate();}
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
      while((timout < MEDIUM) && (digitalReadPortA(buttonUp[0]) == 1))
      {
        delay(1);
        timout++;
      }
      // Motor off
      digitalWritePortB(motorUp[0], LOW);
    }
    // Check if button status changed
    if (salonDownChanged == 1)
    {
      salonDownChanged = 0;
      digitalWritePortB(motorUp[0], LOW);
      digitalWritePortB(motorDown[0], HIGH);
      printf("Salon down\n"); fflush(stdout);
      int timout = 0;
      while((timout < MEDIUM) && (digitalReadPortA(buttonDown[0]) == 1))
      {
        delay(1);
        timout++;
      }
      // Motor off
      digitalWritePortB(motorDown[0], LOW);
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
      while((timout < SMALL) && (digitalReadPortA(buttonUp[1]) == 1))
      {
        delay(1);
        timout++;
      }
      // Motor off
      digitalWritePortB(motorUp[1], LOW);
    }
    // Check if button status changed
    if (sdbDownChanged == 1)
    {
      sdbDownChanged = 0;
      digitalWritePortB(motorUp[1], LOW);
      digitalWritePortB(motorDown[1], HIGH);
      printf("Sdb down\n"); fflush(stdout);
      int timout = 0;
      while((timout < SMALL) && (digitalReadPortA(buttonDown[1]) == 1))
      {
        delay(1);
        timout++;
      }
      // Motor off
      digitalWritePortB(motorDown[1], LOW);
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
      while((timout < MEDIUM) && (digitalReadPortA(buttonUp[2]) == 1))
      {
        delay(1);
        timout++;
      }
      // Motor off
      digitalWritePortB(motorUp[2], LOW);
    }
    // Check if button status changed
    if (erineDownChanged == 1)
    {
      erineDownChanged = 0;
      digitalWritePortB(motorUp[2], LOW);
      digitalWritePortB(motorDown[2], HIGH);
      printf("Erine down\n"); fflush(stdout);
      int timout = 0;
      while((timout < MEDIUM) && (digitalReadPortA(buttonDown[2]) == 1))
      {
        delay(1);
        timout++;
      }
      // Motor off
      digitalWritePortB(motorDown[2], LOW);
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
      while((timout < MEDIUM) && (digitalReadPortA(buttonUp[3]) == 1))
      {
        delay(1);
        timout++;
      }
      // Motor off
      digitalWritePortB(motorUp[3], LOW);
    }
    // Check if button status changed
    if (evaDownChanged == 1)
    {
      evaDownChanged = 0;
      digitalWritePortB(motorUp[3], LOW);
      digitalWritePortB(motorDown[3], HIGH);
      printf("Eva down\n"); fflush(stdout);
      int timout = 0;
      while((timout < MEDIUM) && (digitalReadPortA(buttonDown[3]) == 1))
      {
        delay(1);
        timout++;
      }
      // Motor off
      digitalWritePortB(motorDown[3], LOW);
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
      while((timout < MEDIUM) && (digitalReadPortA(buttonUp[4]) == 1))
      {
        delay(1);
        timout++;
      }
      // Motor off
      digitalWritePortB(motorUp[4], LOW);
    }
    // Check if button status changed
    if (lauraDownChanged == 1)
    {
      lauraDownChanged = 0;
      digitalWritePortB(motorUp[4], LOW);
      digitalWritePortB(motorDown[4], HIGH);
      printf("Laura down\n"); fflush(stdout);
      int timout = 0;
      while((timout < MEDIUM) && (digitalReadPortA(buttonDown[4]) == 1))
      {
        delay(1);
        timout++;
      }
      // Motor off
      digitalWritePortB(motorDown[4], LOW);
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
      while((timout < BIG) && (digitalReadPortA(buttonUp[5]) == 1))
      {
        delay(1);
        timout++;
      }
      // Motor off
      digitalWritePortB(motorUp[5], LOW);
    }
    // Check if button status changed
    if (samDownChanged == 1)
    {
      samDownChanged = 0;
      digitalWritePortB(motorUp[5], LOW);
      digitalWritePortB(motorDown[5], HIGH);
      printf("Sam down\n"); fflush(stdout);
      int timout = 0;
      while((timout < BIG) && (digitalReadPortA(buttonDown[5]) == 1))
      {
        delay(1);
        timout++;
      }
      // Motor off
      digitalWritePortB(motorDown[5], LOW);
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
      while((timout < BIG) && (digitalReadPortA(buttonUp[6]) == 1))
      {
        delay(1);
        timout++;
      }
      // Motor off
      digitalWritePortB(motorUp[6], LOW);
    }
    // Check if button status changed
    if (cuisineDownChanged == 1)
    {
      cuisineDownChanged = 0;
      digitalWritePortB(motorUp[6], LOW);
      digitalWritePortB(motorDown[6], HIGH);
      printf("Cuisine down\n"); fflush(stdout);
      int timout = 0;
      while((timout < BIG) && (digitalReadPortA(buttonDown[6]) == 1))
      {
        delay(1);
        timout++;
      }
      // Motor off
      digitalWritePortB(motorDown[6], LOW);
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
      while((timout < SMALL) && (digitalReadPortA(buttonUp[7]) == 1))
      {
        delay(1);
        timout++;
      }
      // Motor off
      digitalWritePortB(motorUp[4], LOW);
    }
    // Check if button status changed
    if (cellierDownChanged == 1)
    {
      cellierDownChanged = 0;
      digitalWritePortB(motorUp[7], LOW);
      digitalWritePortB(motorDown[7], HIGH);
      printf("Cellier down\n"); fflush(stdout);
      int timout = 0;
      while((timout < SMALL) && (digitalReadPortA(buttonDown[7]) == 1))
      {
        delay(1);
        timout++;
      }
      // Motor off
      digitalWritePortB(motorDown[7], LOW);
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
      while((timout < BIG) && (digitalReadPortC(buttonCentralizedUp) == 1))
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
      while((timout < BIG) && (digitalReadPortC(buttonCentralizedDown) == 1))
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
  genUp = digitalReadPortC(buttonCentralizedUp);
  genDown = digitalReadPortC(buttonCentralizedDown);
}

void printDate()
{
  rawtime = time(NULL);
  timeinfo = localtime(&rawtime);
  strftime(strResponse, 128, "%H:%M:%S %d-%b-%Y", timeinfo);
  printf("%s\n", strResponse);
  fflush(stdout);
}

int main(void)
{
  // Setup GPIO
  wiringOliSetup();
  oliExtSetup();
  int i;
  // Port A as input, B as output
  for (i=0; i<16; i++)
  {
    pinModePortA(i, INPUT);
    pullUpDnCtrlPortA(i, 2);
    pinModePortB(i, OUTPUT);
    pullUpDnCtrlPortB(i, 2);
    digitalWritePortB(i, LOW);
  }
  // Port General as input
  pinModePortC(buttonCentralizedUp, INPUT);
  pullUpDnCtrlPortC(buttonCentralizedUp, 2);
  pinModePortC(buttonCentralizedDown, INPUT);
  pullUpDnCtrlPortC(buttonCentralizedDown, 2);
  // Start
  printf("Start loop\n");
  printDate();

  // Read initial state
  setup();

  int x1 = oliThreadCreate(buttons);
  if (x1 != 0) printf ("Buttons didn't start\n");

  int x2 = oliThreadCreate(salon);
  if (x2 != 0) printf ("Salon didn't start\n");

  int x3 = oliThreadCreate(sdb);
  if (x3 != 0) printf ("Sdb didn't start\n");

  int x4 = oliThreadCreate(erine);
  if (x4 != 0) printf ("Erine didn't start\n");

  int x5 = oliThreadCreate(eva);
  if (x5 != 0) printf ("Eva didn't start\n");

  int x6 = oliThreadCreate(laura);
  if (x6 != 0) printf ("Laura didn't start\n");

  int x7 = oliThreadCreate(sam);
  if (x7 != 0) printf ("Sam didn't start\n");

  int x8 = oliThreadCreate(cuisine);
  if (x8 != 0) printf ("Cuisine didn't start\n");

  int x9 = oliThreadCreate(cellier);
  if (x9 != 0) printf ("Cellier didn't start\n");
  
  int xA = oliThreadCreate(gen);
  if (xA != 0) printf ("Gen didn't start\n");

  // Loop
  while(1)
  {
    delay(5000);
  }

}
