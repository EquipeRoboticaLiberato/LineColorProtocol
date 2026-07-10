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

  // Primeiro le o snapshot principal do escravo.
  sensor.readLineAndColor();

  if (!LineColorProtocolView::connectionAvailable(Serial, sensor, F("Snapshot"))) {
    delay(100);
    return;
  }

  // Depois le a representacao de cor escolhida para a visualizacao.
  if (!LineColorProtocolView::readColorDetails(sensor, modoCor)) {
    (void)LineColorProtocolView::connectionAvailable(Serial, sensor, F("Cor"));
    delay(100);
    return;
  }

  if (!LineColorProtocolView::connectionAvailable(Serial, sensor, F("Cor"))) {
    delay(100);
    return;
  }

  // Por fim, imprime os valores que ja foram lidos.
  LineColorProtocolView::printLineAndColor(Serial, sensor, modoCor);
  delay(50);
}
