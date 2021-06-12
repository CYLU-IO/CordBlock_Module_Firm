#include <CRC.h>
#include <CRC8.h>
#include <Wire.h>
#include <EEPROM.h>
#include <ACS712.h>
#include <ezButton.h>
#include <ArduinoJson.h>
#include <AltSoftSerial.h>


#define MAX_CURRENT 1500
#define MAX_MODULES 20

/*** Pin Setups ***/
#define RST_PIN 2
#define BUTTON_PIN 3
#define LED_PIN 6
#define RELAY_PIN 5
#define CURRENT_SENSOR_PIN A3

/*** SERIAL ***/
#define CMD_FAIL            0x11
#define CMD_EOF             0x20
#define CMD_REQ_ADR         0x41 //'A'
#define CMD_LOAD_MODULE     0x42 //'B'
#define CMD_CONFIRM_RECEIVE 0x43 //'C'
#define CMD_DO_MODULE       0x44 //'D'
#define CMD_HI              0x48 //'H'
#define CMD_INIT_MODULE     0x49 //'I'
#define CMD_LINK_MODULE     0x4C //'L'
#define CMD_UPDATE_MASTER   0x55 //'U'
#define CMD_START           0xFF

/*** Module Actions ***/
#define DO_TURN_ON          0x6E //'n'
#define DO_TURN_OFF         0x66 //'f'

/*** Characteristic Type ***/
#define MODULE_SWITCH_STATE 0x61 //'a' 
#define MODULE_CURRENT      0x62 //'b'

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
#define  module_config_eeprom_address 0x10
uint32_t reseedRandomSeed EEMEM = 0x00;

/*** Global Data ***/
AltSoftSerial altSerial;
ACS712 currentSens(CURRENT_SENSOR_PIN, 5.0, 1024, 100);

void setup() {
  eeprom_read(module_config_eeprom_address, module_config);

  if (module_config.initialized == 255 || module_config.id < 1000 || module_config.id > 9999) eeprom_erase();

  if (!module_config.initialized) {
    module_config.type = 1; //American plug
  }

  i2cInit();
  sensInit();
  serialInit();
  establishContact();
}

void loop() {
  sensLoop();
  receiveSerial();
  receiveAltserial();
}
