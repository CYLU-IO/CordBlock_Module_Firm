void i2cInit() {
  Wire.begin(0x01);
  Wire.onReceive(i2cReceiveCmd);
}

void i2cReceiveCmd() {
  static CMD_STATE state = RC_NONE;

  static char cmd;
  static int length = 0;
  static int buffer_pos;
  static char buffer[MAX_MODULES * 2]; //(addr, action) * MAX_MODULES

  switch (state) {
    case RC_NONE: {
        if (Wire.available() < 1) break;

        uint8_t start_byte = Wire.read();

        if (start_byte != CMD_START) {
          state = RC_NONE;
#if DEBUG
          Serial.print("[I2C] Incorrect start byte: ");
          Serial.println(start_byte, HEX);
#endif
          break;
        }

        state = RC_HEADER;
      }

    case RC_HEADER: {
        if (Wire.available() < 2) break;

        cmd = Wire.read();

        length = Wire.read();

        buffer_pos = 0;

        state = RC_PAYLOAD;
      }

    case RC_PAYLOAD: {
        if (buffer_pos < length && Wire.available()) {
          buffer[buffer_pos++] = Wire.read();
        }

        if (buffer_pos < length) break;

        state = RC_CHECK;
      }

    case RC_CHECK: {
        if (Wire.available() < 2) break;

        uint8_t checksum = Wire.read();
        uint8_t eof = Wire.read();

        state = RC_NONE;

        if (eof != CMD_EOF) {
#if DEBUG
          Serial.print("[I2C] ERROR: Unexpected EOF: ");
          Serial.println(eof, HEX);
#endif
        }

        switch (cmd) {
          case CMD_DO_MODULE:
            if (length < 2) return; //at least needs a pair action

            for (int i = 0; i < length / 2; i++) {
              if (buffer[i * 2] != module_status.addr) continue;

              module_status.controlTask = (uint8_t)buffer[i * 2 + 1];
              break;
            }
            break;
        }

        break;
      }
  }
}
