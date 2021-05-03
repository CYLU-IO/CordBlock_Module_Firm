bool contacted = false;

void serialInit() {
  Serial.begin(9600);
  serial1.begin(9600);
}

void establishContact() {
  while (Serial.available() <= 0) {
    Serial.print('A'); //knock signal
    delay(300);
  }

  contacted = true;

  StaticJsonDocument<32> data;
  DeserializationError err = deserializeJson(data, Serial);

  if (data["action"].as<int>() == 1) {
    addr = data["addr"].as<int>() + 1; //update self-add as serial's addr + 1

    if (serial1.available() <= 0) { //no next slave detected
      id = random(1000, 9999);

      char re[128];
      StaticJsonDocument<128> data;
      
      data["action"] = 2;
      data["amount"] = addr;
      data["id"][0] = id;

      serializeJson(data, re);

      Serial.print('C'); //start transferring return signal
      Serial.print(re); //transfer json data
      Serial.print('D'); //end transferring return signal
      digitalWrite(conncPin, HIGH);
    } else {
      if (serial1.available() > 0) {
        if (char(serial1.read()) == 'A') {
          clearSerial1();
          StaticJsonDocument<32> data;

          data["action"] = 1;
          data["addr"] = addr;

          serializeJson(data, serial1);
        }
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

void serialAction() {

}

void receiveSerial1() {
  if (serial1.available()) { //receive from next slave
    String m = "";

    while (serial1.available() > 0) m += char(serial1.read());

    Serial.println(m);
  }
    /*StaticJsonDocument<200> data;
    DeserializationError err = deserializeJson(data, serial1);

    if (err == DeserializationError::Ok) {
      switch (data["action"].as<int>()) {
        case 1: //request to init addr
          addr = data["addr"].as<int>() + 1; //update self-add as serial's addr + 1
          //requestInitAddr(); //request next slave to init addr
          break;

        case 2: //receive next slave's return data
          Serial.println("I should process slave's data");
          break;
      }
    } else {
      Serial.println(err.c_str());

      while (Serial.available() > 0) //Flush all bytes buffer
        Serial.read();
    }
  }*/
}
