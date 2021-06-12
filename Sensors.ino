/***
   Instances
*/
const int PRESS_TIME = 2000; // 1000 milliseconds

ezButton button(BUTTON_PIN);

unsigned long pressedTime  = 0;
bool isPressing = false;
bool isLongDetected = false;

bool stimulation = false;

bool currentSensorCalibrated = false;

/***
   Basic Functions
*/
void sensInit() {
  digitalWrite(RST_PIN, HIGH);

  pinMode(RELAY_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(RST_PIN, OUTPUT);
  pinMode(CURRENT_SENSOR_PIN, INPUT);

  currentSens.autoMidPoint(60);
  while (!currentSensorCalibrated) getCurrent(2000);

  button.setDebounceTime(30);
  reseedRandom(&reseedRandomSeed);
}

void sensLoop() {
  button.loop();
  buttonEvent();

  if (!stimulation) module_status.current = getCurrent();

  if (module_status.current >= MAX_CURRENT) {
    turnSwitch(false);
    blinkLED(1, 100);
  }
}

void buttonEvent() {
  if (button.isPressed()) {
    pressedTime = millis();
    isPressing = true;
    isLongDetected = false;
  }

  if (button.isReleased()) {
    isPressing = false;

    long pressDuration = millis() - pressedTime;

    if ( pressDuration < PRESS_TIME ) turnSwitch(); //quick press
  }

  if (isPressing == true && isLongDetected == false) {
    long pressDuration = millis() - pressedTime;

    if ( pressDuration > PRESS_TIME ) {
      isLongDetected = true;

      if (stimulation) {
        blinkLED(2);
        stimulation = false;
        module_status.current = 0000;
      } else {
        blinkLED(3);
        stimulation = true;
        module_status.current = 1600; //16A
      }

      //digitalWrite(RST_PIN, LOW);
    }
  }
}

void blinkLED(int times) {
  blinkLED(times, 200);
}

void blinkLED(int times, int interval) {
  int origin = digitalRead(LED_PIN);

  digitalWrite(LED_PIN, LOW);

  for (int i = 0; i < times; i++) {
    digitalWrite(LED_PIN, HIGH);
    delay(interval);
    digitalWrite(LED_PIN, LOW);
    delay(interval);
  }

  digitalWrite(LED_PIN, origin);
}

void turnSwitch() {
  turnSwitch((module_config.switchState == false) ? HIGH : LOW);
}

void turnSwitch(int state) {
  if (module_status.current >= MAX_CURRENT) state = false;

  module_config.switchState = state;

  digitalWrite(RELAY_PIN, state);
  sendUpdateMaster(Serial, MODULE_SWITCH_STATE, (int)state);
  eeprom_write(module_config_eeprom_address, module_config); //save state
}

unsigned long pt = millis();

int getCurrent() {
  static int previous_current = 0;
  int current = getCurrent(200);

  if (previous_current != current) {
    sendUpdateMaster(Serial, MODULE_CURRENT, (int)current);
    previous_current = current;
  }

  return current;
}
int getCurrent(int interval) {
  static int c = 0;
  static int a = 0;
  static int a_count = 0;
  static int offset = 0;

  if (millis() - pt < interval) {
    a += currentSens.mA_AC(60) / 10;
    a_count++;
  } else {
    c = a / a_count - offset;

    if (!currentSensorCalibrated && c > 0) {
      offset = c;
      c -= offset;
      currentSensorCalibrated = true;
    }

    a = 0;
    a_count = 0;
    pt = millis();
  }

  return c < 0 ? 0 : c;
}
