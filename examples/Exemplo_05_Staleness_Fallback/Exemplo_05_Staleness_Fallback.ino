#include <Wire.h>
#include <LineColorProtocol.h>
#include <LineColorProtocolView.h>

ColorSensorI2C sensor(SLAVE_ADDRESS, &Wire);

void setup() {
  Serial.begin(115200);
  while (!Serial) {}

  sensor.begin(100000UL);
  sensor.setThreshold(950);

  // Timeout curto para ficar facil testar: desligue o escravo e observe o fallback.
  sensor.setStalenessTimeout(250UL);

  Serial.println(F("=== Exemplo 05: Fallback por Status ==="));
  Serial.println(F("Desconecte o modulo I2C para simular falha e observar o fallback."));
}

void loop() {
  // Qualquer leitura valida renova o status interno da conexao.
  sensor.readLineAndColor();

  ColorSensorI2C::ConnectionStatus currentStatus = sensor.status();
  if (currentStatus != ColorSensorI2C::CONNECTED) {
    Serial.print(F("[FALLBACK] Status="));
    Serial.print(LineColorProtocolView::statusName(currentStatus));
    Serial.print(F(" lastError="));
    Serial.print(sensor.getLastError());
    Serial.print(F(" lastSuccess="));
    Serial.println(sensor.getLastSuccess());

    // Aqui, no robo real: parar motores / entrar em modo seguro.
    delay(100);
    return;
  }

  // Se chegou aqui, a leitura e recente o suficiente para usar no controle.
  Serial.print(F("OK pos="));
  Serial.print(sensor.getPosition());
  Serial.print(F(" bool=0b"));
  LineColorProtocol::printSensorBoolean(Serial, sensor.getBoolean());
  Serial.println();
  delay(50);
}
