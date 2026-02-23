//==========================================================================
//                              COLORSENSORI2C_CPP
//==========================================================================

#include "Arduino.h"
#include "LineColorProtocol.h"

ColorSensorI2C::ColorSensorI2C(uint8_t slaveAddress, TwoWire* wireInstance)
  : _slaveAddress(slaveAddress), wire(wireInstance) {
      //_slaveAddress = slaveAddress;
  sensorCount = 6;
}

//==========================================================================

void ColorSensorI2C::begin(uint32_t clockHz) {

  wire->begin();
  i2cClockHz = clockHz;
  wire->setClock(i2cClockHz);
  (void)handshake();
  
}

//==========================================================================

//void ColorSensorI2C::read(){
//
//  enviarComando(READ_IR_RAW);
//
//}

//==========================================================================

void ColorSensorI2C::readLine(){

  tryHandshakeIfNeeded();
  syncThresholdIfNeeded();

  if (supportsLineSnapshot() && enviarComando(READ_LINE_SNAPSHOT)) {
    return;
  }

  // Fallback para firmwares antigos ou falha temporária do snapshot.
  enviarComando(READ_IR_CALIBRATED);
  //enviarComando(READ_IR_RAW);
  enviarComando(READ_POSITION);
  enviarComando(READ_IR_BOOLEAN);

}

//==========================================================================

bool ColorSensorI2C::readLineAndColor(){

  tryHandshakeIfNeeded();
  syncThresholdIfNeeded();

  if (supportsLineColorSnapshot()) {
    return enviarComando(READ_LINE_COLOR_SNAPSHOT);
  }

  readLine();
  readColor();
  return lastError == COMM_OK;

}

//==========================================================================

void ColorSensorI2C::readColor(){

  tryHandshakeIfNeeded();
  syncThresholdIfNeeded();

  enviarComando(READ_COLOR);

}

//==========================================================================

bool ColorSensorI2C::readStats(){
  tryHandshakeIfNeeded();
  if ((capabilityFlags & CAP_READ_STATS) == 0) return false;
  return enviarComando(READ_STATS, 100);
}

//==========================================================================

bool ColorSensorI2C::armEepromWrite(){
  tryHandshakeIfNeeded();
  if ((capabilityFlags & CAP_EEPROM_UNLOCK) == 0) return false;
  return enviarComando(ARM_EEPROM_WRITE, 100);
}

//==========================================================================

bool ColorSensorI2C::enviarComando(uint8_t comando, unsigned long timeout) {

  uint8_t frame[MAX_BUFFER] = {0};
  uint8_t frameLen = 0;
  const uint8_t seq = nextSequence();
  const uint8_t bytesEsperados = expectedResponseLength(comando);
  const bool expectsResponse = bytesEsperados > 0;

  frame[frameLen++] = comando;
  frame[frameLen++] = seq;

  switch (comando) {
    case SET_MODE:
      frame[frameLen++] = REQUEST;
      break;

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
    case READ_POSITION:
    case READ_IR_BOOLEAN:
    case READ_IR_CALIBRATED:
    case READ_CALIBRATION_MIN:
    case READ_CALIBRATION_MAX:
    case READ_LINE_SNAPSHOT:
    case READ_DEVICE_INFO:
    case READ_LINE_COLOR_SNAPSHOT:
    case READ_STATS:
      break;

    default:
      Serial.println(F("Comando desconhecido"));
      setCommError(COMM_ERR_UNKNOWN_COMMAND);
      return false;
  }

  if (frameLen + 1 > MAX_BUFFER) {
    setCommError(COMM_ERR_INVALID_SIZE);
    return false;
  }
  frame[frameLen++] = crc8(frame, frameLen);

  const uint8_t maxAttempts = expectsResponse ? 3 : 2;
  const unsigned long perAttemptTimeout = max((unsigned long)20, timeout / max((unsigned long)1, (unsigned long)maxAttempts));
  lastAckStatus = ACK_OK;

  for (uint8_t attempt = 0; attempt < maxAttempts; attempt++) {
    uint8_t txStatus;

    wire->beginTransmission(_slaveAddress);
    wire->write(frame, frameLen);
    txStatus = wire->endTransmission();

    if (txStatus != 0) {
      Serial.print(F("Erro I2C TX: "));
      Serial.println(txStatus);
      setCommError(COMM_ERR_TX);
    } else if (!expectsResponse) {
      setCommSuccess();
      return true;
    } else if (solicitarDados(comando, bytesEsperados, perAttemptTimeout, seq)) {
      return true;
    } else {
      Serial.println(F("Erro: leitura I2C"));
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

bool ColorSensorI2C::solicitarDados(uint8_t comando, uint8_t quantidade, unsigned long timeout, uint8_t expectedSeq) {

  byte dataIn[MAX_BUFFER] = {0};

  if (quantidade == 0 || quantidade > MAX_BUFFER) {
    Serial.println(F("Erro: tamanho invalido"));
    setCommError(COMM_ERR_INVALID_SIZE);
    return false;
  }

  unsigned long startTime = millis();
  uint8_t requested = wire->requestFrom((int)_slaveAddress, (int)quantidade);

  while (wire->available() < quantidade)
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


  if (requested != quantidade) {
    Serial.print(F("Erro I2C RX curto: "));
    Serial.print(requested);
    Serial.print(F("/"));
    Serial.println(quantidade);
    setCommError(COMM_ERR_RX_SHORT);
    return false;
  }

  if (i != quantidade || !checksum(dataIn, i)) {
    Serial.println(F("Erro: checksum"));
    setCommError(COMM_ERR_CHECKSUM);
    return false;
  }

  if (dataIn[0] != expectedSeq) {
    Serial.println(F("Erro: seq"));
    setCommError(COMM_ERR_SEQ_MISMATCH);
    return false;
  }

  if (isWriteCommand(comando)) {
    // ACK frame: [seq][cmd][status][crc8]
    if (quantidade != 4 || dataIn[1] != comando) {
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

  // Armazena somente o payload (sem seq e sem crc)
  storeValues(&dataIn[1], comando);
  setCommSuccess();
  return true;

}

//==========================================================================

uint8_t ColorSensorI2C::processarComando(uint8_t comando) {
  return expectedResponseLength(comando);

}

//==========================================================================

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
//      expectedBytes = 17;  // Espera 17 bytes de resposta
      for(int i=0; i<=_WHITE; i++)
        direita.rawRGB[i] = values[i*2] << 8 | values[(i*2)+1];

      for(int i=0; i<=_WHITE; i++)
        esquerda.rawRGB[i] = values[(i*2)+8] << 8 | values[(i*2)+9];
          
      break;
    //----------------------
//    case READ_IR_RAW:
////      expectedBytes = 17;  // Espera 17 bytes de resposta
//      
//      for(int i=0; i < sensorCount; i++)
//        sensorValueRAW[i] = values[i*2] << 8 | values[(i*2)+1];
//
//      break;
    //----------------------
    case READ_IR_CALIBRATED:
//      expectedBytes = 17;  // Espera 17 bytes de resposta
      //sensorBoolean = 0;
      
      for(int i=0; i < sensorCount; i++){
        sensorValue[i] = values[i*2] << 8 | values[(i*2)+1];
        //if(sensorValue[i] >= threshold)
        //  bitSet(sensorBoolean, i);
      }
      for(int i=sensorCount; i < QTRMaxSensors; i++){
        sensorValue[i] = 0;
      }

      //sensorBoolean = adjustBoolean(sensorBoolean, sensorCount);

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

      remoteMode = values[2];
      capabilityFlags = readU32BE(&values[3]);
      remoteStatusFlags = values[7];
      handshakeOk = (protocolVersion >= PROTOCOL_VERSION_MIN_SUPPORTED);

      break;
    //----------------------
    case READ_STATS:
      remoteUptimeMs = readU32BE(&values[0]);
      remoteRxCrcErrors = readU16BE(&values[4]);
      remoteRxFrameErrors = readU16BE(&values[6]);
      remoteRxUnknownCmd = readU16BE(&values[8]);
      remoteTxResponses = readU16BE(&values[10]);
      remoteQtrCalibrationCount = readU16BE(&values[12]);
      remoteColorCalibrationCount = readU16BE(&values[14]);
      remoteEepromWriteCount = readU16BE(&values[16]);
      remoteLastEepromWriteMs = readU32BE(&values[18]);
      remoteEepromUnlockRemainingMs = readU16BE(&values[22]);
      break;
    //----------------------
    //----------------------
    case READ_CALIBRATION_MIN:
//      expectedBytes = 17;  // Espera 17 bytes de resposta
      for(int i=0; i<sensorCount; i++)
        sensorCalibration.minimum[i] = values[i*2] << 8 | values[(i*2)+1];
      for(int i=sensorCount; i<QTRMaxSensors; i++)
        sensorCalibration.minimum[i] = 0;
    
    break;
    //----------------------
    case READ_CALIBRATION_MAX:
//      expectedBytes = 17;  // Espera 17 bytes de resposta
      for(int i=0; i<sensorCount; i++)
        sensorCalibration.maximum[i] = values[i*2] << 8 | values[(i*2)+1];
      for(int i=sensorCount; i<QTRMaxSensors; i++)
        sensorCalibration.maximum[i] = 0;
    
    break;
    //----------------------
    case READ_POSITION:
      //expectedBytes = 3;  // Espera 3 bytes de resposta
      linePosition = values[0] << 8 | values[1];
    break;
    //----------------------
    case READ_IR_BOOLEAN:
      //expectedBytes = 2;  // Espera 2 bytes de resposta
      //sensorBoolean = values[0];
      sensorBoolean = adjustBoolean(values[0], sensorCount);

    break;
    //----------------------
    default:
      Serial.println(F("Nao foi possivel armazenar"));
      break;
  }  
  
  
}

//==========================================================================

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
bool ColorSensorI2C::checksum(byte *values, uint8_t numBytes) {
  return LineColorProtocol::frameCRCValid(values, numBytes);

}
//==========================================================================

uint8_t ColorSensorI2C::crc8(const uint8_t *data, uint8_t len) const {
  return LineColorProtocol::crc8(data, len);
}

//==========================================================================

bool ColorSensorI2C::isWriteCommand(uint8_t comando) const {
  return LineColorProtocol::isWriteCommand(comando);
}

//==========================================================================

uint8_t ColorSensorI2C::expectedResponseLength(uint8_t comando) const {
  return LineColorProtocol::expectedResponseLength(comando, sensorCount);
}

//==========================================================================

uint8_t ColorSensorI2C::nextSequence() {
  txSequence++;
  if (txSequence == 0) txSequence++;
  return txSequence;
}

//==========================================================================

uint16_t ColorSensorI2C::readU16BE(const uint8_t *data) const {
  return LineColorProtocol::readU16BE(data);
}

//==========================================================================

uint32_t ColorSensorI2C::readU32BE(const uint8_t *data) const {
  return LineColorProtocol::readU32BE(data);
}

//==========================================================================

void ColorSensorI2C::setCommError(CommError errorCode) {
  lastError = (uint8_t)errorCode;
}

//==========================================================================

void ColorSensorI2C::setCommSuccess() {
  lastError = (uint8_t)COMM_OK;
  lastSuccess = millis();
}

//==========================================================================

void ColorSensorI2C::tryHandshakeIfNeeded() {
  if (handshakeOk) return;

  unsigned long now = millis();
  if ((long)(now - nextHandshakeRetry) < 0) return;

  if (handshake()) {
    nextHandshakeRetry = 0;
  }
}

//==========================================================================

void ColorSensorI2C::syncThresholdIfNeeded() {
  if (!thresholdDirty) return;

  if (enviarComando(SET_THRESHOLD)) {
    thresholdDirty = false;
  }
}

//==========================================================================

bool ColorSensorI2C::handshake() {
  handshakeOk = false;
  capabilityFlags = 0;

  if (!enviarComando(READ_DEVICE_INFO, 50)) {
    nextHandshakeRetry = millis() + 250;
    return false;
  }

  // Se o firmware remoto não entende o comando, enviarComando falha; se respondeu,
  // os campos foram preenchidos em storeValues().
  if (!handshakeOk) {
    capabilityFlags = 0;
    nextHandshakeRetry = millis() + 250;
    return false;
  }

  nextHandshakeRetry = 0;
  return true;
}

//==========================================================================

void ColorSensorI2C::setThreshold(uint16_t value){
  if (value > 1000) value = 1000;
  threshold = value;
  thresholdDirty = true;
  tryHandshakeIfNeeded();
  syncThresholdIfNeeded();
}

//==========================================================================

uint16_t ColorSensorI2C::getPosition(){
  return linePosition;
}


//==========================================================================

uint8_t ColorSensorI2C::getColor(uint8_t lado){
  if(lado == DIREITA)
    return direita.color;
  else if(lado == ESQUERDA)
    return esquerda.color;
  else
    return _ERROR;
}

//==========================================================================

byte ColorSensorI2C::getBoolean(){
  return sensorBoolean;
}

//==========================================================================

uint8_t ColorSensorI2C::getSensorCount(){
  return sensorCount;
}


//==========================================================================

uint16_t ColorSensorI2C::getThreshold(){
  return threshold;
}

//==========================================================================

uint8_t ColorSensorI2C::getLastError() const{
  return lastError;
}

//==========================================================================

unsigned long ColorSensorI2C::getLastSuccess() const{
  return lastSuccess;
}

//==========================================================================

bool ColorSensorI2C::hasHandshake() const{
  return handshakeOk;
}

//==========================================================================

bool ColorSensorI2C::supportsLineSnapshot() const{
  return (capabilityFlags & CAP_LINE_SNAPSHOT) != 0;
}

//==========================================================================

bool ColorSensorI2C::supportsLineColorSnapshot() const{
  return (capabilityFlags & CAP_LINE_COLOR_SNAPSHOT) != 0;
}

//==========================================================================

bool ColorSensorI2C::remoteQtrCalibrated() const{
  return (remoteStatusFlags & STATUS_QTR_CALIBRATED) != 0;
}

//==========================================================================

uint8_t ColorSensorI2C::getLastAckStatus() const{
  return lastAckStatus;
}

//==========================================================================

void ColorSensorI2C::setStalenessTimeout(unsigned long timeoutMs){
  stalenessTimeoutMs = timeoutMs;
}

//==========================================================================

bool ColorSensorI2C::isStale() const{
  if (lastSuccess == 0) return false;
  if (lastError == COMM_OK) return false;
  return (millis() - lastSuccess) > stalenessTimeoutMs;
}

//==========================================================================

uint32_t ColorSensorI2C::getCapabilities() const{
  return capabilityFlags;
}

//==========================================================================

uint32_t ColorSensorI2C::getUptimeRemote() const{ return remoteUptimeMs; }
uint16_t ColorSensorI2C::getRxCrcErrorsRemote() const{ return remoteRxCrcErrors; }
uint16_t ColorSensorI2C::getRxFrameErrorsRemote() const{ return remoteRxFrameErrors; }
uint16_t ColorSensorI2C::getRxUnknownCmdRemote() const{ return remoteRxUnknownCmd; }
uint16_t ColorSensorI2C::getTxResponsesRemote() const{ return remoteTxResponses; }
uint16_t ColorSensorI2C::getQtrCalibrationCountRemote() const{ return remoteQtrCalibrationCount; }
uint16_t ColorSensorI2C::getColorCalibrationCountRemote() const{ return remoteColorCalibrationCount; }
uint16_t ColorSensorI2C::getEepromWriteCountRemote() const{ return remoteEepromWriteCount; }
uint32_t ColorSensorI2C::getLastEepromWriteMillisRemote() const{ return remoteLastEepromWriteMs; }
uint16_t ColorSensorI2C::getEepromUnlockRemainingMsRemote() const{ return remoteEepromUnlockRemainingMs; }

//==========================================================================

uint16_t ColorSensorI2C::getSingleSensor(uint8_t sensor){
  if(sensor >= QTRMaxSensors)
    return 0;
  else
    return sensorValue[sensor];
    //return sensorValueRAW[sensor];
}

//==========================================================================

bool ColorSensorI2C::onLine(){
  if (supportsLineSnapshot() || supportsLineColorSnapshot()) {
    return (remoteStatusFlags & STATUS_ON_LINE) != 0;
  }

  if(linePosition == 0)
    return false;
  
  if(linePosition == 1000*sensorCount-1000)
    return false;

  return true;
}

//==========================================================================

void ColorSensorI2C::lineCalibrate(){
  tryHandshakeIfNeeded();
  enviarComando(QTR_CALIBRATE);
}

//==========================================================================

void ColorSensorI2C::colorCalibrate(){
  tryHandshakeIfNeeded();
  enviarComando(CALIBRATE_COLOR);
}

//==========================================================================

//==========================================================================
