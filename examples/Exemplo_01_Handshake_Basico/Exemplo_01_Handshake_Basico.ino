#include <Wire.h>
#include <LineColorProtocol.h>
#include <LineColorProtocolView.h>

ColorSensorI2C sensor(SLAVE_ADDRESS, &Wire);

void setup() {
  Serial.begin(115200);
  while (!Serial) {}

  // begin() inicia o barramento e retorna se o modulo respondeu no inicio.
  bool conectadoNoInicio = sensor.begin(100000UL);

  Serial.println(F("=== Exemplo 01: Status Basico ==="));
  Serial.print(F("Conectado no begin(): "));
  Serial.println(conectadoNoInicio ? F("sim") : F("nao"));
  Serial.println(F("Se estiver desligado, o loop continua tentando pelo status()."));
}

void loop() {
  // Para o robo, normalmente basta olhar se status() esta CONNECTED.
  ColorSensorI2C::ConnectionStatus currentStatus = sensor.status();

  // A funcao de visualizacao apenas imprime o status ja consultado acima.
  LineColorProtocolView::printBasicStatus(Serial, sensor, currentStatus);
  delay(1000);
}
