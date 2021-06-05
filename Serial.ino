#define CMD_OK              0x10
#define CMD_FAIL            0x11
#define CMD_PASS_MSG        0x12
#define CMD_INSYNC          0x14
#define CMD_NOSYNC          0x15
#define CMD_EOF             0x20
#define CMD_REQ_ADR         0x41 //'A'
#define CMD_LOAD_MODULE     0x42 //'B'
#define CMD_CONFIRM_RECEIVE 0x43 //'C'
#define CMD_LINK_MODULE     0x4C //'L'
#define CMD_INIT_MODULE     0x49 //'I'
#define CMD_HI              0x52 //'H'
#define CMD_START           0xFF

StaticJsonDocument<96> data;
int cmdLength;
char cmdBuf[96];

void serialInit() {
  Serial.begin(9600);
  altSerial.begin(9600);
}

void establishContact() {
  while (Serial.available() <= 0) {
    sendCmd(Serial, CMD_REQ_ADR);

    digitalWrite(LED_PIN, HIGH);
    delay(50);
    digitalWrite(LED_PIN, LOW);
    delay(241);
  }
}

void receiveSerial() {
  if (Serial.available()) {
    char cmd = receiveCmd(Serial);

    switch (cmd) {
      case CMD_LOAD_MODULE:
        if (cmdLength == 0) return;

        sendCmd(altSerial, CMD_HI);
        module_status.addr = (int)cmdBuf[0] + 1; //update self-addr as the increasement of the previous addr

        if (!module_config.initialized) {
          module_config.id = random(1000, 9999);
          strcpy(module_config.name, (String("Switch ") + String(module_status.addr)).c_str());
        }

        delay(100);

        if (altSerial.available() == 0) { //last module
          module_status.lastModule = true;

          data["total"] = module_status.addr;
          data["addr"] = module_status.addr;
          data["id"] = module_config.id;
          data["switch_state"] = (int)module_config.switchState;
          data["name"] = module_config.name;

          int l = measureJson(data);
          char *p = (char*)malloc(l * sizeof(char));
          serializeJson(data, p, l);
          sendCmd(Serial, CMD_LINK_MODULE, p, l);
          free(p);
        } else {
          clearSerial(altSerial);
          sendAddress(altSerial);
        }

        module_status.initialized = true;
        eeprom_write(module_config_eeprom_address, module_config);
        break;

      case CMD_HI:
        sendCmd(Serial, CMD_HI);
        break;

      case CMD_INIT_MODULE:
        sendCmd(altSerial, CMD_INIT_MODULE); //pass to next module

        delay(100);

        i2cInit();
        turnSwitch(module_config.switchState);
        module_status.completeInit = true;
        digitalWrite(LED_PIN, HIGH);
        break;
    }
  }
}

void receiveAltserial() {
  if (!module_status.initialized) return;

  if (altSerial.available()) {
    char cmd = receiveCmd(altSerial);

    switch (cmd) {
      case CMD_REQ_ADR:
        if (module_status.completeInit) sendAddress(altSerial);
        digitalWrite(LED_PIN, LOW);
        break;

      case CMD_LINK_MODULE:
        sendCmd(Serial, CMD_LINK_MODULE, cmdBuf, cmdLength); //pass first

        if (receiveCmd(Serial) != CMD_CONFIRM_RECEIVE) return;

        DeserializationError err = deserializeJson(data, cmdBuf);

        if (err == DeserializationError::Ok) {
          if (data["addr"].as<int>() - 1 != module_status.addr) return; //not my turn yet

          data["addr"] = module_status.addr;
          data["id"] = module_config.id;
          data["switch_state"] = (int)module_config.switchState;
          data["name"] = module_config.name;

          int l = measureJson(data);
          char *p = (char*)malloc(l * sizeof(char));
          serializeJson(data, p, l);
          sendCmd(Serial, CMD_LINK_MODULE, p, l);
          free(p);
        }
        break;
    }
  }
}

/*** Util ***/
char receiveCmd(Stream &_serial) {
  Stream* serial = &_serial;

  if ((uint8_t)serialRead(_serial) == CMD_START) {
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

    char checksum = serialRead(_serial); //checksum, don't bother it yett

    if (serialRead(_serial) != CMD_EOF) return CMD_FAIL; //error

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
  buf[4 + length] = (char)calcCRC(payload); //checksum
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

void sendAddress(Stream &_serial) {
  char p[1] = {module_status.addr};
  sendCmd(_serial, CMD_LOAD_MODULE, p, sizeof(p)); //pass self-addr
}

void eraseCmdBuf() {
  for (int i = 0; i < sizeof(cmdBuf); i++) cmdBuf[i] = 0x00;
}

void clearSerial(Stream &_serial) {
  Stream* serial = &_serial;

  while (serial->available() > 0) serial->read();
}
