#define LINECOLORPROTOCOL_NO_I2C_CLIENT
#include <LineColorProtocol.h>

using namespace LineColorProtocol;

void setup() {
  Serial.begin(115200);

  const uint8_t sensorCount = 6;
  const uint8_t seq = 0x10;
  uint8_t frame[MAX_FRAME_BUFFER] = {0};
  uint8_t len = 0;

  // Response frame format:
  // [seq][sensorCount][statusFlags][positionH][positionL][boolean][lineValues...][rightColor][rightRating][leftColor][leftRating][crc]
  frame[len++] = seq;
  frame[len++] = sensorCount;
  frame[len++] = protocolStatusFlags(true, true);
  appendU16BE(frame, len, 2450);
  frame[len++] = 0b00111100;

  for (uint8_t i = 0; i < sensorCount; i++) {
    appendU16BE(frame, len, (uint16_t)(100 + i * 150));
  }

  frame[len++] = 0; // right color (_RED em ColorSensorI2C)
  frame[len++] = 92;
  frame[len++] = 1; // left color (_GREEN)
  frame[len++] = 88;
  appendCRC(frame, len);

  Serial.println(F("=== Exemplo 04: Snapshot Linha+Cor ==="));
  Serial.print(F("Len esperado: "));
  Serial.println(expectedResponseLength(READ_LINE_COLOR_SNAPSHOT, sensorCount));
  Serial.print(F("Len real: "));
  Serial.println(len);
  Serial.print(F("CRC valido? "));
  Serial.println(frameCRCValid(frame, len) ? F("SIM") : F("NAO"));

  const uint8_t *payload = &frame[1];
  uint8_t parsedCount = payload[0];
  uint8_t flags = payload[1];
  uint16_t pos = readU16BE(&payload[2]);
  uint8_t bits = payload[4];

  Serial.print(F("sensorCount=")); Serial.println(parsedCount);
  Serial.print(F("flags=0x")); Serial.println(flags, HEX);
  Serial.print(F("pos=")); Serial.println(pos);
  Serial.print(F("boolean=0b"));
  printSensorBoolean(Serial, bits);
  Serial.println();

  for (uint8_t i = 0; i < parsedCount; i++) {
    uint8_t idx = 5 + (i * 2);
    Serial.print(F("S"));
    Serial.print(i);
    Serial.print(F("="));
    Serial.println(readU16BE(&payload[idx]));
  }
}

void loop() {}
