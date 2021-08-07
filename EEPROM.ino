void eepromInit() {
  EEPROM.get(MODULE_CONFIG_EEPROM_ADDR, module_config);
  
  if (module_config.initialized != 0x01) {
    eepromFormat(MODULE_CONFIG_EEPROM_ADDR, module_config);
#if DEBUG
    Serial.println("[EEPROM] Formating");
#endif
  }
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
