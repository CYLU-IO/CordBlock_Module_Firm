bool currentSensorCalibrated = false;

/***
   Basic Functions
*/
void sensInit() {
  //digitalWrite(RST_PIN, HIGH);

  pinMode(RELAY_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  //pinMode(RST_PIN, OUTPUT);
  pinMode(CURRENT_SENSOR_PIN, INPUT);

  currentSens.autoMidPoint(60);
  while (!currentSensorCalibrated) getCurrent(2000);
}

void sensLoop() {
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
  sendUpdateMaster(Serial1, MODULE_SWITCH_STATE, (int)state);
  //eeprom_write(module_config_eeprom_address, module_config); //save state

  Serial.print("[SENSOR] Relay state changes to ");
  Serial.println(state);
}

unsigned long pt = millis();

int getCurrent() {
  static int previous_current = 0;
  int current = getCurrent(200);

  if (previous_current != current) {
    //sendUpdateMaster(Serial, MODULE_CURRENT, (int)current);
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
