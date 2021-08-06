uint8_t calcCRC(uart_msg_pack* pack) {
  static CRC8 crc;

  crc.reset();
  crc.setPolynome(0x05);
  crc.add((uint8_t)pack->cmd);
  crc.add((uint8_t*)pack->payload, pack->length);

  return crc.getCRC();
}

uint8_t calcCRC(char* str, int length) {
  static CRC8 crc;

  crc.reset();
  crc.setPolynome(0x05);
  crc.add((uint8_t*)str, length);

  return crc.getCRC();
}

int bytesCombine(char low_byte, char high_byte) {
  return (low_byte & 0xff) | high_byte << 8;
}
