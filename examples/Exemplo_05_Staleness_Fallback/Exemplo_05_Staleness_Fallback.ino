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
  sensor.setThreshold(950);
  sensor.setStalenessTimeout(250UL);

  Serial.println(F("=== Exemplo 05: Fallback por Status ==="));
  Serial.println(F("Desconecte o modulo I2C para simular falha e observar o fallback."));
}

void loop() {
  sensor.readLineAndColor();

  ColorSensorI2C::ConnectionStatus currentStatus = sensor.status();
  if (currentStatus != ColorSensorI2C::CONNECTED) {
    Serial.print(F("[FALLBACK] Status="));
    Serial.print(nomeStatus(currentStatus));
    Serial.print(F(" lastError="));
    Serial.print(sensor.getLastError());
    Serial.print(F(" lastSuccess="));
    Serial.println(sensor.getLastSuccess());
    // Aqui, no robo real: parar motores / entrar em modo seguro.
    delay(100);
    return;
  }

  Serial.print(F("OK pos="));
  Serial.print(sensor.getPosition());
  Serial.print(F(" bool=0b"));
  LineColorProtocol::printSensorBoolean(Serial, sensor.getBoolean());
  Serial.println();
  delay(50);
}
