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
  bool initialized;
} module_config;

struct Module_status {
  int addr;
  int current;
  bool initialized;
  bool completeInit;
  bool lastModule;
} module_status;

void setup() {
  i2cInit();
  sensInit();
  serialInit();
  eepromInit();
  buttonInit();
  
  establishContact();
  Serial.begin(9600);
}

void loop() {
  sensLoop();
  buttonLoop();
  
  receiveSerial1();
  receiveSerial2();
}
