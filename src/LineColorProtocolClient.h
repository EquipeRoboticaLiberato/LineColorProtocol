//==========================================================================
//                              COLORSENSORI2C_H
//==========================================================================

#ifndef LINE_COLOR_PROTOCOL_CLIENT_H
#define LINE_COLOR_PROTOCOL_CLIENT_H

#include <Arduino.h>
#include <Wire.h>
#ifndef LINE_COLOR_PROTOCOL_H
#include <LineColorProtocol.h>
#endif

//==========================================================================
//                              GLOBAL DEFINES
//==========================================================================

enum COLORS {
  _RED,
  _GREEN,
  _BLUE,
  _WHITE,
  _BLACK,
  _YELLOW,
  _CYAN,
  _MAGENTA,
  _GREY,
  _SILVER,
  _NO_COLOR,
  _ERROR
};

// Tabela global indexada por enum de cor.
extern const char * const colorStr[12];

#ifndef DIREITA
  #define DIREITA   0
#endif

#ifndef ESQUERDA
  #define ESQUERDA  1
#endif

//==========================================================================
//                              I2C VARIABLES
//==========================================================================

// Endereço I2C do escravo e aliases de comandos da camada de protocolo
static constexpr uint8_t SLAVE_ADDRESS = LineColorProtocol::DEFAULT_SLAVE_ADDRESS;

static constexpr uint8_t READ_COLOR = LineColorProtocol::READ_COLOR;
static constexpr uint8_t READ_RAW_RGBW = LineColorProtocol::READ_RAW_RGBW;
static constexpr uint8_t READ_IR_RAW = LineColorProtocol::READ_IR_RAW;
static constexpr uint8_t SET_THRESHOLD = LineColorProtocol::SET_THRESHOLD;
static constexpr uint8_t QTR_CALIBRATE = LineColorProtocol::QTR_CALIBRATE;
static constexpr uint8_t CALIBRATE_COLOR = LineColorProtocol::CALIBRATE_COLOR;
static constexpr uint8_t READ_LINE_SNAPSHOT = LineColorProtocol::READ_LINE_SNAPSHOT;
static constexpr uint8_t READ_DEVICE_INFO = LineColorProtocol::READ_DEVICE_INFO;
static constexpr uint8_t ARM_EEPROM_WRITE = LineColorProtocol::ARM_EEPROM_WRITE;
static constexpr uint8_t READ_LINE_COLOR_SNAPSHOT = LineColorProtocol::READ_LINE_COLOR_SNAPSHOT;
static constexpr uint8_t READ_STATS = LineColorProtocol::READ_STATS;

static constexpr uint8_t MAX_BUFFER = LineColorProtocol::MAX_BUFFER;

static constexpr uint8_t STATUS_QTR_CALIBRATED = LineColorProtocol::STATUS_QTR_CALIBRATED;
static constexpr uint8_t STATUS_ON_LINE = LineColorProtocol::STATUS_ON_LINE;

static constexpr uint8_t ACK_OK = LineColorProtocol::ACK_OK;
static constexpr uint8_t ACK_ERR_BAD_LENGTH = LineColorProtocol::ACK_ERR_BAD_LENGTH;
static constexpr uint8_t ACK_ERR_CRC = LineColorProtocol::ACK_ERR_CRC;
static constexpr uint8_t ACK_ERR_BAD_VALUE = LineColorProtocol::ACK_ERR_BAD_VALUE;
static constexpr uint8_t ACK_ERR_LOCK_REQUIRED = LineColorProtocol::ACK_ERR_LOCK_REQUIRED;
static constexpr uint8_t ACK_ERR_BUSY = LineColorProtocol::ACK_ERR_BUSY;
static constexpr uint8_t ACK_ERR_UNKNOWN_CMD = LineColorProtocol::ACK_ERR_UNKNOWN_CMD;

static constexpr uint8_t EEPROM_UNLOCK_KEY_1 = LineColorProtocol::EEPROM_UNLOCK_KEY_1;
static constexpr uint8_t EEPROM_UNLOCK_KEY_2 = LineColorProtocol::EEPROM_UNLOCK_KEY_2;

/// The maximum number of sensors supported by an instance of this class.
const uint8_t QTRMaxSensors = 8;


struct ColorData{
  uint8_t color;
  uint8_t rating;
  uint16_t rawRGB[4];
};

struct LineColorRemoteStats {
  uint32_t uptimeMs;
  uint16_t rxCrcErrors;
  uint16_t rxFrameErrors;
  uint16_t rxUnknownCommands;
  uint16_t txResponses;
  uint16_t qtrCalibrationCount;
  uint16_t colorCalibrationCount;
  uint16_t eepromWriteCount;
  uint32_t lastEepromWriteMs;
  uint16_t eepromUnlockRemainingMs;
};


class ColorSensorI2C {
  public:
    enum CommError : uint8_t {
      COMM_OK = 0,
      COMM_ERR_INVALID_SIZE,
      COMM_ERR_TX,
      COMM_ERR_RX_TIMEOUT,
      COMM_ERR_RX_SHORT,
      COMM_ERR_CHECKSUM,
      COMM_ERR_UNKNOWN_COMMAND,
      COMM_ERR_SEQ_MISMATCH,
      COMM_ERR_REMOTE_ACK
    };
  
    ColorSensorI2C(uint8_t slaveAddress, TwoWire* wireInstance);

    // API basica / mais indicada para blocos
    void begin(uint32_t clockHz = 100000UL);
    bool handshake();
    bool isConnected() const;
    uint8_t getProtocolVersion() const;
    void setThreshold(uint16_t value);
    bool armEepromWrite();
    void readLine();
    void readLineRaw();
    void readColor();
    void readColorRaw();
    bool readLineAndColor();
    void lineCalibrate();
    void colorCalibrate();

    uint8_t getColor(uint8_t lado);
    uint16_t getPosition();
    byte getBoolean();
    uint8_t getSensorCount();
    uint16_t getSingleSensor(uint8_t sensor);
    uint16_t getSingleSensorRaw(uint8_t sensor);
    uint16_t getRawRGBW(uint8_t lado, uint8_t channel);
    bool onLine();
    const char* getColorStr();
    uint16_t getThreshold();
    uint8_t getLastError() const;
    void setStalenessTimeout(unsigned long timeoutMs);
    bool isStale() const;

    // API de diagnostico / uso avancado
    bool enviarComando(uint8_t comando, unsigned long timeout = 500);
    unsigned long getLastSuccess() const;
    bool remoteQtrCalibrated() const;
    uint8_t getLastAckStatus() const;
    bool readStats();
    const LineColorRemoteStats &getStats() const;

    static const char * const * colorStr;

  private:
  
    bool solicitarDados(uint8_t comando, uint8_t quantidade, unsigned long timeout, uint8_t expectedSeq);
    bool checksum(byte *values, uint8_t numBytes);
    void storeValues(byte *values, uint8_t comando);
    uint8_t adjustBoolean(uint8_t sensorData, uint8_t numSensors);
    void setCommError(CommError errorCode);
    void setCommSuccess();
    void tryHandshakeIfNeeded();
    void syncThresholdIfNeeded();
    uint8_t crc8(const uint8_t *data, uint8_t len) const;
    bool isWriteCommand(uint8_t comando) const;
    uint8_t expectedResponseLength(uint8_t comando) const;
    uint8_t nextSequence();
    uint16_t readU16BE(const uint8_t *data) const;
    uint32_t readU32BE(const uint8_t *data) const;

    ColorData direita = {_ERROR, 0, {0, 0, 0, 0}};
    ColorData esquerda = {_ERROR, 0, {0, 0, 0, 0}};
    uint16_t sensorValue[QTRMaxSensors] = {0};
    uint16_t sensorValueRAW[QTRMaxSensors] = {0};
    uint16_t linePosition = 0;
    byte sensorBoolean = 0;
    uint8_t sensorCount = 0;

    
    uint16_t threshold = 500;
    uint8_t protocolVersion = 0;
    uint8_t remoteStatusFlags = 0;
    bool handshakeOk = false;
    uint8_t lastError = COMM_OK;
    unsigned long lastSuccess = 0;
    unsigned long nextHandshakeRetry = 0;
    bool thresholdDirty = false;
    uint8_t txSequence = 0;
    uint8_t lastAckStatus = ACK_OK;
    unsigned long stalenessTimeoutMs = 500;
    
    LineColorRemoteStats remoteStats = {};
    
    uint8_t _slaveAddress;
    TwoWire* wire; // Ponteiro para o objeto Wire/I2C

};

#endif
