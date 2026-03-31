# LineColorProtocol

Biblioteca Arduino para comunicacao I2C com um modulo remoto baseado no protocolo `LineColorProtocol`.

Ela foi criada para conversar com o modulo `Line_Follower_Color_Sensor`, encapsulando o protocolo e oferecendo uma API simples para leitura de linha, leitura de cor, diagnostico e calibracao remota.

## Para que serve

Com esta biblioteca, o Arduino mestre pode:

- ler linha calibrada
- ler posicao da linha
- ler mapa booleano dos sensores
- ler cor classificada
- ler snapshot combinado de linha + cor
- ler valores RAW do sensor de linha
- ler valores RAW RGBW dos sensores de cor
- solicitar calibracao remota com protecao de escrita
- consultar estado e estatisticas de comunicacao

## Exemplo minimo de uso

```cpp
#include <Wire.h>
#include <LineColorProtocol.h>

ColorSensorI2C sensor(SLAVE_ADDRESS, &Wire);

void setup() {
  Serial.begin(115200);
  sensor.begin(100000UL);
  sensor.setThreshold(950);
}

void loop() {
  sensor.readLineAndColor();

  if (sensor.status() != ColorSensorI2C::CONNECTED) {
    Serial.println(F("Modulo nao disponivel"));
    delay(100);
    return;
  }

  Serial.print(F("Posicao: "));
  Serial.println(sensor.getPosition());

  Serial.print(F("Booleano: 0b"));
  LineColorProtocol::printSensorBoolean(Serial, sensor.getBoolean());
  Serial.println();

  Serial.print(F("Cor esquerda: "));
  Serial.println(ColorSensorI2C::colorStr[sensor.getColor(ESQUERDA)]);

  Serial.print(F("Cor direita: "));
  Serial.println(ColorSensorI2C::colorStr[sensor.getColor(DIREITA)]);

  delay(50);
}
```

## Funcionalidades principais

### Uso comum

- `begin(clockHz)`
- `status()`
- `readLine()`
- `readColor()`
- `readLineAndColor()`
- `getPosition()`
- `getBoolean()`
- `getSingleSensor()`
- `getColor()`
- `setThreshold()`

### Leitura RAW

- `readLineRaw()`
- `getSingleSensorRaw()`
- `readColorRaw()`
- `getRawRGBW()`

### Calibracao remota segura

- `armEepromWrite()`
- `lineCalibrate()`
- `colorCalibrate()`

### Diagnostico avancado

- `getLastError()`
- `getLastSuccess()`
- `getLastAckStatus()`
- `getProtocolVersion()`
- `readStats()`
- `getStats()`

## Estados de conexao

`status()` retorna um destes estados:

- `ColorSensorI2C::DISCONNECTED`
  - o modulo ainda nao entrou ou ainda nao concluiu sincronizacao valida
- `ColorSensorI2C::CONNECTED`
  - comunicacao normal
- `ColorSensorI2C::LOST_CONNECTION`
  - o modulo ja conectou antes, mas perdeu comunicacao alem do timeout configurado

## Comandos e capacidades principais do protocolo

A biblioteca ja encapsula os comandos mais importantes do protocolo atual, incluindo:

- `READ_LINE_SNAPSHOT`
- `READ_LINE_COLOR_SNAPSHOT`
- `READ_COLOR`
- `READ_IR_RAW`
- `READ_RAW_RGBW`
- `READ_DEVICE_INFO`
- `READ_STATS`
- `SET_THRESHOLD`
- `QTR_CALIBRATE`
- `CALIBRATE_COLOR`
- `ARM_EEPROM_WRITE`

## Observacao importante

Esta biblioteca foi feita para se comunicar com o modulo `Line_Follower_Color_Sensor`.

Ou seja, quem usa a biblioteca precisa apenas conhecer a API do mestre. O firmware interno do modulo remoto e os detalhes da gravacao ficam transparentes para o uso normal.

## Exemplos

Veja a pasta `examples/` para exemplos prontos:

- CRC e framing basico
- tamanhos de comandos
- write + ACK
- snapshot linha + cor
- estatisticas e helpers
