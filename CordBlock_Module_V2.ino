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
  int id;
  int type;
  char name[25];
  bool switchState;
  int initialized;
} module_config;

struct Module_status {
  int addr;
  int current;
  int controlTask;
  bool initialized;
  bool completeInit;
  bool lastModule;
} module_status;

void setup() {
#if DEBUG
  Serial.begin(9600);
#endif

  i2cInit();
  sensInit();
  serialInit();
  eepromInit();
  buttonInit();
}

void loop() {
  sensLoop();
  buttonLoop();

  establishContact();

  receiveSerial1();
  receiveSerial2();
}
