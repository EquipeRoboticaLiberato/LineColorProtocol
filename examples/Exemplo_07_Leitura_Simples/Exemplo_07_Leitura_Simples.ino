#include <Wire.h>
#include <LineColorProtocol.h>
#include <LineColorProtocolView.h>

ColorSensorI2C sensor(SLAVE_ADDRESS, &Wire);

void setup() {
  Serial.begin(115200);
  while (!Serial) {}

  sensor.begin(100000UL);
  sensor.setThreshold(950);
}

void loop() {
  LineColorProtocolView::readAndPrintLineAndColor(
      Serial,
      sensor,
      LineColorProtocolView::COLOR_VIEW_HSV);

  delay(100);
}
