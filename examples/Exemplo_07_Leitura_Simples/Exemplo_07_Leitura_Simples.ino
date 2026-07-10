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
  // Este helper imprime linha + cor em HSV com pouco codigo no sketch.
  LineColorProtocolView::readAndPrintLineAndColor(
      Serial,
      sensor,
      LineColorProtocolView::COLOR_VIEW_HSV);

  delay(100);
}
