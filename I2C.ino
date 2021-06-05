/***
   Basic Functions
*/
void i2cInit() {
  Wire.begin(module_status.addr);
  Wire.onRequest(requestEvent);
  Wire.onReceive(receiveEvent);
}

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
  String str = int2str(module_config.id, 4) + String(module_config.switchState) + int2str(module_status.current, 4);
  byte re[str.length()];

  for (byte i = 0; i < str.length(); i++) re[i] = (byte)str.charAt(i);

  Wire.write(re , sizeof(re));
}
