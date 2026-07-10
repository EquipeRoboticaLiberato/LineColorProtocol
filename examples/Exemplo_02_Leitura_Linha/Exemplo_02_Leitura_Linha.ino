#include <Wire.h>
#include <LineColorProtocol.h>
#include <LineColorProtocolView.h>

ColorSensorI2C sensor(SLAVE_ADDRESS, &Wire);

void setup() {
  Serial.begin(115200);
  while (!Serial) {}

  sensor.begin(100000UL);
  sensor.setThreshold(950);
  sensor.setStalenessTimeout(300);

  Serial.println(F("=== Exemplo 02: Leitura de Linha ==="));
  Serial.print(F("Comando snapshot usado: "));
  Serial.println(LineColorProtocol::commandName(LineColorProtocol::READ_LINE_SNAPSHOT));
}

void loop() {
  sensor.readLine();

  if (!LineColorProtocolView::connectionAvailable(Serial, sensor, F("Linha calibrada"))) {
    delay(100);
    return;
  }

  sensor.readLineRaw();

  if (!LineColorProtocolView::connectionAvailable(Serial, sensor, F("Linha RAW"))) {
    delay(100);
    return;
  }

  Serial.print(F("Pos: "));
  Serial.print('\t');
  Serial.print(sensor.getPosition());
  Serial.print('\t');

  Serial.print(F(" | Bool: 0b"));
  LineColorProtocol::printSensorBoolean(Serial, sensor.getBoolean());
  
  Serial.print(F(" | Sensores: "));
  for (uint8_t i = 0; i < sensor.getSensorCount(); i++) {
    Serial.print(sensor.getSingleSensor(i));
    if (i + 1 < sensor.getSensorCount()) Serial.print('\t');
  }

  Serial.print(F(" | RAW: "));
  for (uint8_t i = 0; i < sensor.getSensorCount(); i++) {
    Serial.print(sensor.getSingleSensorRaw(i));
    if (i + 1 < sensor.getSensorCount()) Serial.print('\t');
  }
  Serial.println();

  delay(50);
}
