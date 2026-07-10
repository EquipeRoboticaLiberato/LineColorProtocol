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
      }
      else {
        LineColorProtocolView::printAckError(Serial, sensor);
      }
      ultimoSnapshotMs = millis();
    }
    else if (ch == '2') {
      Serial.println(F("Cor: apresente as referencias por 10s"));

      // Durante este bloqueio o exemplo deixa de imprimir snapshots.
      if (sensor.colorCalibrateAndWait()) {
        Serial.println(F("Cor OK - snapshots retomados"));
      }
      else {
        LineColorProtocolView::printAckError(Serial, sensor);
      }
      ultimoSnapshotMs = millis();
    }
    else if (ch == 's' || ch == 'S') {
      // Primeiro le as estatisticas; depois imprime os valores recebidos.
      if (sensor.readStats()) {
        LineColorProtocolView::printStats(Serial, sensor.getStats());
      }
      else {
        Serial.print(F("Falha READ_STATS erro="));
        Serial.println(sensor.getLastError());
      }
    }
    else if (LineColorProtocolView::setColorViewMode(ch, modoCor)) {
      Serial.print(F("Modo de cor alterado: "));
      Serial.println(ch);
    }
  }

  if (millis() - ultimoSnapshotMs >= INTERVALO_SNAPSHOT_MS) {
    ultimoSnapshotMs = millis();

    // Le o snapshot principal, depois a visualizacao de cor escolhida.
    sensor.readLineAndColor();

    if (!LineColorProtocolView::connectionAvailable(Serial, sensor, F("Snapshot"))) {
      return;
    }

    if (!LineColorProtocolView::readColorDetails(sensor, modoCor)) {
      (void)LineColorProtocolView::connectionAvailable(Serial, sensor, F("Cor"));
      return;
    }

    if (!LineColorProtocolView::connectionAvailable(Serial, sensor, F("Cor"))) {
      return;
    }

    // A impressao fica separada da leitura para ficar mais facil de estudar.
    LineColorProtocolView::printLineAndColor(Serial, sensor, modoCor);
  }
}
