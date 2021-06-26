/*** EEPROM Address ***/
#define  RANDOMSEED_EEPROM_ADDR      0x00
#define  MODULE_CONFIG_EEPROM_ADDR   0x10

void eepromInit() {
  EEPROM.get(MODULE_CONFIG_EEPROM_ADDR, module_config);

  if (module_config.initialized != 0x01) {
    eepromFormat(MODULE_CONFIG_EEPROM_ADDR, module_config);
#if DEBUG
    Serial.println("[EEPROM] Formatiing");
#endif
  }

  reseedRandom((uint32_t*)(RANDOMSEED_EEPROM_ADDR));
}

template <class T>
int eepromFormat(int ee, T& value) {
  for (int i = 0; i < sizeof(value); i++) EEPROM.write(ee++, 0xFF);

  return ee + sizeof(value);
}

template <class T>
int eepromUpdate(int ee, T& value) {
  const byte* p = (const byte*)(const void*)&value;

  for (int i = 0; i < sizeof(value); i++) EEPROM.update(ee++, *p++);

  return ee + sizeof(value);
}

/*** Randomseed ***/
void reseedRandom(uint32_t* address) {
  static const uint32_t HappyPrime = 127807 /*937*/;
  uint32_t raw;
  unsigned long seed;

  raw = EEPROM.read(address);

  do {
    raw += HappyPrime;
    seed = raw & 0x7FFFFFFF;
  } while ((seed < 1) || (seed > 2147483646));

  srandom(seed);
  EEPROM.write(address, raw);
}

void reseedRandomInit(uint32_t* address, uint32_t value) {
  //reseedRandomInit((uint32_t*)(address), value);
  EEPROM.write(address, value);
}
