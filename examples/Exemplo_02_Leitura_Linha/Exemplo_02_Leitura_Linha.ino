#include <Wire.h>
#include <LineColorProtocol.h>
#include <LineColorProtocolView.h>

ColorSensorI2C sensor(SLAVE_ADDRESS, &Wire);

void setup() {
  Serial.begin(115200);
  while (!Serial) {}

  sensor.begin(100000UL);

  // Ajuste o limiar conforme a pista e a calibracao usada no modulo escravo.
  sensor.setThreshold(950);

  // Depois desse tempo sem resposta valida, status() passa a LOST_CONNECTION.
  sensor.setStalenessTimeout(300);

  Serial.println(F("=== Exemplo 02: Leitura de Linha ==="));
  Serial.print(F("Comando snapshot usado: "));
  Serial.println(LineColorProtocol::commandName(LineColorProtocol::READ_LINE_SNAPSHOT));
}

void loop() {
  // Este helper le a linha calibrada e tambem os valores RAW dos sensores.
  if (!LineColorProtocolView::readAndPrintLine(Serial, sensor)) {
    delay(100);
    return;
  }

  delay(50);
}
