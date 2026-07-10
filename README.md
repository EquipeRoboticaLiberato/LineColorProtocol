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
- ler valores RAW RGB corrigidos por ambiente e fator K
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
- `readColorCalibration()`
- `getColorCalibrationMin()`
- `getColorCalibrationMax()`
- `readColorCorrectedRaw()`
- `getCorrectedRawRGB()`

Os limites calibrados de cor sao retornados por canal RGB, em `int16_t`, pois representam valores ja subtraidos da leitura ambiente/branco.

O RGB RAW corrigido mostra a etapa intermediaria usada pelo firmware antes da normalizacao: canal RAW com subtracao da leitura ambiente/branco e multiplicacao pelo fator K calculado pela calibracao.

### Cor processada

- `readColorRGB()`
- `getRGB()`
- `readColorHSV()`
- `getHSV()`

`getHSV()` retorna `H` em graus (`0..360`) e `S`/`V` em percentual (`0..100`).

### Visualizacao para exemplos

- `#include <LineColorProtocolView.h>`
- `LineColorProtocolView::readAndPrintLineAndColor()`
- `LineColorProtocolView::readAndPrintStats()`
- `LineColorProtocolView::updateColorViewMode()`

Esse header e opcional. Ele concentra funcoes de impressao no `Serial` para manter os exemplos mais curtos e mais amigaveis para iniciantes.

### Calibracao remota segura

- `armEepromWrite()`
- `lineCalibrate()`
- `colorCalibrate()`
- `lineCalibrateAndWait()`
- `colorCalibrateAndWait()`

`lineCalibrate()` e `colorCalibrate()` retornam `bool`: `true` indica que o escravo aceitou o comando e respondeu `ACK_OK`.
Os helpers `lineCalibrateAndWait()` e `colorCalibrateAndWait()` fazem o fluxo completo mais seguro para uso comum: armam a escrita em EEPROM, solicitam a calibracao e aguardam o modulo remoto sair de `BUSY`.

### Diagnostico avancado

- `getLastError()`
- `getLastSuccess()`
- `getLastAckStatus()`
- `getProtocolVersion()`
- `remoteQtrCalibrated()`
- `remoteColorCalibrated()`
- `remoteBusy()`
- `remoteLineCalibrating()`
- `remoteColorCalibrating()`
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

`begin(clockHz)` retorna `true` quando o handshake inicial foi concluido. Se retornar `false`, o barramento I2C foi iniciado mesmo assim e o sketch pode continuar; chamadas futuras de `status()` tentam reconectar automaticamente.

## Comandos e capacidades principais do protocolo

A biblioteca ja encapsula os comandos mais importantes do protocolo atual (`PROTOCOL_VERSION_CURRENT = 0x08`), incluindo:

- `READ_LINE_SNAPSHOT`
- `READ_LINE_COLOR_SNAPSHOT`
- `READ_COLOR`
- `READ_IR_RAW`
- `READ_RAW_RGBW`
- `READ_COLOR_CALIBRATION`
- `READ_COLOR_CORRECTED_RGB`
- `READ_COLOR_RGB`
- `READ_COLOR_HSV`
- `READ_DEVICE_INFO`
- `READ_STATS`
- `SET_THRESHOLD`
- `QTR_CALIBRATE`
- `CALIBRATE_COLOR`
- `ARM_EEPROM_WRITE`

O protocolo atual tambem publica flags de status para indicar calibracao de linha valida, calibracao de cor valida, modulo ocupado, calibracao de linha em andamento e calibracao de cor em andamento.

## Observacao importante

Esta biblioteca foi feita para se comunicar com o modulo `Line_Follower_Color_Sensor`.

Ou seja, quem usa a biblioteca precisa apenas conhecer a API do mestre. O firmware interno do modulo remoto e os detalhes da gravacao ficam transparentes para o uso normal.

Como a versao `0x08` adiciona leitura remota dos limites calibrados de cor, atualize a biblioteca do mestre e o firmware do escravo juntos.

## Exemplos

A pasta `examples/` foi organizada com foco no uso real do mestre:

- `Exemplo_01_Handshake_Basico`
  - mostra o estado da conexao, versao remota e erros mais recentes
- `Exemplo_02_Leitura_Linha`
  - le linha calibrada e linha RAW
- `Exemplo_03_Snapshot_Combinado`
  - le linha + cor no snapshot principal e permite alternar a visualizacao entre RGBW RAW, limites calibrados, RGB RAW corrigido, RGB normalizado e HSV
- `Exemplo_04_Calibracao_Segura_Estatisticas`
  - mostra calibracao remota com espera segura, estatisticas e leituras logo apos calibrar
- `Exemplo_05_Staleness_Fallback`
  - mostra como fazer fallback quando o modulo nao estiver `CONNECTED`
- `Exemplo_07_Leitura_Simples`
  - exemplo curto para demonstracao inicial de linha + cor

Tambem foi mantido um exemplo mais interno, separado dos exemplos de uso comum:

- `Exemplo_06_Protocolo_Puro`
  - mostra CRC, framing, ACK, tamanhos esperados de resposta e parse de frames simulados
  - este exemplo usa `#define LINECOLORPROTOCOL_NO_I2C_CLIENT` por ser focado apenas nos helpers do protocolo
