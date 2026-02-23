#define LINECOLORPROTOCOL_NO_I2C_CLIENT
#include <LineColorProtocol.h>

using namespace LineColorProtocol;

void printFrame(const uint8_t *data, uint8_t len) {
  for (uint8_t i = 0; i < len; i++) {
    if (data[i] < 16) Serial.print('0');
    Serial.print(data[i], HEX);
    if (i + 1 < len) Serial.print(' ');
  }
  Serial.println();
}

void setup() {
  Serial.begin(115200);

  uint8_t frame[MAX_FRAME_BUFFER] = {0};
  uint8_t len = 0;

  frame[len++] = READ_DEVICE_INFO;
  frame[len++] = 0x01; // sequence id
  appendCRC(frame, len);

  Serial.println(F("=== Exemplo 01: CRC-8 Basico ==="));
  Serial.print(F("Frame request: "));
  printFrame(frame, len);
  Serial.print(F("CRC valido? "));
  Serial.println(frameCRCValid(frame, len) ? F("SIM") : F("NAO"));
}

void loop() {}
