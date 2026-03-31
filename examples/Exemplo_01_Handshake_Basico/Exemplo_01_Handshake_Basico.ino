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

void setup() {
  Serial.begin(115200);
  while (!Serial) {}

  sensor.begin(100000UL);

  Serial.println(F("=== Exemplo 01: Status Basico ==="));
  Serial.println(F("O handshake agora eh automatico no begin() e nas consultas de status()."));
}

void loop() {
  ColorSensorI2C::ConnectionStatus currentStatus = sensor.status();

  Serial.print(F("Status: "));
  Serial.println(nomeStatus(currentStatus));
  Serial.print(F("SensorCount remoto: "));
  Serial.println(sensor.getSensorCount());
  Serial.print(F("Versao protocolo remoto: "));
  Serial.println(sensor.getProtocolVersion());
  Serial.print(F("Versao protocolo atual (lib): "));
  Serial.println(LineColorProtocol::PROTOCOL_VERSION_CURRENT);
  Serial.print(F("Ultimo erro: "));
  Serial.println(sensor.getLastError());
  Serial.println();

  delay(1000);
}
