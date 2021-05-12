#include <Wire.h>
#include <EEPROM.h>
#include <ACS712.h>
#include <ezButton.h>
#include <ArduinoJson.h>
#include <SoftwareSerial.h>

#define MAX_CURRENT 1500

/***
   Pin Setups
*/
#define CURRENT_SENSOR_PIN A3
#define BUTTON_PIN 3
#define LED_PIN 6
#define RELAY_PIN 5
#define RESET_PIN 13

/***
   Global Parameters
*/
int current = 0;
uint32_t reseedRandomSeed EEMEM = 0xFFFFFFFF;
ACS712 currentSens(CURRENT_SENSOR_PIN, 5.0, 1023, 100);
SoftwareSerial serial1(8, 7); // RX, TX

/*
   SlaveParameter
*/
String name = "插座";
int id = 0;
int addr = 51; //initial I2C address
bool switchStat = false;
bool initialized = false;

void setup() {
  sensInit();
  serialInit();
}

void loop() {
  if (!initialized) establishContact();
  
  sensLoop();
  receiveSerial1();

  /*
  if (serial1.available()) { //receive from next slave
    String m = "";

    while (serial1.available() > 0) m += char(serial1.read());

    Serial.println(m);
  }*/
}
