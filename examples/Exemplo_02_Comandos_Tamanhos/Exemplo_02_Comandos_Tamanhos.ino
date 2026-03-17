#define LINECOLORPROTOCOL_NO_I2C_CLIENT
#include <LineColorProtocol.h>

using namespace LineColorProtocol;

const uint8_t commands[] = {
  READ_COLOR,
  READ_RAW_RGBW,
  READ_IR_RAW,
  SET_THRESHOLD,
  READ_LINE_SNAPSHOT,
  READ_LINE_COLOR_SNAPSHOT,
  READ_DEVICE_INFO,
  READ_STATS,
  ARM_EEPROM_WRITE
};

void setup() {
  Serial.begin(115200);

  Serial.println(F("=== Exemplo 02: Comandos e Tamanhos ==="));
  Serial.println(F("cmd | nome | read/write | respLen (sensorCount=6)"));

  for (uint8_t i = 0; i < sizeof(commands); i++) {
    uint8_t cmd = commands[i];
    Serial.print(F("0x"));
    if (cmd < 16) Serial.print('0');
    Serial.print(cmd, HEX);
    Serial.print(F(" | "));
    Serial.print(commandName(cmd));
    Serial.print(F(" | "));
    Serial.print(isWriteCommand(cmd) ? F("WRITE") : F("READ"));
    Serial.print(F(" | "));
    Serial.println(expectedResponseLength(cmd, 6));
  }
}

void loop() {}
