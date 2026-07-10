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
  // Primeiro le a linha calibrada, como nos sketches antigos do robo.
  sensor.readLine();

  if (!LineColorProtocolView::connectionAvailable(Serial, sensor, F("Linha calibrada"))) {
    delay(100);
    return;
  }

  // Depois le os valores RAW, uteis para diagnostico e calibracao.
  sensor.readLineRaw();

  if (!LineColorProtocolView::connectionAvailable(Serial, sensor, F("Linha RAW"))) {
    delay(100);
    return;
  }

  // A funcao de visualizacao apenas imprime o que ja foi lido acima.
  LineColorProtocolView::printLineValues(Serial, sensor);

  delay(50);
}
