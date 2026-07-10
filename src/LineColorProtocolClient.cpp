//==========================================================================
//                              COLORSENSORI2C_CPP
//==========================================================================

#include "Arduino.h"
#include "LineColorProtocol.h"

#ifndef LINECOLORPROTOCOLCLIENT_ENABLE_DEBUG_LOGS
#define LINECOLORPROTOCOLCLIENT_ENABLE_DEBUG_LOGS 0
#endif

#if LINECOLORPROTOCOLCLIENT_ENABLE_DEBUG_LOGS
#define LCP_LOG_PRINT(x) Serial.print(x)
#define LCP_LOG_PRINTLN(x) Serial.println(x)
#else
#define LCP_LOG_PRINT(x) do {} while (0)
#define LCP_LOG_PRINTLN(x) do {} while (0)
#endif

const char * const colorStr[12] = {
  "RED", "GREEN", "BLUE", "WHITE", "BLACK", "YELLOW",
  "CYAN", "MAGENTA", "GREY", "SILVER", "NO COLOR", "ERROR"
};

const char * const * ColorSensorI2C::colorStr = ::colorStr;

// Descricao: Inicializa o objeto e seus estados internos padrao.
// Entradas: `slaveAddress`, `wireInstance`.
// Exemplo: `ColorSensorI2C sensor(SLAVE_ADDRESS, &Wire);`
ColorSensorI2C::ColorSensorI2C(uint8_t slaveAddress, TwoWire* wireInstance)
  : _slaveAddress(slaveAddress), wire(wireInstance) {
      //_slaveAddress = slaveAddress;
  sensorCount = 6;
}

//==========================================================================

// Descricao: Inicializa a comunicacao I2C e tenta fazer handshake com o escravo.
// Entradas: `clockHz`.
// Exemplo: `bool conectado = sensor.begin(100000UL);`
bool ColorSensorI2C::begin(uint32_t clockHz) {

  wire->begin();
  wire->setClock(clockHz);

  #if defined(ARDUINO_ARCH_AVR)
    wire->setWireTimeout(25000UL, true);
  #endif

  nextStatusProbe = 0;
  return handshake();
  
}

//==========================================================================

//void ColorSensorI2C::read(){
//
//  enviarComando(READ_IR_RAW);
//
//}

//==========================================================================

// Descricao: Le os dados de linha usando o snapshot unico do protocolo atual.
// Entradas: sem parametros.
// Exemplo: `sensor.readLine();`
void ColorSensorI2C::readLine(){

  tryHandshakeIfNeeded();
  syncThresholdIfNeeded();
  enviarComando(READ_LINE_SNAPSHOT);

}

//==========================================================================

// Descricao: Le os valores crus do QTR, sem aplicar calibracao.
// Entradas: sem parametros.
// Exemplo: `sensor.readLineRaw();`
void ColorSensorI2C::readLineRaw(){

  tryHandshakeIfNeeded();
  syncThresholdIfNeeded();
  enviarComando(READ_IR_RAW);

}

//==========================================================================

// Descricao: Le linha e cor em uma unica chamada usando o snapshot combinado atual.
// Entradas: sem parametros.
// Exemplo: `sensor.readLineAndColor();`
bool ColorSensorI2C::readLineAndColor(){

  tryHandshakeIfNeeded();
  syncThresholdIfNeeded();
  return enviarComando(READ_LINE_COLOR_SNAPSHOT);

}

//==========================================================================

// Descricao: Le as classificacoes de cor dos sensores esquerdo e direito.
// Entradas: sem parametros.
// Exemplo: `sensor.readColor();`
void ColorSensorI2C::readColor(){

  tryHandshakeIfNeeded();
  syncThresholdIfNeeded();

  enviarComando(READ_COLOR);

}

//==========================================================================

// Descricao: Le os valores crus RGBW dos sensores de cor.
// Entradas: sem parametros.
// Exemplo: `sensor.readColorRaw();`
void ColorSensorI2C::readColorRaw(){

  tryHandshakeIfNeeded();
  syncThresholdIfNeeded();
  enviarComando(READ_RAW_RGBW);

}

//==========================================================================

// Descricao: Le os limites calibrados min/max dos sensores de cor remotos.
// Entradas: sem parametros.
// Exemplo: `sensor.readColorCalibration();`
bool ColorSensorI2C::readColorCalibration(){

  tryHandshakeIfNeeded();
  syncThresholdIfNeeded();
  return enviarComando(READ_COLOR_CALIBRATION);

}

//==========================================================================

// Descricao: Le os valores RAW RGB corrigidos por ambiente e fator K.
// Entradas: sem parametros.
// Exemplo: `sensor.readColorCorrectedRaw();`
bool ColorSensorI2C::readColorCorrectedRaw(){

  tryHandshakeIfNeeded();
  syncThresholdIfNeeded();
  return enviarComando(READ_COLOR_CORRECTED_RGB);

}

//==========================================================================

// Descricao: Le o RGB normalizado dos sensores de cor em escala 0..255.
// Entradas: sem parametros.
// Exemplo: `sensor.readColorRGB();`
bool ColorSensorI2C::readColorRGB(){

  tryHandshakeIfNeeded();
  syncThresholdIfNeeded();
  return enviarComando(READ_COLOR_RGB);

}

//==========================================================================

// Descricao: Le o HSV usado pelo remoto para classificar as cores.
// Entradas: sem parametros.
// Exemplo: `sensor.readColorHSV();`
bool ColorSensorI2C::readColorHSV(){

  tryHandshakeIfNeeded();
  syncThresholdIfNeeded();
  return enviarComando(READ_COLOR_HSV);

}

//==========================================================================

// Descricao: Solicita estatisticas de comunicacao e runtime do escravo.
// Entradas: sem parametros.
// Exemplo: `sensor.readStats();`
bool ColorSensorI2C::readStats(){
  tryHandshakeIfNeeded();
  return enviarComando(READ_STATS, 100);
}

//==========================================================================

// Descricao: Arma temporariamente a janela remota que permite gravacao em EEPROM.
// Entradas: sem parametros.
// Exemplo: `sensor.armEepromWrite();`
bool ColorSensorI2C::armEepromWrite(){
  tryHandshakeIfNeeded();
  return enviarComando(ARM_EEPROM_WRITE, 100);
}

//==========================================================================

// Descricao: Monta, envia e (quando aplicavel) valida o frame de comando e resposta.
// Entradas: `comando`, `timeout`.
// Exemplo: `sensor.enviarComando(READ_DEVICE_INFO);`
bool ColorSensorI2C::enviarComando(uint8_t comando, unsigned long timeout) {

  uint8_t frame[MAX_BUFFER] = {0};
  uint8_t frameLen = 0;
  const uint8_t seq = nextSequence();
  const uint8_t bytesEsperados = expectedResponseLength(comando);
  const bool expectsResponse = bytesEsperados > 0;

  frame[frameLen++] = comando;
  frame[frameLen++] = seq;

  switch (comando) {
    case SET_THRESHOLD:
      frame[frameLen++] = (uint8_t)(threshold >> 8);
      frame[frameLen++] = (uint8_t)(threshold & 0xFF);
      break;

    case ARM_EEPROM_WRITE:
      frame[frameLen++] = EEPROM_UNLOCK_KEY_1;
      frame[frameLen++] = EEPROM_UNLOCK_KEY_2;
      break;

    case QTR_CALIBRATE:
    case CALIBRATE_COLOR:
    case READ_COLOR:
    case READ_RAW_RGBW:
    case READ_IR_RAW:
    case READ_LINE_SNAPSHOT:
    case READ_DEVICE_INFO:
    case READ_LINE_COLOR_SNAPSHOT:
    case READ_STATS:
    case READ_COLOR_RGB:
    case READ_COLOR_HSV:
    case READ_COLOR_CORRECTED_RGB:
    case READ_COLOR_CALIBRATION:
      break;

    default:
      LCP_LOG_PRINTLN(F("Comando desconhecido"));
      setCommError(COMM_ERR_UNKNOWN_COMMAND);
      return false;
  }

  if (frameLen + 1 > MAX_BUFFER) {
    setCommError(COMM_ERR_INVALID_SIZE);
    return false;
  }
  LineColorProtocol::appendCRC(frame, frameLen);

  const uint8_t maxAttempts = expectsResponse ? 3 : 2;
  const unsigned long perAttemptTimeout = max((unsigned long)20, timeout / max((unsigned long)1, (unsigned long)maxAttempts));
  lastAckStatus = ACK_OK;

  for (uint8_t attempt = 0; attempt < maxAttempts; attempt++) {
    uint8_t txStatus;

    wire->beginTransmission(_slaveAddress);
    wire->write(frame, frameLen);
    txStatus = wire->endTransmission();

    if (txStatus != 0) {
      LCP_LOG_PRINT(F("Erro I2C TX: "));
      LCP_LOG_PRINTLN(txStatus);
      setCommError(COMM_ERR_TX);
    } else if (!expectsResponse) {
      setCommSuccess();
      return true;
    } else if (solicitarDados(comando, bytesEsperados, perAttemptTimeout, seq)) {
      return true;
    } else {
      LCP_LOG_PRINTLN(F("Erro: leitura I2C"));
      if (lastError == COMM_ERR_REMOTE_ACK ||
          lastError == COMM_ERR_UNKNOWN_COMMAND ||
          lastError == COMM_ERR_SEQ_MISMATCH) {
        break;
      }
    }

    if (attempt + 1 < maxAttempts) {
      delay((uint8_t)(1 << attempt)); // backoff curto: 1ms, 2ms, 4ms
    }
  }

  return false;

}

//==========================================================================

// Descricao: Solicita bytes do escravo, valida CRC/seq e armazena o payload recebido.
// Entradas: `comando`, `quantidade`, `timeout`, `expectedSeq`.
// Exemplo: `sensor.solicitarDados(READ_COLOR, 6, 50, 1); // uso interno`
bool ColorSensorI2C::solicitarDados(uint8_t comando, uint8_t quantidade, unsigned long timeout, uint8_t expectedSeq) {

  byte dataIn[MAX_BUFFER] = {0};

  if (quantidade == 0 || quantidade > MAX_BUFFER) {
    LCP_LOG_PRINTLN(F("Erro: tamanho invalido"));
    setCommError(COMM_ERR_INVALID_SIZE);
    return false;
  }

  unsigned long startTime = millis();
  uint8_t requested = wire->requestFrom((int)_slaveAddress, (int)quantidade);

  while (wire->available() < requested)
    if (millis() - startTime >= timeout)
    {
      setCommError(COMM_ERR_RX_TIMEOUT);
      return false;  // Timeout atingido
    }

  // Se os dados foram recebidos antes do timeout
  uint8_t i = 0;
  while (wire->available()) {
    if (i < MAX_BUFFER)
      dataIn[i] = wire->read();
    else
      (void)wire->read();
    i++;
  }

  if (i != requested || requested < 3) {
    LCP_LOG_PRINT(F("Erro I2C RX invalido: "));
    LCP_LOG_PRINT(i);
    LCP_LOG_PRINT(F("/"));
    LCP_LOG_PRINTLN(requested);
    setCommError(COMM_ERR_RX_SHORT);
    return false;
  }

  if (!checksum(dataIn, requested)) {
    LCP_LOG_PRINTLN(F("Erro: checksum"));
    setCommError(COMM_ERR_CHECKSUM);
    return false;
  }

  if (dataIn[0] != expectedSeq) {
    LCP_LOG_PRINTLN(F("Erro: seq"));
    setCommError(COMM_ERR_SEQ_MISMATCH);
    return false;
  }

  if (isWriteCommand(comando)) {
    // ACK frame: [seq][cmd][status][crc8]
    if (requested != 4 || dataIn[1] != comando) {
      setCommError(COMM_ERR_RX_SHORT);
      return false;
    }
    lastAckStatus = dataIn[2];
    if (lastAckStatus != ACK_OK) {
      setCommError(COMM_ERR_REMOTE_ACK);
      return false;
    }
    setCommSuccess();
    return true;
  }

  const uint8_t payloadLen = (uint8_t)(requested - 2);

  switch (comando) {
    case READ_COLOR:
      if (payloadLen != 4) {
        setCommError(COMM_ERR_INVALID_SIZE);
        return false;
      }
      break;

    case READ_RAW_RGBW:
      if (payloadLen != 16) {
        setCommError(COMM_ERR_INVALID_SIZE);
        return false;
      }
      break;

    case READ_DEVICE_INFO:
      if (payloadLen != 8) {
        setCommError(COMM_ERR_INVALID_SIZE);
        return false;
      }
      break;

    case READ_STATS:
      if (payloadLen != 24) {
        setCommError(COMM_ERR_INVALID_SIZE);
        return false;
      }
      break;

    case READ_COLOR_RGB:
      if (payloadLen != 6) {
        setCommError(COMM_ERR_INVALID_SIZE);
        return false;
      }
      break;

    case READ_COLOR_HSV:
      if (payloadLen != 8) {
        setCommError(COMM_ERR_INVALID_SIZE);
        return false;
      }
      break;

    case READ_COLOR_CORRECTED_RGB:
      if (payloadLen != 12) {
        setCommError(COMM_ERR_INVALID_SIZE);
        return false;
      }
      break;

    case READ_COLOR_CALIBRATION:
      if (payloadLen != 24) {
        setCommError(COMM_ERR_INVALID_SIZE);
        return false;
      }
      break;

    case READ_IR_RAW:
      if (payloadLen == 0 || (payloadLen & 0x01) != 0) {
        setCommError(COMM_ERR_INVALID_SIZE);
        return false;
      }
      sensorCount = (uint8_t)(payloadLen / 2);
      if (sensorCount == 0 || sensorCount > QTRMaxSensors) {
        setCommError(COMM_ERR_INVALID_SIZE);
        return false;
      }
      break;

    case READ_LINE_SNAPSHOT:
      if (payloadLen < 7) {
        setCommError(COMM_ERR_INVALID_SIZE);
        return false;
      }
      if (dataIn[1] == 0 || dataIn[1] > QTRMaxSensors) {
        setCommError(COMM_ERR_INVALID_SIZE);
        return false;
      }
      if (payloadLen != (uint8_t)(5 + (dataIn[1] * 2))) {
        setCommError(COMM_ERR_INVALID_SIZE);
        return false;
      }
      break;

    case READ_LINE_COLOR_SNAPSHOT:
      if (payloadLen < 11) {
        setCommError(COMM_ERR_INVALID_SIZE);
        return false;
      }
      if (dataIn[1] == 0 || dataIn[1] > QTRMaxSensors) {
        setCommError(COMM_ERR_INVALID_SIZE);
        return false;
      }
      if (payloadLen != (uint8_t)(9 + (dataIn[1] * 2))) {
        setCommError(COMM_ERR_INVALID_SIZE);
        return false;
      }
      break;

    default:
      break;
  }

  // Armazena somente o payload (sem seq e sem crc)
  storeValues(&dataIn[1], comando);
  setCommSuccess();
  return true;

}

//==========================================================================

// Descricao: Parseia o payload recebido e atualiza os campos internos da biblioteca.
// Entradas: `values`, `comando`.
// Exemplo: `sensor.storeValues(buf, READ_COLOR); // uso interno`
void ColorSensorI2C::storeValues(byte *values, uint8_t comando) {
  
  switch (comando) {
    //----------------------
    case READ_COLOR:
    
      direita.color = values[0];
      direita.rating = values[1];
      esquerda.color = values[2];
      esquerda.rating = values[3];
    
      break;
    //----------------------
    case READ_RAW_RGBW:
      for(int i = 0; i <= _WHITE; i++)
        direita.rawRGB[i] = values[i * 2] << 8 | values[(i * 2) + 1];

      for(int i = 0; i <= _WHITE; i++)
        esquerda.rawRGB[i] = values[(i * 2) + 8] << 8 | values[(i * 2) + 9];

      break;
    //----------------------
    case READ_COLOR_RGB:
      for(int i = 0; i <= RGB_BLUE; i++)
        direita.normalizedRGB[i] = values[i];

      for(int i = 0; i <= RGB_BLUE; i++)
        esquerda.normalizedRGB[i] = values[i + 3];

      break;
    //----------------------
    case READ_COLOR_HSV:
      direita.hsv[HSV_HUE] = values[0] << 8 | values[1];
      direita.hsv[HSV_SATURATION] = values[2];
      direita.hsv[HSV_VALUE] = values[3];
      esquerda.hsv[HSV_HUE] = values[4] << 8 | values[5];
      esquerda.hsv[HSV_SATURATION] = values[6];
      esquerda.hsv[HSV_VALUE] = values[7];

      break;
    //----------------------
    case READ_COLOR_CORRECTED_RGB:
      for(int i = 0; i <= RGB_BLUE; i++)
        direita.correctedRawRGB[i] = values[i * 2] << 8 | values[(i * 2) + 1];

      for(int i = 0; i <= RGB_BLUE; i++)
        esquerda.correctedRawRGB[i] = values[(i * 2) + 6] << 8 | values[(i * 2) + 7];

      break;
    //----------------------
    case READ_COLOR_CALIBRATION:
      for(int i = 0; i <= RGB_BLUE; i++)
        direita.calibrationMin[i] = readI16BE(&values[i * 2]);

      for(int i = 0; i <= RGB_BLUE; i++)
        direita.calibrationMax[i] = readI16BE(&values[(i * 2) + 6]);

      for(int i = 0; i <= RGB_BLUE; i++)
        esquerda.calibrationMin[i] = readI16BE(&values[(i * 2) + 12]);

      for(int i = 0; i <= RGB_BLUE; i++)
        esquerda.calibrationMax[i] = readI16BE(&values[(i * 2) + 18]);

      break;
    //----------------------
    case READ_IR_RAW:
      for(int i=0; i < sensorCount; i++){
        sensorValueRAW[i] = values[i*2] << 8 | values[(i*2)+1];
      }
      for(int i=sensorCount; i < QTRMaxSensors; i++){
        sensorValueRAW[i] = 0;
      }
      break;
    //----------------------
    case READ_LINE_SNAPSHOT:
      {
        uint8_t remoteCount = values[0];
        uint8_t flags = values[1];
        uint8_t count = remoteCount;

        if (count == 0 || count > QTRMaxSensors) {
          count = sensorCount;
        }

        sensorCount = count;
        remoteStatusFlags = flags;
        linePosition = values[2] << 8 | values[3];
        sensorBoolean = adjustBoolean(values[4], sensorCount);

        for (int i = 0; i < sensorCount; i++) {
          const uint8_t idx = 5 + (i * 2);
          sensorValue[i] = values[idx] << 8 | values[idx + 1];
        }

        for (int i = sensorCount; i < QTRMaxSensors; i++) {
          sensorValue[i] = 0;
        }
      }

      break;
    //----------------------
    case READ_LINE_COLOR_SNAPSHOT:
      {
        uint8_t remoteCount = values[0];
        uint8_t flags = values[1];
        uint8_t count = remoteCount;

        if (count == 0 || count > QTRMaxSensors) {
          count = sensorCount;
        }

        sensorCount = count;
        remoteStatusFlags = flags;
        linePosition = values[2] << 8 | values[3];
        sensorBoolean = adjustBoolean(values[4], sensorCount);

        for (int i = 0; i < sensorCount; i++) {
          const uint8_t idx = 5 + (i * 2);
          sensorValue[i] = values[idx] << 8 | values[idx + 1];
        }
        for (int i = sensorCount; i < QTRMaxSensors; i++) {
          sensorValue[i] = 0;
        }

        const uint8_t colorIdx = 5 + (sensorCount * 2);
        direita.color = values[colorIdx + 0];
        direita.rating = values[colorIdx + 1];
        esquerda.color = values[colorIdx + 2];
        esquerda.rating = values[colorIdx + 3];
      }

      break;
    //----------------------
    case READ_DEVICE_INFO:
      protocolVersion = values[0];

      if (values[1] >= 1 && values[1] <= QTRMaxSensors) {
        sensorCount = values[1];
      }

      remoteStatusFlags = values[7];
      handshakeOk = (protocolVersion == LineColorProtocol::PROTOCOL_VERSION_CURRENT);

      break;
    //----------------------
    case READ_STATS:
      remoteStats.uptimeMs = readU32BE(&values[0]);
      remoteStats.rxCrcErrors = readU16BE(&values[4]);
      remoteStats.rxFrameErrors = readU16BE(&values[6]);
      remoteStats.rxUnknownCommands = readU16BE(&values[8]);
      remoteStats.txResponses = readU16BE(&values[10]);
      remoteStats.qtrCalibrationCount = readU16BE(&values[12]);
      remoteStats.colorCalibrationCount = readU16BE(&values[14]);
      remoteStats.eepromWriteCount = readU16BE(&values[16]);
      remoteStats.lastEepromWriteMs = readU32BE(&values[18]);
      remoteStats.eepromUnlockRemainingMs = readU16BE(&values[22]);
      break;
    //----------------------
    default:
      LCP_LOG_PRINTLN(F("Nao foi possivel armazenar"));
      break;
  }  
  
  
}

//==========================================================================

// Descricao: Centraliza/alinha o byte binario da linha conforme o numero de sensores.
// Entradas: `sensorData`, `numSensors`.
// Exemplo: `uint8_t b = sensor.adjustBoolean(bits, 6); // uso interno`
uint8_t ColorSensorI2C::adjustBoolean(uint8_t sensorData, uint8_t numSensors) {
  
    if (numSensors == 8) {
        return sensorData;
    }

    if (numSensors < 2 || numSensors > 8) {
        // Caso inválido
        return sensorData;
    }

    // Calcular os deslocamentos para centralizar os bits
    uint8_t padding = 8 - numSensors;  // Bits vazios totais
    uint8_t leftPadding = padding / 2;
    uint8_t rightPadding = padding - leftPadding;

    // Centralizar os bits no byte
    uint8_t centeredData = sensorData << rightPadding;

    // Ajustar LSB e MSB com os valores adjacentes
    uint8_t lsb = (centeredData >> rightPadding) & 1; // Bit mais à direita do dado original
    uint8_t msb = (centeredData >> (7 - leftPadding)) & 1; // Bit mais à esquerda do dado original

    // Aplicar LSB e MSB
    centeredData |= (lsb * ((1 << rightPadding) - 1));            // Preencher bits da direita
    centeredData |= (msb * (((1 << leftPadding) - 1) << (8 - leftPadding))); // Preencher bits da esquerda

    return centeredData;
}

//==========================================================================

// Verifica o checksum
// Descricao: Valida o CRC-8 de um frame recebido.
// Entradas: `values`, `numBytes`.
// Exemplo: `bool ok = sensor.checksum(frame, len); // uso interno`
bool ColorSensorI2C::checksum(byte *values, uint8_t numBytes) {
  return LineColorProtocol::frameCRCValid(values, numBytes);

}
//==========================================================================

// Descricao: Calcula o CRC-8 (polinomio 0x07) de um buffer.
// Entradas: `data`, `len`.
// Exemplo: `uint8_t c = sensor.crc8(frame, len); // uso interno`
uint8_t ColorSensorI2C::crc8(const uint8_t *data, uint8_t len) const {
  return LineColorProtocol::crc8(data, len);
}

//==========================================================================

// Descricao: Verifica se o comando pertence ao grupo de escrita/configuracao.
// Entradas: `comando`.
// Exemplo: `bool wr = sensor.isWriteCommand(SET_THRESHOLD); // uso interno`
bool ColorSensorI2C::isWriteCommand(uint8_t comando) const {
  return LineColorProtocol::isWriteCommand(comando);
}

//==========================================================================

// Descricao: Retorna o tamanho esperado da resposta para um comando e quantidade de sensores.
// Entradas: `comando`.
// Exemplo: `uint8_t n = sensor.expectedResponseLength(READ_STATS); // uso interno`
uint8_t ColorSensorI2C::expectedResponseLength(uint8_t comando) const {
  return LineColorProtocol::expectedResponseLength(comando, sensorCount);
}

//==========================================================================

// Descricao: Gera o proximo sequence id diferente de zero.
// Entradas: sem parametros.
// Exemplo: `uint8_t seq = sensor.nextSequence(); // uso interno`
uint8_t ColorSensorI2C::nextSequence() {
  txSequence++;
  if (txSequence == 0) txSequence++;
  return txSequence;
}

//==========================================================================

// Descricao: Le um inteiro de 16 bits em formato big-endian a partir de um buffer.
// Entradas: `data`.
// Exemplo: `uint16_t v = sensor.readU16BE(buf); // uso interno`
uint16_t ColorSensorI2C::readU16BE(const uint8_t *data) const {
  return LineColorProtocol::readU16BE(data);
}

//==========================================================================

// Descricao: Le um inteiro assinado de 16 bits em formato big-endian a partir de um buffer.
// Entradas: `data`.
// Exemplo: `int16_t v = sensor.readI16BE(buf); // uso interno`
int16_t ColorSensorI2C::readI16BE(const uint8_t *data) const {
  return LineColorProtocol::readI16BE(data);
}

//==========================================================================

// Descricao: Le um inteiro de 32 bits em formato big-endian a partir de um buffer.
// Entradas: `data`.
// Exemplo: `uint32_t v = sensor.readU32BE(buf); // uso interno`
uint32_t ColorSensorI2C::readU32BE(const uint8_t *data) const {
  return LineColorProtocol::readU32BE(data);
}

//==========================================================================

// Descricao: Atualiza o ultimo erro de comunicacao registrado.
// Entradas: `errorCode`.
// Exemplo: `sensor.setCommError(ColorSensorI2C::COMM_ERR_TX); // uso interno`
void ColorSensorI2C::setCommError(CommError errorCode) {
  lastError = (uint8_t)errorCode;
}

//==========================================================================

// Descricao: Marca sucesso de comunicacao e registra o timestamp.
// Entradas: sem parametros.
// Exemplo: `sensor.setCommSuccess(); // uso interno`
void ColorSensorI2C::setCommSuccess() {
  lastError = (uint8_t)COMM_OK;
  lastSuccess = millis();
}

//==========================================================================

// Descricao: Tenta refazer o handshake quando a biblioteca ainda nao esta sincronizada.
// Entradas: sem parametros.
// Exemplo: `sensor.tryHandshakeIfNeeded(); // uso interno`
void ColorSensorI2C::tryHandshakeIfNeeded() {
  if (handshakeOk) return;

  unsigned long now = millis();
  if ((long)(now - nextHandshakeRetry) < 0) return;

  if (handshake()) {
    nextHandshakeRetry = 0;
  }
}

//==========================================================================

// Descricao: Reenvia o threshold pendente para sincronizar com o escravo.
// Entradas: sem parametros.
// Exemplo: `sensor.syncThresholdIfNeeded(); // uso interno`
void ColorSensorI2C::syncThresholdIfNeeded() {
  if (!thresholdDirty) return;

  if (enviarComando(SET_THRESHOLD)) {
    thresholdDirty = false;
  }
}

//==========================================================================

// Descricao: Solicita informacoes do dispositivo remoto e valida compatibilidade de protocolo.
// Entradas: sem parametros.
// Exemplo: `handshake(); // uso interno`
bool ColorSensorI2C::handshake() {
  handshakeOk = false;

  if (!enviarComando(READ_DEVICE_INFO, 50)) {
    nextHandshakeRetry = millis() + 250;
    return false;
  }

  if (!handshakeOk) {
    nextHandshakeRetry = millis() + 250;
    return false;
  }

  nextHandshakeRetry = 0;
  return true;
}

//==========================================================================

// Descricao: Atualiza o threshold local e agenda sincronizacao com o escravo.
// Entradas: `value`.
// Exemplo: `sensor.setThreshold(950);`
void ColorSensorI2C::setThreshold(uint16_t value){
  if (value > 1000) value = 1000;
  threshold = value;
  thresholdDirty = true;
  tryHandshakeIfNeeded();
  syncThresholdIfNeeded();
}

//==========================================================================

// Descricao: Retorna a posicao da linha calculada na ultima leitura.
// Entradas: sem parametros.
// Exemplo: `uint16_t pos = sensor.getPosition();`
uint16_t ColorSensorI2C::getPosition(){
  return linePosition;
}


//==========================================================================

// Descricao: Retorna o valor interno associado ao estado/telemetria deste objeto.
// Entradas: `lado`.
// Exemplo: `uint8_t cor = sensor.getColor(DIREITA);`
uint8_t ColorSensorI2C::getColor(uint8_t lado){
  if(lado == DIREITA)
    return direita.color;
  else if(lado == ESQUERDA)
    return esquerda.color;
  else
    return _ERROR;
}

//==========================================================================

// Descricao: Retorna o nome da cor atual (lado direito por padrao) usando a tabela global.
// Entradas: sem parametros.
// Exemplo: `const char *nome = sensor.getColorStr();`
const char* ColorSensorI2C::getColorStr(){
  uint8_t color = getColor(DIREITA);
  if (color > _ERROR) color = _ERROR;
  return ::colorStr[color];
}

//==========================================================================

// Descricao: Retorna o valor interno associado ao estado/telemetria deste objeto.
// Entradas: sem parametros.
// Exemplo: `byte bits = sensor.getBoolean();`
byte ColorSensorI2C::getBoolean(){
  return sensorBoolean;
}

//==========================================================================

// Descricao: Retorna a quantidade de sensores configurados/descobertos.
// Entradas: sem parametros.
// Exemplo: `uint8_t n = sensor.getSensorCount();`
uint8_t ColorSensorI2C::getSensorCount(){
  return sensorCount;
}


//==========================================================================

// Descricao: Retorna o valor interno associado ao estado/telemetria deste objeto.
// Entradas: sem parametros.
// Exemplo: `uint16_t th = sensor.getThreshold();`
uint16_t ColorSensorI2C::getThreshold(){
  return threshold;
}

//==========================================================================

// Descricao: Retorna o valor interno associado ao estado/telemetria deste objeto.
// Entradas: sem parametros.
// Exemplo: `uint8_t e = sensor.getLastError();`
uint8_t ColorSensorI2C::getLastError() const{
  return lastError;
}

//==========================================================================

// Descricao: Retorna o valor interno associado ao estado/telemetria deste objeto.
// Entradas: sem parametros.
// Exemplo: `unsigned long t = sensor.getLastSuccess();`
unsigned long ColorSensorI2C::getLastSuccess() const{
  return lastSuccess;
}

//==========================================================================

// Descricao: Retorna a versao do protocolo informada pelo escravo no handshake.
// Entradas: sem parametros.
// Exemplo: `uint8_t v = sensor.getProtocolVersion();`
uint8_t ColorSensorI2C::getProtocolVersion() const{
  return protocolVersion;
}

//==========================================================================

// Descricao: Executa a operacao principal deste metodo no contexto da biblioteca.
// Entradas: sem parametros.
// Exemplo: `bool ok = sensor.remoteQtrCalibrated();`
bool ColorSensorI2C::remoteQtrCalibrated() const{
  return (remoteStatusFlags & STATUS_QTR_CALIBRATED) != 0;
}

//==========================================================================

// Descricao: Informa se os sensores de cor remotos ja possuem calibracao valida.
// Entradas: sem parametros.
// Exemplo: `if (sensor.remoteColorCalibrated()) { ... }`
bool ColorSensorI2C::remoteColorCalibrated() const{
  return (remoteStatusFlags & STATUS_COLOR_CALIBRATED) != 0;
}

//==========================================================================

// Descricao: Informa se o modulo remoto esta executando alguma tarefa longa.
// Entradas: sem parametros.
// Exemplo: `while (sensor.remoteBusy()) { ... }`
bool ColorSensorI2C::remoteBusy() const{
  return (remoteStatusFlags & STATUS_BUSY) != 0;
}

//==========================================================================

// Descricao: Informa se a calibracao de linha remota esta em andamento.
// Entradas: sem parametros.
// Exemplo: `if (sensor.remoteLineCalibrating()) { ... }`
bool ColorSensorI2C::remoteLineCalibrating() const{
  return (remoteStatusFlags & STATUS_LINE_CALIBRATING) != 0;
}

//==========================================================================

// Descricao: Informa se a calibracao de cor remota esta em andamento.
// Entradas: sem parametros.
// Exemplo: `if (sensor.remoteColorCalibrating()) { ... }`
bool ColorSensorI2C::remoteColorCalibrating() const{
  return (remoteStatusFlags & STATUS_COLOR_CALIBRATING) != 0;
}

//==========================================================================

// Descricao: Retorna o valor interno associado ao estado/telemetria deste objeto.
// Entradas: sem parametros.
// Exemplo: `uint8_t ack = sensor.getLastAckStatus();`
uint8_t ColorSensorI2C::getLastAckStatus() const{
  return lastAckStatus;
}

//==========================================================================

// Descricao: Atualiza um parametro interno de configuracao deste objeto.
// Entradas: `timeoutMs`.
// Exemplo: `sensor.setStalenessTimeout(300);`
void ColorSensorI2C::setStalenessTimeout(unsigned long timeoutMs){
  stalenessTimeoutMs = timeoutMs;
}

//==========================================================================

// Descricao: Verifica se uma conexao antes valida ficou tempo demais sem sucesso recente.
// Entradas: sem parametros.
// Exemplo: `bool lost = hasLostConnection(); // uso interno`
bool ColorSensorI2C::hasLostConnection() const{
  if (lastSuccess == 0) return false;
  if (lastError == COMM_OK) return false;
  return (millis() - lastSuccess) > stalenessTimeoutMs;
}

//==========================================================================

// Descricao: Decide se ja esta na hora de consultar novamente o remoto.
// Entradas: `now`.
// Exemplo: `if (shouldProbeStatus(millis())) { ... }`
bool ColorSensorI2C::shouldProbeStatus(unsigned long now) const{
  return (long)(now - nextStatusProbe) >= 0;
}

//==========================================================================

// Descricao: Retorna o estado resumido da conexao com o modulo remoto.
// Entradas: sem parametros.
// Exemplo: `if (sensor.status() == ColorSensorI2C::CONNECTED) { ... }`
ColorSensorI2C::ConnectionStatus ColorSensorI2C::status(){
  const unsigned long now = millis();

  if (!handshakeOk) {
    tryHandshakeIfNeeded();
    return handshakeOk ? CONNECTED : DISCONNECTED;
  }

  if (shouldProbeStatus(now)) {
    nextStatusProbe = now + statusProbeIntervalMs;
    (void)refreshDeviceInfo(50);
  }

  if (!handshakeOk) return DISCONNECTED;
  if (hasLostConnection()) return LOST_CONNECTION;
  return CONNECTED;
}

//==========================================================================

// Descricao: Atualiza informacoes leves do remoto, incluindo flags de calibracao.
// Entradas: `timeout`.
// Exemplo: `sensor.refreshDeviceInfo(); // uso interno`
bool ColorSensorI2C::refreshDeviceInfo(unsigned long timeout){
  return enviarComando(READ_DEVICE_INFO, timeout);
}

//==========================================================================

// Descricao: Aguarda o modulo remoto sair do estado ocupado/calibrando.
// Entradas: `timeoutMs`.
// Exemplo: `sensor.waitRemoteIdle(8000); // uso interno`
bool ColorSensorI2C::waitRemoteIdle(unsigned long timeoutMs){
  const unsigned long start = millis();

  do {
    if (refreshDeviceInfo(100)) {
      if (!remoteBusy()) {
        return true;
      }
    }
    delay(50);
  } while ((millis() - start) <= timeoutMs);

  return false;
}

//==========================================================================

// Descricao: Retorna a estrutura de estatisticas lidas do escravo.
// Entradas: sem parametros.
// Exemplo: `const LineColorRemoteStats &stats = sensor.getStats();`
const LineColorRemoteStats &ColorSensorI2C::getStats() const{
  return remoteStats;
}

//==========================================================================

// Descricao: Retorna o valor interno associado ao estado/telemetria deste objeto.
// Entradas: `sensor`.
// Exemplo: `uint16_t v = sensor.getSingleSensor(0);`
uint16_t ColorSensorI2C::getSingleSensor(uint8_t sensor){
  if(sensor >= QTRMaxSensors)
    return 0;
  else
    return sensorValue[sensor];
}

//==========================================================================

// Descricao: Retorna o valor cru do sensor QTR indicado.
// Entradas: `sensor`.
// Exemplo: `uint16_t v = sensor.getSingleSensorRaw(0);`
uint16_t ColorSensorI2C::getSingleSensorRaw(uint8_t sensor){
  if(sensor >= QTRMaxSensors)
    return 0;
  else
    return sensorValueRAW[sensor];
}

//==========================================================================

// Descricao: Retorna o canal RGBW cru do sensor de cor indicado.
// Entradas: `lado`, `channel`.
// Exemplo: `uint16_t w = sensor.getRawRGBW(DIREITA, _WHITE);`
uint16_t ColorSensorI2C::getRawRGBW(uint8_t lado, uint8_t channel){
  if (channel > _WHITE) return 0;

  if(lado == DIREITA)
    return direita.rawRGB[channel];
  else if(lado == ESQUERDA)
    return esquerda.rawRGB[channel];
  else
    return 0;
}

//==========================================================================

// Descricao: Retorna o limite minimo calibrado do canal de cor indicado.
// Entradas: `lado`, `channel`.
// Exemplo: `int16_t rMin = sensor.getColorCalibrationMin(DIREITA, RGB_RED);`
int16_t ColorSensorI2C::getColorCalibrationMin(uint8_t lado, uint8_t channel){
  if (channel > RGB_BLUE) return 0;

  if(lado == DIREITA)
    return direita.calibrationMin[channel];
  else if(lado == ESQUERDA)
    return esquerda.calibrationMin[channel];
  else
    return 0;
}

//==========================================================================

// Descricao: Retorna o limite maximo calibrado do canal de cor indicado.
// Entradas: `lado`, `channel`.
// Exemplo: `int16_t rMax = sensor.getColorCalibrationMax(DIREITA, RGB_RED);`
int16_t ColorSensorI2C::getColorCalibrationMax(uint8_t lado, uint8_t channel){
  if (channel > RGB_BLUE) return 0;

  if(lado == DIREITA)
    return direita.calibrationMax[channel];
  else if(lado == ESQUERDA)
    return esquerda.calibrationMax[channel];
  else
    return 0;
}

//==========================================================================

// Descricao: Retorna o RGB RAW corrigido por ambiente e fator K.
// Entradas: `lado`, `channel`.
// Exemplo: `uint16_t r = sensor.getCorrectedRawRGB(DIREITA, RGB_RED);`
uint16_t ColorSensorI2C::getCorrectedRawRGB(uint8_t lado, uint8_t channel){
  if (channel > RGB_BLUE) return 0;

  if(lado == DIREITA)
    return direita.correctedRawRGB[channel];
  else if(lado == ESQUERDA)
    return esquerda.correctedRawRGB[channel];
  else
    return 0;
}

//==========================================================================

// Descricao: Retorna o canal RGB normalizado indicado, em escala 0..255.
// Entradas: `lado`, `channel`.
// Exemplo: `uint8_t r = sensor.getRGB(DIREITA, RGB_RED);`
uint8_t ColorSensorI2C::getRGB(uint8_t lado, uint8_t channel){
  if (channel > RGB_BLUE) return 0;

  if(lado == DIREITA)
    return direita.normalizedRGB[channel];
  else if(lado == ESQUERDA)
    return esquerda.normalizedRGB[channel];
  else
    return 0;
}

//==========================================================================

// Descricao: Retorna o canal HSV indicado: H=0..360, S/V=0..100.
// Entradas: `lado`, `channel`.
// Exemplo: `uint16_t h = sensor.getHSV(DIREITA, HSV_HUE);`
uint16_t ColorSensorI2C::getHSV(uint8_t lado, uint8_t channel){
  if (channel > HSV_VALUE) return 0;

  if(lado == DIREITA)
    return direita.hsv[channel];
  else if(lado == ESQUERDA)
    return esquerda.hsv[channel];
  else
    return 0;
}

//==========================================================================

// Descricao: Informa se a linha foi detectada pelo snapshot remoto ou por heuristica local.
// Entradas: sem parametros.
// Exemplo: `bool online = sensor.onLine();`
bool ColorSensorI2C::onLine(){
  return (remoteStatusFlags & STATUS_ON_LINE) != 0;
}

//==========================================================================

// Descricao: Solicita ao escravo a calibracao do sensor de linha.
// Entradas: sem parametros.
// Exemplo: `if (sensor.lineCalibrate()) { ... }`
bool ColorSensorI2C::lineCalibrate(){
  tryHandshakeIfNeeded();
  return enviarComando(QTR_CALIBRATE);
}

//==========================================================================

// Descricao: Solicita ao escravo a calibracao dos sensores de cor.
// Entradas: sem parametros.
// Exemplo: `if (sensor.colorCalibrate()) { ... }`
bool ColorSensorI2C::colorCalibrate(){
  tryHandshakeIfNeeded();
  return enviarComando(CALIBRATE_COLOR);
}

//==========================================================================

// Descricao: Arma EEPROM, solicita calibracao de linha e espera finalizar.
// Entradas: `timeoutMs`.
// Exemplo: `sensor.lineCalibrateAndWait();`
bool ColorSensorI2C::lineCalibrateAndWait(unsigned long timeoutMs){
  if (!armEepromWrite()) return false;
  if (!lineCalibrate()) return false;
  if (!waitRemoteIdle(timeoutMs)) return false;
  if (!refreshDeviceInfo(100)) return false;
  return remoteQtrCalibrated();
}

//==========================================================================

// Descricao: Arma EEPROM, solicita calibracao de cor e espera finalizar.
// Entradas: `timeoutMs`.
// Exemplo: `sensor.colorCalibrateAndWait();`
bool ColorSensorI2C::colorCalibrateAndWait(unsigned long timeoutMs){
  if (!armEepromWrite()) return false;
  if (!colorCalibrate()) return false;
  if (!waitRemoteIdle(timeoutMs)) return false;
  if (!refreshDeviceInfo(100)) return false;
  return remoteColorCalibrated();
}

//==========================================================================

//==========================================================================
