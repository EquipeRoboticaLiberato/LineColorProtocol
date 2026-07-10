#include <Wire.h>
#include <LineColorProtocol.h>
#include <LineColorProtocolView.h>

ColorSensorI2C sensor(SLAVE_ADDRESS, &Wire);
LineColorProtocolView::ColorViewMode modoCor = LineColorProtocolView::COLOR_VIEW_RAW_RGBW;

void setup() {
  Serial.begin(115200);
  while (!Serial) {}

  sensor.begin(100000UL);
  sensor.setThreshold(950);
  sensor.setStalenessTimeout(300);

  Serial.println(F("Exemplo 03 - Snapshot Linha+Cor"));
  Serial.println(F("r RAW | l calibracao | c corrigido | g RGB | h HSV"));
}

void loop() {
  LineColorProtocolView::updateColorViewMode(Serial, modoCor);
  LineColorProtocolView::readAndPrintLineAndColor(Serial, sensor, modoCor);
  delay(50);
}
