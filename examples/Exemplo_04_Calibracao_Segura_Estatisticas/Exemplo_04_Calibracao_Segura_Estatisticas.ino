#include <Wire.h>
#include <LineColorProtocol.h>
#include <LineColorProtocolView.h>

ColorSensorI2C sensor(SLAVE_ADDRESS, &Wire);

// O exemplo fica mostrando snapshots; eles param apenas durante a calibracao.
LineColorProtocolView::ColorViewMode modoCor = LineColorProtocolView::COLOR_VIEW_HSV;
const unsigned long INTERVALO_SNAPSHOT_MS = 150;
unsigned long ultimoSnapshotMs = 0;

void setup() {
  Serial.begin(115200);
  while (!Serial) {}

  sensor.begin(100000UL);
  sensor.setThreshold(950);
  sensor.setStalenessTimeout(300);

  Serial.println(F("Exemplo 04 - Calibracao + Snapshot"));
  Serial.println(F("1 calibra linha | 2 calibra cor | s stats"));
  LineColorProtocolView::printColorViewHelp(Serial);
}

void loop() {
  while (Serial.available()) {
    char ch = (char)Serial.read();

    if (ch == '1') {
      Serial.println(F("Linha: mova o sensor por 5s"));

      // lineCalibrateAndWait() arma a EEPROM, pede a calibracao e espera terminar.
      if (sensor.lineCalibrateAndWait()) {
        Serial.println(F("Linha OK - snapshots retomados"));
      } else {
        LineColorProtocolView::printAckError(Serial, sensor);
      }
      ultimoSnapshotMs = millis();
    } else if (ch == '2') {
      Serial.println(F("Cor: apresente as referencias por 10s"));

      // Durante este bloqueio o exemplo deixa de imprimir snapshots.
      if (sensor.colorCalibrateAndWait()) {
        Serial.println(F("Cor OK - snapshots retomados"));
      } else {
        LineColorProtocolView::printAckError(Serial, sensor);
      }
      ultimoSnapshotMs = millis();
    } else if (ch == 's' || ch == 'S') {
      // Estatisticas ajudam a diagnosticar CRC, ACK, escritas e busy remoto.
      LineColorProtocolView::readAndPrintStats(Serial, sensor);
    } else if (LineColorProtocolView::setColorViewMode(ch, modoCor)) {
      Serial.print(F("Modo de cor alterado: "));
      Serial.println(ch);
    }
  }

  if (millis() - ultimoSnapshotMs >= INTERVALO_SNAPSHOT_MS) {
    ultimoSnapshotMs = millis();
    LineColorProtocolView::readAndPrintLineAndColor(Serial, sensor, modoCor);
  }
}
