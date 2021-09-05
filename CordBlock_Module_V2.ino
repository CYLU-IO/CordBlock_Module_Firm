#include <CRC.h>
#include <CRC8.h>
#include <Wire.h>
#include <EEPROM.h>
#include <ACS712.h>
#include <Button2.h>
#include <ArduinoJson.h>
#include <singleLEDLibrary.h>

#include "definitions.h"

struct config_t {
  char name[25];
  int  type;
  int  priority;
  int  initialized;
  bool switchState;
} module_config;

struct module_status_t {
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
  static enum {
    TASK_PHY, TASK_UART1, TASK_UART2, TASK_ROUTINE
  } stage = TASK_PHY;

  switch (stage) {
    case TASK_PHY: {
        sensLoop();
        buttonLoop();
        
        stage = TASK_UART1;
        break;
      }
    case TASK_UART1: {
        receiveSerial1();
        
        stage = TASK_UART2;
        break;
      }
    case TASK_UART2: {
        receiveSerial2();

        stage = TASK_ROUTINE;
        break;
      }
    case TASK_ROUTINE: {
        establishContact();
        ModuleLiveCheckRoutine();

        stage = TASK_PHY;
        break;
      }
  }

  receiveSerial3();
}

///// Module Task Handler /////
void taskLoop() {
  switch (module_status.controlTask) {
    case DO_TURN_ON:
      turnSwitch(HIGH);
      break;

    case DO_TURN_OFF:
      turnSwitch(LOW);
      break;
  }

  module_status.controlTask = 0;
}

///// CRC8 /////
uint8_t calcCRC(uart_msg_pack* pack) {
  static CRC8 crc;

  crc.reset();
  crc.setPolynome(0x05);
  crc.add((uint8_t)pack->cmd);
  crc.add((uint8_t*)pack->payload, pack->length);

  return crc.getCRC();
}

uint8_t calcCRC(char* str, int length) {
  static CRC8 crc;

  crc.reset();
  crc.setPolynome(0x05);
  crc.add((uint8_t*)str, length);

  return crc.getCRC();
}
