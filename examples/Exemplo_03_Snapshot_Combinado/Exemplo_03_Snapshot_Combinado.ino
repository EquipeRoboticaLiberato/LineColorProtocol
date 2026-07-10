#include <Wire.h>
#include <LineColorProtocol.h>
#include <LineColorProtocolView.h>

ColorSensorI2C sensor(SLAVE_ADDRESS, &Wire);

// O modo muda o que sera impresso para cada sensor de cor.
LineColorProtocolView::ColorViewMode modoCor = LineColorProtocolView::COLOR_VIEW_RAW_RGBW;

void setup() {
  Serial.begin(115200);
  while (!Serial) {}

  sensor.begin(100000UL);
  sensor.setThreshold(950);
  sensor.setStalenessTimeout(300);

  Serial.println(F("Exemplo 03 - Snapshot Linha+Cor"));
  LineColorProtocolView::printColorViewHelp(Serial);
}

void loop() {
  // Digite r/l/c/g/h no Monitor Serial para trocar a visualizacao da cor.
  LineColorProtocolView::updateColorViewMode(Serial, modoCor);

  // Le linha e cor em uma rotina curta, pronta para demonstracao.
  LineColorProtocolView::readAndPrintLineAndColor(Serial, sensor, modoCor);
  delay(50);
}
