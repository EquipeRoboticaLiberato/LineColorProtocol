#include <Wire.h>
#include <LineColorProtocol.h>
#include <LineColorProtocolView.h>

ColorSensorI2C sensor(SLAVE_ADDRESS, &Wire);

void setup() {
  Serial.begin(115200);
  while (!Serial) {}

  // Para uma demonstracao simples, basta iniciar I2C e ajustar o limiar da linha.
  sensor.begin(100000UL);
  sensor.setThreshold(950);
}

void loop() {
  // Primeiro le linha + cor do modulo escravo.
  sensor.readLineAndColor();

  if (!LineColorProtocolView::connectionAvailable(Serial, sensor, F("Snapshot"))) {
    delay(100);
    return;
  }

  // Depois le o HSV, que e a visualizacao escolhida para este exemplo simples.
  if (!LineColorProtocolView::readColorDetails(sensor, LineColorProtocolView::COLOR_VIEW_HSV)) {
    (void)LineColorProtocolView::connectionAvailable(Serial, sensor, F("HSV"));
    delay(100);
    return;
  }

  if (!LineColorProtocolView::connectionAvailable(Serial, sensor, F("HSV"))) {
    delay(100);
    return;
  }

  // Por fim, imprime os valores ja atualizados.
  LineColorProtocolView::printLineAndColor(Serial, sensor, LineColorProtocolView::COLOR_VIEW_HSV);

  delay(100);
}
