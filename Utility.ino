String int2str(int n, int leng) {
  String re = "";

  for (int i = leng - 1; i > 0; i--) {
    if (n / (int)(pow(10, i) + ((n % 9 == 0) ? 1 : 0)) == 0) {
      re += "0";
      continue;
    }

    break;
  }

  return re + String(n);
}

uint8_t calcCRC(char* str) {
  CRC32 crc;
  for (int i = 0; i < strlen(str); i++) crc.update(str[i]);

  return crc.finalize();
}
