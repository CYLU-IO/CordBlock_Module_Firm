#include <CRC.h>
#include <CRC8.h>
#include <Wire.h>
#include <EEPROM.h>
#include <ACS712.h>
#include <Button2.h>
#include <ArduinoJson.h>
#include <singleLEDLibrary.h>

#include "firm_definitions.h"

struct config_t {
  int  type;
  int  priority;
  int  initialized;
  char name[25];
  bool switchState;
} module_config;

struct Module_status {
  int  addr;
  int  current;
  int  mcub;
  int  controlTask;
  
  bool initialized;
  bool completeInit;
  
  bool moduleLiveSignal;
  bool moduleLivePrevious;
  unsigned long moduleLiveSentTime;
} module_status;

struct Test {
  bool overloading;
} test;

void setup() {
#if DEBUG
  Serial.begin(9600);
#endif

  sensInit();
  serialInit();
  eepromInit();
  buttonInit();
}

void loop() {
  sensLoop();
  buttonLoop();

  receiveSerial3();

  receiveSerial1();

  receiveSerial3();

  receiveSerial2();

  receiveSerial3();

  establishContact();
  ///ModuleLiveCheckRoutine();

  receiveSerial3();
}
