/***
   Properties
*/
int relayPin = 3;
int currentSensPin = A3;
int buttonPin = 4;
int resetPin = 5;

/***
   Instances
*/
const int PRESS_TIME = 2000; // 1000 milliseconds

ezButton button(buttonPin);

unsigned long pressedTime  = 0;
bool isPressing = false;
bool isLongDetected = false;

/***
   Basic Functions
*/
void sensInit() {
  digitalWrite(resetPin, HIGH);
  
  pinMode(relayPin, OUTPUT);
  pinMode(resetPin, OUTPUT);
  pinMode(currentSensPin, INPUT);

  digitalWrite(relayPin, HIGH);

  button.setDebounceTime(30);
}

void sensLoop() {
  button.loop();
  buttonEvent();

  current = mapCurrent(measureCurrent());
  Serial.println(int2str(current, 4));
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
      Serial.println("A short press is detected");
  }

  if (isPressing == true && isLongDetected == false) {
    long pressDuration = millis() - pressedTime;

    if ( pressDuration > PRESS_TIME ) {
      Serial.println("Reset...");
      digitalWrite(resetPin, LOW);
      isLongDetected = true;
    }
  }
}

int measureCurrent() {
  float collection = 0;

  for (int i = 0; i < 5; i++) {
    collection += pow(analogRead(currentSensPin), 2);
    delay(20);
  }

  return max((int)sqrt(collection / 5), 510);
}

int mapCurrent(int amp) {
  return map(amp, 510, 1023, 0, 1500);
}
