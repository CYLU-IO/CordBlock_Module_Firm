void clearEEPROM() {
  for (int i = 0 ; i < EEPROM.length() ; i++) EEPROM.write(i, 0);
}

void saveInEEPROM(char add, String data) {
  int _size = data.length();
  int i;

  for (i = 0; i < _size; i++) EEPROM.write(add + i, data[i]);

  EEPROM.write(add + _size, '\0'); //Add termination null character for String Data
}


String readFromEEPROM(char add) {
  int i;
  char data[100]; //Max 100 Bytes
  int len = 0;
  unsigned char k;

  k = EEPROM.read(add);

  while (k != '\0' && len < 500) { //Read until null character
    k = EEPROM.read(add + len);
    data[len] = k;
    len++;
  }

  data[len] = '\0';

  return String(data);
}
