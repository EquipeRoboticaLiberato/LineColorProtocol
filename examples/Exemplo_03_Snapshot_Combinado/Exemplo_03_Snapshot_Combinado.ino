#include <Wire.h>
#include <LineColorProtocol.h>

ColorSensorI2C sensor(SLAVE_ADDRESS, &Wire);

const char* nomesCores[] = {
  "RED", "GREEN", "BLUE", "WHITE", "BLACK", "YELLOW",
  "CYAN", "MAGENTA", "GREY", "SILVER", "NO COLOR", "ERROR"
};

const char* nomeCor(uint8_t c) {
  if (c <= _ERROR) return nomesCores[c];
  return "INVALID";
}

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
  Serial.print(sensor.getLastError());
  Serial.print(F(" | Ultimo sucesso(ms): "));
  Serial.println(sensor.getLastSuccess());
  return false;
}

void printRawRGBW(uint8_t lado) {
  Serial.print(sensor.getRawRGBW(lado, _RED));
  Serial.print('/');
  Serial.print(sensor.getRawRGBW(lado, _GREEN));
  Serial.print('/');
  Serial.print(sensor.getRawRGBW(lado, _BLUE));
  Serial.print('/');
  Serial.print(sensor.getRawRGBW(lado, _WHITE));
}

void setup() {
  Serial.begin(115200);
  while (!Serial) {}

  sensor.begin(100000UL);
  sensor.setThreshold(950);
  sensor.setStalenessTimeout(300);

  Serial.println(F("=== Exemplo 03: Snapshot Combinado Linha+Cor ==="));
  Serial.println(F("Usa READ_LINE_COLOR_SNAPSHOT do protocolo atual."));
  Serial.print(F("Comando: "));
  Serial.println(LineColorProtocol::commandName(LineColorProtocol::READ_LINE_COLOR_SNAPSHOT));
}

void loop() {
  sensor.readLineAndColor();

  if (!conexaoDisponivel(F("Snapshot linha+cor"))) {
    delay(100);
    return;
  }

  sensor.readColorRaw();

  if (!conexaoDisponivel(F("RGBW RAW"))) {
    delay(100);
    return;
  }

  Serial.print(F("Linha pos="));
  Serial.print(sensor.getPosition());
  Serial.print(F(" bool=0b"));
  LineColorProtocol::printSensorBoolean(Serial, sensor.getBoolean());
  Serial.print(F(" onLine="));
  Serial.print(sensor.onLine() ? F("1") : F("0"));

  Serial.print(F(" | Esq="));
  Serial.print(nomeCor(sensor.getColor(ESQUERDA)));
  Serial.print(F(" RAW="));
  printRawRGBW(ESQUERDA);
  Serial.print(F(" | Dir="));
  Serial.print(nomeCor(sensor.getColor(DIREITA)));
  Serial.print(F(" RAW="));
  printRawRGBW(DIREITA);
  Serial.println();

  delay(50);
}
