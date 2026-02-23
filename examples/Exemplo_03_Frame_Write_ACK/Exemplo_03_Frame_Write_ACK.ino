#define LINECOLORPROTOCOL_NO_I2C_CLIENT
#include <LineColorProtocol.h>

using namespace LineColorProtocol;

void printHex(const uint8_t *data, uint8_t len) {
  for (uint8_t i = 0; i < len; i++) {
    if (data[i] < 16) Serial.print('0');
    Serial.print(data[i], HEX);
    Serial.print(' ');
  }
  Serial.println();
}

void setup() {
  Serial.begin(115200);

  const uint8_t seq = 0x2A;
  const uint16_t threshold = 950;

  uint8_t request[MAX_FRAME_BUFFER] = {0};
  uint8_t reqLen = 0;
  request[reqLen++] = SET_THRESHOLD;
  request[reqLen++] = seq;
  appendU16BE(request, reqLen, threshold);
  appendCRC(request, reqLen);

  uint8_t ack[MAX_FRAME_BUFFER] = {0};
  uint8_t ackLen = 0;
  ack[ackLen++] = seq;
  ack[ackLen++] = SET_THRESHOLD;
  ack[ackLen++] = ACK_OK;
  appendCRC(ack, ackLen);

  Serial.println(F("=== Exemplo 03: Frame de Write + ACK ==="));
  Serial.print(F("Request SET_THRESHOLD: "));
  printHex(request, reqLen);
  Serial.print(F("ACK: "));
  printHex(ack, ackLen);

  Serial.print(F("ACK valido? "));
  Serial.println(frameCRCValid(ack, ackLen) ? F("SIM") : F("NAO"));
  Serial.print(F("Status ACK: "));
  Serial.println(ackStatusName(ack[2]));
}

void loop() {}
