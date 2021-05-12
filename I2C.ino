/***
   Basic Functions
*/
void i2cInit() {
  //clearEEPROM();
  //If there is a determined address from the previous connection, ues it.
  //if (readFromEEPROM(0).length() > 0) addr = readFromEEPROM(0).toInt();

  Wire.begin(addr);
  Wire.onRequest(requestEvent);
  Wire.onReceive(receiveEvent);
}
/*
   I2C Utilites
*/
void receiveEvent() { //receive from Master
  String reqData, args = "";

  while (0 < Wire.available()) reqData += char(Wire.read());

  if (reqData.length() > 0) {
    switch (reqData.substring(0, 1).toInt()) {
      case 1: //turn switch on
        turnSwitch(HIGH);
        break;

      case 2: //turn switch off
        turnSwitch(LOW);
        break;

      case 9: //start working
        initialized = true;
        //turnSwitch(HIGH);
        digitalWrite(LED_PIN, HIGH);
        break;
    }
  }
}

void requestEvent() {
  String str = int2str(id, 4) + String(switchStat) + int2str(current, 4);
  byte re[str.length()];

  for (byte i = 0; i < str.length(); i++) re[i] = (byte)str.charAt(i);

  Wire.write(re , sizeof(re));
}

/*
   Identity Update
*/
void updateI2cAddr(int newAddr) {
  addr = newAddr;
  Wire.begin(addr);
  Serial.println("The I2C address has been changed to " + String(addr));
}

void updateUid(int newId) {
  id = newId;
  //saveInEEPROM(2, int2str(addr, 4));
  Serial.println("The UID has been changed to " + String(id));
}
