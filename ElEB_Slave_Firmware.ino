#include <Wire.h>
#include <EEPROM.h>
#include <ezButton.h>
#include <Thread.h>
#include <StaticThreadController.h>

/***
   Global Parameters
*/
int current = 0;

/***
   Instances
*/
Thread* sensThread = new Thread();
Thread* i2cThread = new Thread();
StaticThreadController<2> threadControl (sensThread, i2cThread);

void setup() {
  Serial.begin(9600);
  Serial.println("Program starts");

  sensInit();
  i2cManInit();

  sensThread->onRun(sensLoop);
  i2cThread->onRun(i2cManLoop);
  sensThread->setInterval(1);
  i2cThread->setInterval(200);
}

void loop() {
  threadControl.run();

  //delay(20);
}
