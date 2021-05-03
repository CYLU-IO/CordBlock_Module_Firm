#include <Wire.h>
#include <Thread.h>
#include <EEPROM.h>
#include <ACS712.h>
#include <ezButton.h>
#include <ArduinoJson.h>
#include <SoftwareSerial.h>
#include <StaticThreadController.h>

#define MAX_CURRENT 1500

/***
   Pin Setups
*/
int currentSensPin = A3;
int buttonPin = 3;
int conncPin = 6;
int relayPin = 5;
int resetPin = 13;

/***
   Global Parameters
*/
int current = 0;
uint32_t reseedRandomSeed EEMEM = 0xFFFFFFFF;
ACS712 currentSens(currentSensPin, 5.0, 1023, 100);
SoftwareSerial serial1(8, 7); // RX, TX

/*
   SlaveParameter
*/
int id = 0;
int addr = 51; //initial I2C address
bool unique = false;
bool switchStat = false;
bool initialized = false;

/***
   Thread Instances
*/
Thread* sensThread = new Thread();
Thread* i2cThread = new Thread();
StaticThreadController<2> threadControl (sensThread, i2cThread);

void setup() {
  sensInit();
  serialInit();
  establishContact();
  
  //i2cInit();

  /*
  sensThread->onRun(sensLoop);
  sensThread->setInterval(1);

  i2cThread->onRun(i2cLoop);
  i2cThread->setInterval(1);*/
}

void loop() {
  //threadControl.run();
  receiveSerial1();

  delay(1000);
}
