#include <Wire.h>
#include <LineColorProtocol.h>

ColorSensorI2C sensor(SLAVE_ADDRESS, &Wire);

void printStats() {
  if (!sensor.readStats()) {
    Serial.print(F("Falha em READ_STATS. lastError="));
    Serial.println(sensor.getLastError());
    return;
  }

  Serial.println(F("--- Stats remotos ---"));
  const LineColorRemoteStats &stats = sensor.getStats();
  Serial.print(F("Uptime(ms): ")); Serial.println(stats.uptimeMs);
  Serial.print(F("RX CRC err: ")); Serial.println(stats.rxCrcErrors);
  Serial.print(F("RX frame err: ")); Serial.println(stats.rxFrameErrors);
  Serial.print(F("RX unknown cmd: ")); Serial.println(stats.rxUnknownCommands);
  Serial.print(F("TX responses: ")); Serial.println(stats.txResponses);
  Serial.print(F("QTR calib count: ")); Serial.println(stats.qtrCalibrationCount);
  Serial.print(F("Color calib count: ")); Serial.println(stats.colorCalibrationCount);
  Serial.print(F("EEPROM writes: ")); Serial.println(stats.eepromWriteCount);
  Serial.print(F("Last EEPROM write(ms): ")); Serial.println(stats.lastEepromWriteMs);
  Serial.print(F("Unlock restante(ms): ")); Serial.println(stats.eepromUnlockRemainingMs);
}

void setup() {
  Serial.begin(115200);
  while (!Serial) {}

  sensor.begin(100000UL);
  Serial.println(F("=== Exemplo 04: Calibracao Segura + Estatisticas ==="));
  Serial.println(F("Comandos via Serial:"));
  Serial.println(F("  s -> ler stats"));
  Serial.println(F("  l -> unlock + lineCalibrate (mova o sensor por ~5s)"));
  Serial.println(F("  c -> unlock + colorCalibrate (leva ~10s no firmware atual)"));
}

void loop() {
  if (!Serial.available()) return;

  char ch = (char)Serial.read();
  if (ch == 's') {
    printStats();
  } else if (ch == 'l') {
    Serial.println(F("Armando janela de EEPROM..."));
    if (!sensor.armEepromWrite()) {
      Serial.print(F("Falhou unlock. lastError="));
      Serial.print(sensor.getLastError());
      Serial.print(F(" ack="));
      Serial.print(sensor.getLastAckStatus());
      Serial.print(F(" ("));
      Serial.print(LineColorProtocol::ackStatusName(sensor.getLastAckStatus()));
      Serial.println(F(")"));
      return;
    }

    Serial.println(F("Enviando lineCalibrate..."));
    sensor.lineCalibrate();
    Serial.print(F("ACK status lineCalibrate: "));
    Serial.print(sensor.getLastAckStatus());
    Serial.print(F(" ("));
    Serial.print(LineColorProtocol::ackStatusName(sensor.getLastAckStatus()));
    Serial.println(F(")"));
  } else if (ch == 'c') {
    Serial.println(F("Armando janela de EEPROM..."));
    if (!sensor.armEepromWrite()) {
      Serial.print(F("Falhou unlock. lastError="));
      Serial.print(sensor.getLastError());
      Serial.print(F(" ack="));
      Serial.print(sensor.getLastAckStatus());
      Serial.print(F(" ("));
      Serial.print(LineColorProtocol::ackStatusName(sensor.getLastAckStatus()));
      Serial.println(F(")"));
      return;
    }

    Serial.println(F("Enviando colorCalibrate..."));
    sensor.colorCalibrate();
    Serial.print(F("ACK status colorCalibrate: "));
    Serial.print(sensor.getLastAckStatus());
    Serial.print(F(" ("));
    Serial.print(LineColorProtocol::ackStatusName(sensor.getLastAckStatus()));
    Serial.println(F(")"));
  }
}
