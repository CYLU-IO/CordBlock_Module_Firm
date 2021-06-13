bool currentSensorCalibrated = false;

ACS712 currentSens(CURRENT_SENSOR_PIN, 5.0, 1024, 100);

sllib led(LED_PIN);

void sensInit() {
  //digitalWrite(RST_PIN, HIGH);

  //pinMode(RST_PIN, OUTPUT);
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  //pinMode(CURRENT_SENSOR_PIN, INPUT);

  //currentSens.autoMidPoint(60);
  //while (!currentSensorCalibrated) getCurrent(2000);
}

void sensLoop() {
  led.update();
}

/*** Relay ***/
void turnSwitch() {
  turnSwitch((module_config.switchState == false) ? HIGH : LOW);
}

void turnSwitch(int state) {
  if (module_status.current >= MAX_CURRENT) state = false;

  module_config.switchState = state;
 
  sendUpdateMaster(Serial1, MODULE_SWITCH_STATE, (int)state);
  eepromUpdate(MODULE_CONFIG_EEPROM_ADDR, module_config); //save state

  digitalWrite(RELAY_PIN, state);
  Serial.print("[SENSOR] Relay state changes to "); Serial.println(state);
}


/*** Current Sensor ***/
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
