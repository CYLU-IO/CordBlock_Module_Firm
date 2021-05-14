/***
   Basic Functions
*/
void i2cInit() {
  //clearEEPROM();
  //If there is a determined address from the previous connection, ues it.
  //if (readFromEEPROM(0).length() > 0) addr = readFromEEPROM(0).toInt();

  Wire.begin(module_info.addr);
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
    }
  }
}

void requestEvent() {
  String str = int2str(module_info.id, 4) + String(module_info.switchState) + int2str(module_info.current, 4);
  byte re[str.length()];

  for (byte i = 0; i < str.length(); i++) re[i] = (byte)str.charAt(i);

  Wire.write(re , sizeof(re));
}
