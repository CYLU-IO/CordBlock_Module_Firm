#include <Wire.h>
#include <EEPROM.h>
#include <ezButton.h>
#include <Thread.h>
#include <StaticThreadController.h>
#include <ACS712.h>

#define MAX_CURRENT 1500
#define I2C_FREQUENCY 100000

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
ACS712 currentSens(currentSensPin, 5.0, 1023, 100);

/***
   Thread Instances
*/
Thread* sensThread = new Thread();
Thread* i2cThread = new Thread();
StaticThreadController<2> threadControl (sensThread, i2cThread);

void setup() {
  Serial.begin(9600);
  Serial.println("Program starts");

  sensInit();
  i2cInit();

  sensThread->onRun(sensLoop);
  sensThread->setInterval(1);

  i2cThread->onRun(i2cLoop);
  i2cThread->setInterval(200);
}

void loop() {
  threadControl.run();
  delay(20);
}
