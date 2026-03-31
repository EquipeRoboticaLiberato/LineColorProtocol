#include <Wire.h>
#include <LineColorProtocol.h>

ColorSensorI2C sensor(SLAVE_ADDRESS, &Wire);

const __FlashStringHelper* nomeStatus(ColorSensorI2C::ConnectionStatus status) {
  switch (status) {
    case ColorSensorI2C::DISCONNECTED: return F("DISCONNECTED");
    case ColorSensorI2C::CONNECTED: return F("CONNECTED");
    case ColorSensorI2C::LOST_CONNECTION: return F("LOST_CONNECTION");
    default: return F("UNKNOWN");
  }
}

bool conexaoDisponivel(const __FlashStringHelper* etapa) {
  ColorSensorI2C::ConnectionStatus currentStatus = sensor.status();
  if (currentStatus == ColorSensorI2C::CONNECTED) return true;

  Serial.print(etapa);
  Serial.print(F(" status="));
  Serial.print(nomeStatus(currentStatus));
  Serial.print(F(" erro="));
  Serial.println(sensor.getLastError());
  return false;
}

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

  if (!conexaoDisponivel(F("Linha calibrada"))) {
    delay(100);
    return;
  }

  sensor.readLineRaw();

  if (!conexaoDisponivel(F("Linha RAW"))) {
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
