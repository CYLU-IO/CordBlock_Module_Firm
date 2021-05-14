#include <Wire.h>
#include <Thread.h>
#include <EEPROM.h>
#include <ACS712.h>
#include <ezButton.h>
#include <ArduinoJson.h>
#include <AltSoftSerial.h>
#include <StaticThreadController.h>

#define MAX_CURRENT 1500

/***
   Pin Setups
*/
#define CURRENT_SENSOR_PIN A3
#define BUTTON_PIN 3
#define LED_PIN 6
#define RELAY_PIN 5
#define RESET_PIN 13

typedef struct {
  int id;
  int addr;
  int current;
  char name[25];
  bool switchState;
  bool initialized;
  bool completeInit;
  bool lastModule;
} Module_Info;

/*** Global Data ***/
Module_Info module_info;
AltSoftSerial altSerial;
uint32_t reseedRandomSeed EEMEM = 0xFFFFFFFF;
ACS712 currentSens(CURRENT_SENSOR_PIN, 5.0, 1023, 100);

/*** Thread Instances ***/
Thread* alivePulseThread = new Thread();
StaticThreadController<1> threadControl (alivePulseThread);

void setup() {
  module_info.initialized = false;

  //alivePulseThread->onRun(alivePulse);
  //alivePulseThread->setInterval(300);

  sensInit();
  serialInit();
  establishContact();
}

void loop() {
  sensLoop();
  receiveSerial();
  receivealtSerial();
  //threadControl.run();
}
