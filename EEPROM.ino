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

/*
   Random Seed
*/
void reseedRandom( uint32_t* address ) {
  static const uint32_t HappyPrime = 127807 /*937*/;
  uint32_t raw;
  unsigned long seed;

  raw = eeprom_read_dword( address );

  do{
    raw += HappyPrime;
    seed = raw & 0x7FFFFFFF;
  } while ( (seed < 1) || (seed > 2147483646) );

  srandom( seed );
  eeprom_write_dword( address, raw );
}

inline void reseedRandom( unsigned short address ) {
  reseedRandom( (uint32_t*)(address) );
}

void reseedRandomInit( uint32_t* address, uint32_t value ) {
  eeprom_write_dword( address, value );
}

inline void reseedRandomInit( unsigned short address, uint32_t value ) {
  reseedRandomInit( (uint32_t*)(address), value );
}
