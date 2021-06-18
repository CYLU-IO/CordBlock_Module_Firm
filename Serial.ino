StaticJsonDocument<96> data;

void serialInit() {
  Serial1.swap(0);
  Serial1.begin(9600);

  Serial2.swap(0);
  Serial2.begin(9600);
}

void establishContact() {
  static unsigned long t;

  if (!module_status.initialized) {
    if (millis() - t > 300) {
      sendCmd(Serial1, CMD_REQ_ADR);
      led.blinkSingle(50);
      t = millis();
    }
  }
}

void receiveSerial1() {
  static CMD_STATE state = RC_NONE;

  static char cmd;
  static int length;
  static int buffer_pos;
  static char buffer[96];

  if (Serial1.available()) {
    switch (receiveCmd(Serial1, state, cmd, length, buffer_pos, buffer)) {
      case CMD_LOAD_MODULE:
        led.setOffSingle();

        if (length < 1) return;

        sendCmd(Serial2, CMD_HI);
        module_status.addr = (int)buffer[0] + 1; //update self-addr as the increasement of the previous addr

        if (module_config.initialized != 0x01) {
          module_config.id = random(1000, 9999);
          strcpy(module_config.name, (String("Switch ") + String(module_status.addr)).c_str());
          module_config.type = 1; //American plug
          module_config.initialized = 0x01;
        }

        delay(100);

        if (Serial2.available() == 0) { //last module
          module_status.lastModule = true;

          data["total"] = module_status.addr;
          data["addr"] = module_status.addr;
          data["id"] = module_config.id;
          data["switch_state"] = (int)module_config.switchState;
          data["name"] = module_config.name;

          int l = measureJson(data);
          char *p = (char*)malloc(l * sizeof(char));
          serializeJson(data, p, l);
          sendCmd(Serial1, CMD_LINK_MODULE, p, l);
          free(p);
        } else {
          clearSerial(Serial2);
          sendAddress(Serial2);
        }

        module_status.initialized = true;

#if DEBUG
        Serial.println("[UART] Module status: initialized");
#endif
        break;

      case CMD_HI:
        sendCmd(Serial1, CMD_HI);
        break;

      case CMD_INIT_MODULE:
        if (length < 1) return; //at least needs a targeted address

        sendCmd(Serial2, CMD_INIT_MODULE, buffer, length); //pass first

        for (int i = 0; i < length; i++) {
          if (buffer[i] != module_status.addr) continue; //not my turn

          EEPROM.put(MODULE_CONFIG_EEPROM_ADDR, module_config);

          turnSwitch(module_config.switchState);
          module_status.completeInit = true;

#if DEBUG
          Serial.print("[UART] Module id: ");
          Serial.println(module_config.id);
#endif

          led.setOnSingle();
        }
        break;
    }
  }
}

void receiveSerial2() {
  if (!module_status.initialized) return;

  static CMD_STATE state = RC_NONE;

  static char cmd;
  static int length;
  static int buffer_pos;
  static char buffer[96];

  if (Serial2.available()) {
    switch (receiveCmd(Serial2, state, cmd, length, buffer_pos, buffer)) {
      case CMD_REQ_ADR:
        sendAddress(Serial2);
        break;

      case CMD_UPDATE_MASTER:
        sendCmd(Serial1, CMD_UPDATE_MASTER, buffer, length);
        break;

      case CMD_LINK_MODULE:
        sendCmd(Serial1, CMD_LINK_MODULE, buffer, length); //pass first
        DeserializationError err = deserializeJson(data, buffer);

        if (err == DeserializationError::Ok) {
          if (data["id"].as<int>() == module_config.id) module_config.id = random(1000, 9999); //conflict id
          if (data["addr"].as<int>() - 1 != module_status.addr) return; //not my turn yet

          data["addr"] = module_status.addr;
          data["id"] = module_config.id;
          data["switch_state"] = (int)module_config.switchState;
          data["name"] = module_config.name;

          int l = measureJson(data);
          char *p = (char*)malloc(l * sizeof(char));
          serializeJson(data, p, l);
          sendCmd(Serial1, CMD_LINK_MODULE, p, l);
          free(p);
        } else {
          sendAddress(Serial2);
        }
        break;
    }
  }
}

void sendAddress(Stream &_serial) {
  char p[1] = {module_status.addr};
  sendCmd(_serial, CMD_LOAD_MODULE, p, sizeof(p)); //pass self-addr

#if DEBUG
  Serial.println("[UART] Sending address");
#endif
}

void sendUpdateMaster(Stream &_serial, char type, int value) {
  char p[3] = {module_status.addr, type, value};
  sendCmd(_serial, CMD_UPDATE_MASTER, p, sizeof(p));
}

/*** Util ***/
char receiveCmd(Stream &_serial, CMD_STATE &state, char &cmd, int &length, int &buffer_pos, char *buffer) {
  Stream* serial = &_serial;

  switch (state) {
    case RC_NONE: {
        cmd = CMD_FAIL;

        if (serial->available() < 1 || (uint8_t)serial->read() != CMD_START) break;

        state = RC_HEADER;
      }

    case RC_HEADER: {
        if (serial->available() < 3) break;

        cmd = serial->read();

        length = serial->read();
        length |= (uint16_t) serial->read() << 8;

#if DEBUG
        Serial.print("[UART] Content length: ");
        Serial.println(length);
#endif

        buffer_pos = 0;

        state = RC_PAYLOAD;
      }

    case RC_PAYLOAD: {
        if (buffer_pos < length && serial->available()) {
          buffer[buffer_pos++] = serial->read();
        }

        if (buffer_pos < length) break;

        state = RC_CHECK;
      }

    case RC_CHECK: {
        if (serial->available() < 2) break;

        uint8_t checksum = serial->read();

        uint8_t eof = serial->read();

        if (eof != CMD_EOF) {
#if DEBUG
          Serial.print("[UART] ERROR: Unexpected EOF: ");
          Serial.println(eof, HEX);
#endif
          state = RC_NONE;
          break;
        }

        state = RC_NONE;

        return cmd;
      }
  }

  return CMD_FAIL;
}
/*char receiveCmd(Stream &_serial) {
  Stream* serial = &_serial;

  uint8_t cb = serialRead(_serial);

  if (cb == CMD_START) {
    char cmd = serialRead(_serial);

    uint16_t length = serialRead(_serial);
    length = length | serialRead(_serial) << 8;

  #if DEBUG
    Serial.print("[UART] Content length: ");
    Serial.println(length);
  #endif

    if (length > sizeof(buffer)) { //oversize
      length = 0;
      while (serialRead(_serial) != CMD_EOF);
      return CMD_FAIL;
    }

    int buf_count = 0;

    while (buf_count != length) {
      buffer[buf_count] = serialRead(_serial);
      buf_count++;
    }

    uint8_t checksum = serialRead(_serial); //checksum

    if (serialRead(_serial) != CMD_EOF && calcCRC(buffer, length) != checksum) return CMD_FAIL; //error

    return cmd;
  }

  return CMD_FAIL;
  }*/

void sendCmd(Stream &_serial, char cmd, char* payload, int length) {
  Stream* serial = &_serial;
  char buf[6 + length]; //start, data_length, data, checksum, stop

  buf[0] = CMD_START; //start_byte
  buf[1] = cmd; //cmd_byte
  buf[2] = length & 0xff; //data_length - low byte
  buf[3] = (length >> 8) & 0xff; //data_length - high byte
  buf[4 + length] = calcCRC(payload, length); //checksum
  buf[5 + length] = CMD_EOF; //stop_byte

  for (int i = 0; i < length; i++) //load buf
    buf[4 + i] = payload[i];

  for (int i = 0; i < sizeof(buf); i++)
    serial->print(buf[i]);
}
void sendCmd(Stream &_serial, char cmd) {
  char *p = 0x00;
  sendCmd(_serial, cmd, p, 0);
}

char serialRead(Stream &_serial) {
  Stream* serial = &_serial;

  while (!serial->available())
    delay(1);

  return (uint8_t)serial->read();
}

void clearSerial(Stream &_serial) {
  Stream* serial = &_serial;

  while (serial->available() > 0) serial->read();
}
