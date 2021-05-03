/***
   Instances
*/
const int PRESS_TIME = 2000; // 1000 milliseconds

ezButton button(buttonPin);

unsigned long pressedTime  = 0;
bool isPressing = false;
bool isLongDetected = false;

bool stimulation = false;

/***
   Basic Functions
*/
void sensInit() {
  digitalWrite(resetPin, HIGH);

  pinMode(relayPin, OUTPUT);
  pinMode(conncPin, OUTPUT);
  pinMode(resetPin, OUTPUT);
  pinMode(currentSensPin, INPUT);

  currentSens.autoMidPoint(60);

  button.setDebounceTime(30);
  reseedRandom(&reseedRandomSeed);
}

void sensLoop() {
  button.loop();
  buttonEvent();

  if (!stimulation) current = getCurrent();
  //Serial.println(int2str(getCurrent(), 4));
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

    if ( pressDuration < PRESS_TIME )
      Serial.println("Turn switch");
      turnSwitch();

  }

  if (isPressing == true && isLongDetected == false) {
    long pressDuration = millis() - pressedTime;

    if ( pressDuration > PRESS_TIME ) {
      isLongDetected = true;

      if (stimulation) {
        Serial.println("Close Stimulation...");
        stimulation = false;
        current = 0000;
      } else {
        Serial.println("Stimulate Overloading...");
        stimulation = true;
        current = 1600; //16A
      }
      
      //digitalWrite(resetPin, LOW);
    }
  }
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

  digitalWrite(relayPin, state);
}

int getCurrent() {
  return currentSens.mA_AC() / 10;
}
