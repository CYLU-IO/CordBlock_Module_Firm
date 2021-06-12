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

uint8_t calcCRC(char* str, int length) {
  static CRC8 crc;

  crc.reset();
  crc.setPolynome(0x05);
  crc.add((uint8_t*)str, length);

  return crc.getCRC();
}

int largest(int arr[], int n) {
  int i;

  int max = arr[0];

  for (i = 1; i < n; i++)
    if (arr[i] > max)
      max = arr[i];

  return max;
}
