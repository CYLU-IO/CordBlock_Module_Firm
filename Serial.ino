void serialInit() {
  Serial.begin(9600);
  serial1.begin(9600);
}

void establishContact() {
  while (Serial.available() <= 0) {
    Serial.print('A'); //knock signal
    delay(200);
  }

  receiveSerial();
}

void receiveSerial() {
  StaticJsonDocument<32> data;
  DeserializationError err = deserializeJson(data, Serial);

  if (data["action"].as<int>() == 1) {
    id = (id == 0) ? random(1000, 9999) : id;
    addr = data["addr"].as<int>() + 1; //update self-addr as serial's addr + 1
    name += String(addr);
    i2cInit(); //initialize I2C
    initialized = true;

    if (serial1.available() <= 0) { //no next slave detected
      StaticJsonDocument<128> data;

      data["action"] = 2;
      data["amount"] = addr;

      JsonArray modulesArr = data.createNestedArray("modules");
      JsonArray moduleArr = modulesArr.createNestedArray();

      moduleArr[0] = id; //id
      moduleArr[1] = switchStat; //state
      moduleArr[2] = current; //current
      moduleArr[3] = name; //name

      char re[128];
      serializeJson(data, re);

      Serial.print('C'); //start transferring return signal
      Serial.print(re); //transfer json data
      Serial.print('D'); //end transferring return signal
    } else {
      if (serial1.available() > 0) {
        if (char(serial1.read()) == 'A') {
          StaticJsonDocument<32> data;

          data["action"] = 1;
          data["addr"] = addr;

          clearSerial1();
          serializeJson(data, serial1);
        }
      }
    }
  }
}

void receiveSerial1() {
  if (serial1.available() > 0) {
    char sig = char(serial1.read());

    if (sig == 'A') {
      StaticJsonDocument<32> data;

      data["action"] = 1;
      data["addr"] = addr;

      serializeJson(data, serial1);
    } else if (sig == 'C') { //receive Return array from the next module
      String m = "";

      while (true) {
        if (serial1.available() > 0) {
          char c = char(serial1.read());

          if (c == 'D') break;

          m += c;
        }
      }

      StaticJsonDocument<512> data;
      DeserializationError err = deserializeJson(data, m);

      if (err == DeserializationError::Ok) {
        JsonArray moduleArr = data["modules"].createNestedArray();

        moduleArr[0] = id; //id
        moduleArr[1] = switchStat; //state
        moduleArr[2] = current; //current
        moduleArr[3] = name; //name

        char re[512];
        serializeJson(data, re);

        Serial.print('C'); //start transferring return signal
        Serial.print(re); //transfer json data
        Serial.print('D'); //end transferring return signal
      }
    }
  }
}

void clearSerial() {
  while (Serial.available() > 0) Serial.read();
}

void clearSerial1() {
  while (serial1.available() > 0) serial1.read();
}
