/***
   Instances
*/
const int PRESS_TIME = 2000; // 1000 milliseconds

ezButton button(BUTTON_PIN);

unsigned long pressedTime  = 0;
bool isPressing = false;
bool isLongDetected = false;

bool stimulation = false;

/***
   Basic Functions
*/
void sensInit() {
  digitalWrite(RESET_PIN, HIGH);
  
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(RESET_PIN, OUTPUT);
  pinMode(CURRENT_SENSOR_PIN, INPUT);

  currentSens.autoMidPoint(60);

  button.setDebounceTime(30);
  reseedRandom(&reseedRandomSeed);
}

void sensLoop() {
  button.loop();
  buttonEvent();

  if (!stimulation) current = getCurrent();
  
  if (current >= MAX_CURRENT) {
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
        current = 0000;
      } else {
        blinkLED(3);
        stimulation = true;
        current = 1600; //16A
      }
      
      //digitalWrite(RESET_PIN, LOW);
    }
  }
}

void blinkLED(int times) {
  blinkLED(times, 200);
}

void blinkLED(int times, int interval) {
  for (int i = 0; i < times; i++) {
    digitalWrite(LED_PIN, HIGH);
    delay(interval);
    digitalWrite(LED_PIN, LOW);
    delay(interval);
  }

  digitalWrite(LED_PIN, HIGH);
}

void turnSwitch() {
  turnSwitch((switchStat == false) ? HIGH : LOW);
}

void turnSwitch(int state) {
  if (current >= MAX_CURRENT) state = false;
  
  if (state == HIGH) {
    switchStat = true;
  } else {
    switchStat = false;
  }

  digitalWrite(RELAY_PIN, state);
}

int getCurrent() {
  return currentSens.mA_AC() / 10;
}
