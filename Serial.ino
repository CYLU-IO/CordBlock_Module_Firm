char m_altserial[512];

void serialInit() {
  Serial.begin(9600);
  altSerial.begin(9600);
}

void establishContact() {
  while (Serial.available() <= 0) {
    Serial.print('A'); //knock signal
    delay(300);
  }
}

void alivePulse() {
  //if (module_info.initialized) Serial.print('B'); //alive signal
}

void receiveSerial() {
  if (Serial.available() > 0) {
    char sig = char(Serial.read());

    if (sig == 'C') {
      String m = "";

      while (true) {
        if (Serial.available() > 0) {
          char c = char(Serial.read());

          if (c == 'D') break;

          m += c;
        }
      }

      if (!module_info.initialized) {
        StaticJsonDocument<16> data;
        DeserializationError err = deserializeJson(data, m);

        if (err == DeserializationError::Ok) {
          strcpy(module_info.name, "Plug");
          module_info.id = random(1000, 9999);
          module_info.addr = data["addr"].as<int>() + 1; //update self-addr as serial's addr + 1

          if (altSerial.available() <= 0) { //no next slave detected
            module_info.lastModule = true;
            StaticJsonDocument<64> data;

            data["amount"] = module_info.addr;

            JsonArray modulesArr = data.createNestedArray("modules");
            JsonArray moduleArr = modulesArr.createNestedArray();

            moduleArr[0] = module_info.id; //id
            moduleArr[1] = module_info.switchState; //state
            moduleArr[2] = module_info.current; //current
            moduleArr[3] = module_info.name; //name

            char re[64];
            serializeJson(data, re);

            Serial.print('C'); //start transferring return signal
            Serial.print(re); //transfer json data
            Serial.print('D'); //end transferring return signal
          } else {
            StaticJsonDocument<16> data;

            data["addr"] = module_info.addr;

            //clearAltSerial();

            altSerial.print('C');
            serializeJson(data, altSerial);
            altSerial.print('D');
          }

          module_info.initialized = true;
        }
      } else {
        //after init. json
      }
    } else if (sig == 'I') {
      i2cInit();
      digitalWrite(LED_PIN, HIGH);
      altSerial.print('I'); //pass to next module
      module_info.completeInit = true;
    }
  }
}

void receivealtSerial() {
  if (altSerial.available() > 0) {
    char sig = char(altSerial.read());

    if (sig == 'A' && module_info.initialized) {
      StaticJsonDocument<16> data;

      data["addr"] = module_info.addr;

      //clearAltSerial();

      altSerial.print('C');
      serializeJson(data, altSerial);
      altSerial.print('D');
    } else if (sig == 'C') { //receive Return array from the next module
      int _c = 0;

      while (true) {
        if (altSerial.available() > 0) {
          char c = char(altSerial.read());

          if (c == 'D') break;

          m_altserial[_c] = c;
          _c++;
        }
      }

      StaticJsonDocument<1024> data;
      DeserializationError err = deserializeJson(data, m_altserial);

      if (err == DeserializationError::Ok) {
        JsonArray moduleArr = data["modules"].createNestedArray();

        moduleArr[0] = module_info.id; //id
        moduleArr[1] = module_info.switchState; //state
        moduleArr[2] = module_info.current; //current
        moduleArr[3] = module_info.name; //name

        char re[512];
        serializeJson(data, re);

        Serial.print('C'); //start transferring return signal
        Serial.print(re); //transfer json data
        Serial.print('D'); //end transferring return signal*/
      }
    }
  }
}

void clearSerial() {
  while (Serial.available() > 0) Serial.read();
}

void clearAltSerial() {
  while (altSerial.available() > 0) altSerial.read();
}
