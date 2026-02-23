#define LINECOLORPROTOCOL_NO_I2C_CLIENT
#include <LineColorProtocol.h>

using namespace LineColorProtocol;

void setup() {
  Serial.begin(115200);

  Serial.println(F("=== Exemplo 05: Stats + Capabilities ==="));

  uint32_t caps = CAPABILITIES_DEFAULT;
  Serial.print(F("Capabilities mask = 0x"));
  Serial.println(caps, HEX);

  const uint8_t sensorCount = 6;
  Serial.print(F("Resp len READ_STATS: "));
  Serial.println(expectedResponseLength(READ_STATS, sensorCount));
  Serial.print(F("Resp len READ_LINE_SNAPSHOT: "));
  Serial.println(expectedResponseLength(READ_LINE_SNAPSHOT, sensorCount));

  Serial.println(F("Comandos que exigem refresh em modo REQUEST:"));
  const uint8_t cmds[] = {
    READ_COLOR, READ_POSITION, READ_DEVICE_INFO, READ_STATS, READ_LINE_COLOR_SNAPSHOT
  };
  for (uint8_t i = 0; i < sizeof(cmds); i++) {
    Serial.print(F("- "));
    Serial.print(commandName(cmds[i]));
    Serial.print(F(": "));
    Serial.println(requiresRefreshInRequestMode(cmds[i]) ? F("SIM") : F("NAO"));
  }

  uint8_t statsFrame[MAX_FRAME_BUFFER] = {0};
  uint8_t len = 0;
  statsFrame[len++] = 0x55; // seq
  appendU32BE(statsFrame, len, 123456UL); // uptime
  appendU16BE(statsFrame, len, 1);  // rx crc
  appendU16BE(statsFrame, len, 2);  // rx frame
  appendU16BE(statsFrame, len, 0);  // rx unknown
  appendU16BE(statsFrame, len, 120); // tx responses
  appendU16BE(statsFrame, len, 3);  // qtr cal
  appendU16BE(statsFrame, len, 1);  // color cal
  appendU16BE(statsFrame, len, 4);  // eeprom writes
  appendU32BE(statsFrame, len, 120000UL); // last eeprom write ms
  appendU16BE(statsFrame, len, 15000); // unlock remaining
  appendCRC(statsFrame, len);

  Serial.print(F("Frame READ_STATS simulado valido? "));
  Serial.println(frameCRCValid(statsFrame, len) ? F("SIM") : F("NAO"));
  Serial.print(F("Uptime parseado: "));
  Serial.println(readU32BE(&statsFrame[1]));
}

void loop() {}
