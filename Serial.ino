StaticJsonDocument<96> data;
int cmdLength;
char cmdBuf[96];

void serialInit() {
  Serial1.swap(0);
  Serial1.begin(9600);

  Serial1.swap(0);
  Serial2.begin(9600);
}

void establishContact() {
  clearSerial(Serial1);
  clearSerial(Serial2);

  while (Serial1.available() <= 0) {
    sendCmd(Serial1, CMD_REQ_ADR);
    led.blinkSingle(300);
    delay(300);
  }
}

void receiveSerial1() {
  if (Serial1.available()) {
    char cmd = receiveCmd(Serial1);

    switch (cmd) {
      case CMD_LOAD_MODULE:
        led.setOffSingle();

        if (cmdLength < 1) return;

        sendCmd(Serial2, CMD_HI);
        module_status.addr = (int)cmdBuf[0] + 1; //update self-addr as the increasement of the previous addr

        if (!module_config.initialized) {
          module_config.id = random(1000, 9999);
          strcpy(module_config.name, (String("Switch ") + String(module_status.addr)).c_str());
          module_config.initialized = true;
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
        break;

      case CMD_HI:
        sendCmd(Serial1, CMD_HI);
        break;

      case CMD_INIT_MODULE:
        if (cmdLength < 1) return; //at least needs a targeted address

        sendCmd(Serial2, CMD_INIT_MODULE, cmdBuf, cmdLength); //pass first

        for (int i = 0; i < cmdLength; i++) {
          if (cmdBuf[i] != module_status.addr) continue; //not my turn

          //eepromFormat(MODULE_CONFIG_EEPROM_ADDR, module_config);
          EEPROM.put(MODULE_CONFIG_EEPROM_ADDR, module_config);

          turnSwitch(module_config.switchState);
          module_status.completeInit = true;

          led.setOnSingle();
        }
        break;

      case CMD_DO_MODULE:
        if (cmdLength < 2) return; //at least needs a targeted address

        sendCmd(Serial2, CMD_DO_MODULE, cmdBuf, cmdLength); //pass first

        for (int i = 1; i < cmdLength; i++) {
          if (cmdBuf[i] != module_status.addr) continue; //not my turn

          switch ((uint8_t)cmdBuf[0]) { //actions
            case DO_TURN_ON:
              turnSwitch(HIGH);
              break;

            case DO_TURN_OFF:
              turnSwitch(LOW);
              break;
          }
        }
        break;
    }
  }
}

void receiveSerial2() {
  if (!module_status.initialized) return;

  if (Serial2.available()) {
    char cmd = receiveCmd(Serial2);

    switch (cmd) {
      case CMD_REQ_ADR:
        sendAddress(Serial2);
        break;

      case CMD_UPDATE_MASTER:
        sendCmd(Serial1, CMD_UPDATE_MASTER, cmdBuf, cmdLength);
        break;

      case CMD_LINK_MODULE:
        sendCmd(Serial1, CMD_LINK_MODULE, cmdBuf, cmdLength); //pass first
        DeserializationError err = deserializeJson(data, cmdBuf);

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
}

void sendUpdateMaster(Stream &_serial, char type, int value) {
  char p[3] = {module_status.addr, type, value};
  sendCmd(_serial, CMD_UPDATE_MASTER, p, sizeof(p));
}

/*** Util ***/
char receiveCmd(Stream &_serial) {
  Stream* serial = &_serial;

  uint8_t cb = serialRead(_serial);

  if (cb == CMD_START) {
    char cmd = serialRead(_serial); //cmd_byte

    uint16_t length = serialRead(_serial);
    cmdLength = length | serialRead(_serial) << 8;

    if (cmdLength > sizeof(cmdBuf)) { //oversize
      eraseCmdBuf();
      while (serialRead(_serial) != CMD_EOF);
      return CMD_FAIL;
    }

    int buf_count = 0;
    if (length > 0) eraseCmdBuf();

    while (buf_count != cmdLength) {
      cmdBuf[buf_count] = serialRead(_serial);
      buf_count++;
    }

    uint8_t checksum = serialRead(_serial); //checksum

    if (serialRead(_serial) != CMD_EOF && calcCRC(cmdBuf, cmdLength) != checksum) return CMD_FAIL; //error

    return cmd;
  }

  return CMD_FAIL;
}

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

void eraseCmdBuf() {
  for (int i = 0; i < sizeof(cmdBuf); i++) cmdBuf[i] = 0x00;
}

void clearSerial(Stream &_serial) {
  Stream* serial = &_serial;

  while (serial->available() > 0) serial->read();
}
