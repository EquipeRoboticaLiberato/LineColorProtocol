#define LINECOLORPROTOCOL_NO_I2C_CLIENT
#include <LineColorProtocol.h>

using namespace LineColorProtocol;

void printHex(const uint8_t *data, uint8_t len) {
  for (uint8_t i = 0; i < len; i++) {
    if (data[i] < 16) Serial.print('0');
    Serial.print(data[i], HEX);
    if (i + 1 < len) Serial.print(' ');
  }
  Serial.println();
}

void setup() {
  Serial.begin(115200);

  Serial.println(F("=== Exemplo 06: Protocolo Puro ==="));
  Serial.println(F("Este exemplo mostra o framing e os helpers internos do protocolo."));

  const uint8_t sensorCount = 6;

  uint8_t request[MAX_FRAME_BUFFER] = {0};
  uint8_t reqLen = 0;
  request[reqLen++] = READ_DEVICE_INFO;
  request[reqLen++] = 0x01;
  appendCRC(request, reqLen);

  Serial.print(F("READ_DEVICE_INFO request: "));
  printHex(request, reqLen);
  Serial.print(F("CRC valido? "));
  Serial.println(frameCRCValid(request, reqLen) ? F("SIM") : F("NAO"));
  Serial.println();

  Serial.println(F("Comandos atuais e tamanho esperado de resposta:"));
  const uint8_t cmds[] = {
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

  for (uint8_t i = 0; i < sizeof(cmds); i++) {
    Serial.print(F("- "));
    Serial.print(commandName(cmds[i]));
    Serial.print(F(" | tipo="));
    Serial.print(isWriteCommand(cmds[i]) ? F("WRITE") : F("READ"));
    Serial.print(F(" | respLen="));
    Serial.println(expectedResponseLength(cmds[i], sensorCount));
  }
  Serial.println();

  uint8_t ack[MAX_FRAME_BUFFER] = {0};
  uint8_t ackLen = 0;
  ack[ackLen++] = 0x2A;
  ack[ackLen++] = SET_THRESHOLD;
  ack[ackLen++] = ACK_OK;
  appendCRC(ack, ackLen);

  Serial.print(F("ACK SET_THRESHOLD: "));
  printHex(ack, ackLen);
  Serial.print(F("Status ACK: "));
  Serial.println(ackStatusName(ack[2]));
  Serial.println();

  uint8_t snapshot[MAX_FRAME_BUFFER] = {0};
  uint8_t snapLen = 0;
  snapshot[snapLen++] = 0x10;
  snapshot[snapLen++] = sensorCount;
  snapshot[snapLen++] = protocolStatusFlags(true, true);
  appendU16BE(snapshot, snapLen, 2450);
  snapshot[snapLen++] = 0b00111100;

  for (uint8_t i = 0; i < sensorCount; i++) {
    appendU16BE(snapshot, snapLen, (uint16_t)(100 + i * 150));
  }

  snapshot[snapLen++] = 0;
  snapshot[snapLen++] = 92;
  snapshot[snapLen++] = 1;
  snapshot[snapLen++] = 88;
  appendCRC(snapshot, snapLen);

  Serial.print(F("Snapshot linha+cor: "));
  printHex(snapshot, snapLen);
  Serial.print(F("Bool parseado: 0b"));
  printSensorBoolean(Serial, snapshot[5]);
  Serial.println();
  Serial.print(F("Posicao parseada: "));
  Serial.println(readU16BE(&snapshot[3]));
  Serial.println();

  uint8_t statsFrame[MAX_FRAME_BUFFER] = {0};
  uint8_t statsLen = 0;
  statsFrame[statsLen++] = 0x55;
  appendU32BE(statsFrame, statsLen, 123456UL);
  appendU16BE(statsFrame, statsLen, 1);
  appendU16BE(statsFrame, statsLen, 2);
  appendU16BE(statsFrame, statsLen, 0);
  appendU16BE(statsFrame, statsLen, 120);
  appendU16BE(statsFrame, statsLen, 3);
  appendU16BE(statsFrame, statsLen, 1);
  appendU16BE(statsFrame, statsLen, 4);
  appendU32BE(statsFrame, statsLen, 120000UL);
  appendU16BE(statsFrame, statsLen, 15000);
  appendCRC(statsFrame, statsLen);

  Serial.print(F("Frame READ_STATS valido? "));
  Serial.println(frameCRCValid(statsFrame, statsLen) ? F("SIM") : F("NAO"));
  Serial.print(F("Uptime parseado: "));
  Serial.println(readU32BE(&statsFrame[1]));
}

void loop() {}
