void i2cInit() {
  Wire.begin(0x01);
  Wire.onReceive(i2cReceiveCmd);
}

void i2cReceiveCmd(int length) {
  static char i2c_cmd;
  static int i2c_cmdLength = 0;
  static char i2c_cmdBuf[MAX_MODULES * 2]; //(addr, action) * MAX_MODULES

  while (Wire.available()) {
    /*** Receive CMD ***/
    uint8_t sb = Wire.read();

    if (sb == CMD_START) {
      Serial.println("Receivel action cmd in I2C");
      i2c_cmd = Wire.read(); //cmd byte
      i2c_cmdLength = Wire.read(); //legnth byte

      int buf_count = 0;

      if (i2c_cmdLength > 0) {
        for (int i = 0; i < sizeof(i2c_cmdBuf); i++) i2c_cmdBuf[i] = 0x00;
      }

      while (buf_count != i2c_cmdLength) {
        i2c_cmdBuf[buf_count] = Wire.read();
        buf_count++;
      }

      uint8_t checksum = Wire.read();

      if (Wire.read() != CMD_EOF && calcCRC(i2c_cmdBuf, i2c_cmdLength) != checksum) return; //error

      /*** Execute CMD ***/
      switch (i2c_cmd) {
        case CMD_DO_MODULE:
          if (i2c_cmdLength < 2) return; //at least needs a pair action

          for (int i = 0; i < i2c_cmdLength / 2; i++) {
            if (i2c_cmdBuf[i * 2] != module_status.addr) continue; //not my turn

            switch ((uint8_t)i2c_cmdBuf[i * 2 + 1]) { //actions
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

}
