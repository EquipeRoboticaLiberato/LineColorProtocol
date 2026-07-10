#include <Wire.h>
#include <LineColorProtocol.h>
#include <LineColorProtocolView.h>

ColorSensorI2C sensor(SLAVE_ADDRESS, &Wire);

void mostrarLeitura(LineColorProtocolView::ColorViewMode modo) {
  for (uint8_t i = 0; i < 8; i++) {
    LineColorProtocolView::readAndPrintLineAndColor(Serial, sensor, modo);
    delay(150);
  }
}

void setup() {
  Serial.begin(115200);
  while (!Serial) {}

  sensor.begin(100000UL);
  sensor.setThreshold(950);
  sensor.setStalenessTimeout(300);

  Serial.println(F("Exemplo 04 - Calibracao + leitura"));
  Serial.println(F("s stats | l calibra linha | c calibra cor | v snapshot"));
}

void loop() {
  if (!Serial.available()) return;

  char ch = (char)Serial.read();

  if (ch == 's') {
    LineColorProtocolView::readAndPrintStats(Serial, sensor);
  }

  if (ch == 'v') {
    mostrarLeitura(LineColorProtocolView::COLOR_VIEW_HSV);
  }

  if (ch == 'l') {
    Serial.println(F("Linha: mova o sensor por 5s"));
    if (sensor.lineCalibrateAndWait()) {
      Serial.println(F("Linha OK"));
      mostrarLeitura(LineColorProtocolView::COLOR_VIEW_RAW_RGBW);
    } else {
      LineColorProtocolView::printAckError(Serial, sensor);
    }
  }

  if (ch == 'c') {
    Serial.println(F("Cor: apresente as referencias por 10s"));
    if (sensor.colorCalibrateAndWait()) {
      Serial.println(F("Cor OK"));
      mostrarLeitura(LineColorProtocolView::COLOR_VIEW_CALIBRATION);
      mostrarLeitura(LineColorProtocolView::COLOR_VIEW_HSV);
    } else {
      LineColorProtocolView::printAckError(Serial, sensor);
    }
  }
}
