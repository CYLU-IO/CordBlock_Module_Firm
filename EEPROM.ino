template <class T> int eeprom_write(int ee, const T& value) {
  const byte* p = (const byte*)(const void*)&value;
  int i;
  for (i = 0; i < sizeof(value); i++)
    EEPROM.write(ee++, *p++);
  return i;
}

template <class T> int eeprom_read(int ee, T& value) {
  byte* p = (byte*)(void*)&value;
  int i;
  for (i = 0; i < sizeof(value); i++)
    *p++ = EEPROM.read(ee++);
  return i;
}

void eeprom_erase() {
  uint16_t address = 1;
  uint8_t dump_row[16];

  while (address < 512) {
    memset(dump_row, 0x00, sizeof(dump_row));
    eeprom_write_block(dump_row, (void*)address, sizeof(dump_row));
    address += sizeof(dump_row);
  }
}

/*
   Random Seed
*/
void reseedRandom(uint32_t* address) {
  static const uint32_t HappyPrime = 127807 /*937*/;
  uint32_t raw;
  unsigned long seed;

  raw = eeprom_read_dword(address);

  do {
    raw += HappyPrime;
    seed = raw & 0x7FFFFFFF;
  } while ((seed < 1) || (seed > 2147483646));

  srandom(seed);
  eeprom_write_dword(address, raw);
}

inline void reseedRandom(unsigned short address) {
  reseedRandom((uint32_t*)(address));
}

void reseedRandomInit(uint32_t* address, uint32_t value) {
  eeprom_write_dword(address, value);
}

inline void reseedRandomInit(unsigned short address, uint32_t value) {
  reseedRandomInit((uint32_t*)(address), value);
}
