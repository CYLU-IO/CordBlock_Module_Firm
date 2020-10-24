/***
   Parameters
*/
int addr = 51; //initial I2C address
bool unique = false;
bool switchStat = false;
int priority = 0;

/***
   Basic Functions
*/
void i2cManInit() {
  clearEEROM();
  //If there is a determined address from the previous connection, ues it.
  if (readFromEEROM(0).length() > 0) addr = readFromEERPM(0).toInt();

  Wire.begin(addr);
  Wire.onRequest(requestEvent);
  Wire.onReceive(receiveEvent);

  randomSeed(analogRead(0));
}

void i2cManLoop() {
  unique = isUnique(addr);
}

/*
   I2C Utilites
*/
void receiveEvent() { //receive from Master
  String reqData, args = "";

  while (0 < Wire.available()) reqData += char(Wire.read());

  if (reqData.length() > 0) {
    args = reqData.substring(1);

    switch (reqData.substring(0, 1).toInt()) {
      case 1: //randomly update address
        randomSeed(analogRead(0));
        updateI2cAddr(random(args.substring(0, 2).toInt(), args.substring(2).toInt() + 1));
        break;

      case 2: //update certained address
        updateI2cAddr(args.toInt());
        break;

      case 3: //register an id
        
        break;
    }
  }
}

void requestEvent() {
  String resString = int2str(addr, 2) + String(unique) + String(switchStat) + String(priority) + int2str(current, 4);
  byte res[resString.length()];

  for (byte i = 0; i < resString.length(); i++) res[i] = (byte)resString.charAt(i);

  Wire.write(res, sizeof(res));
}

bool isUnique(int target) {
  String res = "";

  Wire.requestFrom(target, 3);

  while (Wire.available()) {
    char b = Wire.read();
    res += b;
  }

  return (res.substring(0).toInt() == target) ? false : true;
}

/*
   Identity Update
*/
void updateI2cAddr(int newAddr) {
  addr = newAddr;
  saveInEERPOM(0, int2str(addr, 2));
  Wire.begin(addr);
  Serial.println("The I2C address has been changed to " + String(addr));
}

/*
   Format Utilities
*/
String int2str(int n, int leng) {
  String re = "";

  for (int i = leng - 1; i > 0; i--) {
    if (n / (int)(pow(10, i) + ((n % 9 == 0) ? 1 : 0)) == 0) {
      re += "0";
      continue;
    }

    break;
  }

  return re + String(n);
}
