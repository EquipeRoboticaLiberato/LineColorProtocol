#include <Wire.h>
#include <LineColorProtocol.h>
#include <LineColorProtocolView.h>

ColorSensorI2C sensor(SLAVE_ADDRESS, &Wire);

void setup() {
  Serial.begin(115200);
  while (!Serial) {}

  bool conectadoNoInicio = sensor.begin(100000UL);

  Serial.println(F("=== Exemplo 01: Status Basico ==="));
  Serial.print(F("Conectado no begin(): "));
  Serial.println(conectadoNoInicio ? F("sim") : F("nao"));
  Serial.println(F("Se o modulo estiver desligado, o loop segue tentando reconectar pelo status()."));
}

void loop() {
  ColorSensorI2C::ConnectionStatus currentStatus = sensor.status();

  Serial.print(F("Status: "));
  Serial.println(LineColorProtocolView::statusName(currentStatus));
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
