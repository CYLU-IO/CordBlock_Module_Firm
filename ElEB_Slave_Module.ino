#include <Wire.h>
#include <CRC32.h>
#include <EEPROM.h>
#include <ACS712.h>
#include <ezButton.h>
#include <ArduinoJson.h>
#include <StreamUtils.h>
#include <AltSoftSerial.h>


#define MAX_CURRENT 1500

/*** Pin Setups ***/
#define RST_PIN 2
#define BUTTON_PIN 3
#define LED_PIN 6
#define RELAY_PIN 5
#define CURRENT_SENSOR_PIN A3

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

/*** EEPROM Addressing ***/
#define  module_config_eeprom_address 0x00
uint32_t reseedRandomSeed EEMEM = 0xFFFFFFFF;

/*** Global Data ***/
AltSoftSerial altSerial;
ACS712 currentSens(CURRENT_SENSOR_PIN, 5.0, 1023, 100);

void setup() {
  //eeprom_erase();
  eeprom_read(module_config_eeprom_address, module_config);

  if (!module_config.initialized) { //new device
    module_config.type = 1; //American plug
  }

  sensInit();
  serialInit();
  establishContact();
}

void loop() {
  sensLoop();
  receiveSerial();
  receiveAltserial();
}
