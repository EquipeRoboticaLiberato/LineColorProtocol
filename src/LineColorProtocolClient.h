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

// Define as cores
//enum COLORS {
//  _RED,
//  _GREEN,
//  _BLUE,
//  _WHITE,
//  _BLACK,
//  _YELLOW,
//  _SILVER,
//  _NO_COLOR,
//  _ERROR
//};

// Define as cores
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

const String colorStr[] = {"RED", "GREEN", "BLUE", "WHITE", "BLACK", "YELLOW", "CYAN", "MAGENTA", "GREY", "SILVER", "NO COLOR", "ERROR"};

static constexpr uint8_t CONTINUOUS = LineColorProtocol::CONTINUOUS;
static constexpr uint8_t REQUEST = LineColorProtocol::REQUEST;

#ifndef DIREITA
  #define DIREITA   0
#endif

#ifndef ESQUERDA
  #define ESQUERDA  1
#endif

//==========================================================================
//                              I2C VARIABLES
//==========================================================================

// Endereço I2C do escravo e aliases de compatibilidade da camada de protocolo
static constexpr uint8_t SLAVE_ADDRESS = LineColorProtocol::DEFAULT_SLAVE_ADDRESS;

static constexpr uint8_t READ_COLOR = LineColorProtocol::READ_COLOR;
static constexpr uint8_t READ_RAW_RGBW = LineColorProtocol::READ_RAW_RGBW;
static constexpr uint8_t READ_POSITION = LineColorProtocol::READ_POSITION;
static constexpr uint8_t SET_MODE = LineColorProtocol::SET_MODE;
static constexpr uint8_t READ_IR_BOOLEAN = LineColorProtocol::READ_IR_BOOLEAN;
static constexpr uint8_t SET_THRESHOLD = LineColorProtocol::SET_THRESHOLD;
static constexpr uint8_t READ_CALIBRATION_MIN = LineColorProtocol::READ_CALIBRATION_MIN;
static constexpr uint8_t READ_CALIBRATION_MAX = LineColorProtocol::READ_CALIBRATION_MAX;
static constexpr uint8_t QTR_CALIBRATE = LineColorProtocol::QTR_CALIBRATE;
static constexpr uint8_t CALIBRATE_COLOR = LineColorProtocol::CALIBRATE_COLOR;
static constexpr uint8_t READ_IR_CALIBRATED = LineColorProtocol::READ_IR_CALIBRATED;
static constexpr uint8_t READ_LINE_SNAPSHOT = LineColorProtocol::READ_LINE_SNAPSHOT;
static constexpr uint8_t READ_DEVICE_INFO = LineColorProtocol::READ_DEVICE_INFO;
static constexpr uint8_t ARM_EEPROM_WRITE = LineColorProtocol::ARM_EEPROM_WRITE;
static constexpr uint8_t READ_LINE_COLOR_SNAPSHOT = LineColorProtocol::READ_LINE_COLOR_SNAPSHOT;
static constexpr uint8_t READ_STATS = LineColorProtocol::READ_STATS;

static constexpr uint8_t MAX_BUFFER = LineColorProtocol::MAX_BUFFER;
static constexpr uint8_t PROTOCOL_VERSION_MIN_SUPPORTED = LineColorProtocol::PROTOCOL_VERSION_MIN_SUPPORTED;

static constexpr uint32_t CAP_LINE_SNAPSHOT = LineColorProtocol::CAP_LINE_SNAPSHOT;
static constexpr uint32_t CAP_DEVICE_INFO = LineColorProtocol::CAP_DEVICE_INFO;
static constexpr uint32_t CAP_LINE_COLOR_SNAPSHOT = LineColorProtocol::CAP_LINE_COLOR_SNAPSHOT;
static constexpr uint32_t CAP_READ_STATS = LineColorProtocol::CAP_READ_STATS;
static constexpr uint32_t CAP_WRITE_ACK = LineColorProtocol::CAP_WRITE_ACK;
static constexpr uint32_t CAP_EEPROM_UNLOCK = LineColorProtocol::CAP_EEPROM_UNLOCK;
static constexpr uint32_t CAP_CRC8_SEQ = LineColorProtocol::CAP_CRC8_SEQ;

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


struct CalibrationData
{
  // Lowest readings seen during calibration.
  uint16_t minimum[QTRMaxSensors];
  // Highest readings seen during calibration.
  uint16_t maximum[QTRMaxSensors];
};

struct ColorData{
  uint8_t color;
  uint8_t rating;
  unsigned int rawRGB[4];
};


//struct SensorData {
//  ColorData direita;
//  ColorData esquerda;
//  CalibrationData sensorCalibration;
//  uint16_t sensorValue[QTRMaxSensors];
//  uint16_t linePosition;
//  byte sensorBoolean;
//  uint8_t sensorCount = 0;
//  
//};


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
    void begin(uint32_t clockHz = 100000UL);
    bool enviarComando(uint8_t comando, unsigned long timeout = 500);
    void setThreshold(uint16_t value);
    bool handshake();
    bool readStats();
    bool readLineAndColor();
    bool armEepromWrite();

    //SensorData data;

    uint8_t getColor(uint8_t lado);
    uint16_t getPosition();
    byte getBoolean();
    uint8_t getSensorCount();
    uint16_t getSingleSensor(uint8_t sensor);
    bool onLine();
    const char* getColorStr();
    //void read();
    void readColor();

    void readLine();
    void lineCalibrate();
    void colorCalibrate();

    uint16_t getThreshold();
    uint8_t getLastError() const;
    unsigned long getLastSuccess() const;
    bool hasHandshake() const;
    bool supportsLineSnapshot() const;
    bool supportsLineColorSnapshot() const;
    bool remoteQtrCalibrated() const;
    uint8_t getLastAckStatus() const;
    void setStalenessTimeout(unsigned long timeoutMs);
    bool isStale() const;
    uint32_t getCapabilities() const;
    uint32_t getUptimeRemote() const;
    uint16_t getRxCrcErrorsRemote() const;
    uint16_t getRxFrameErrorsRemote() const;
    uint16_t getRxUnknownCmdRemote() const;
    uint16_t getTxResponsesRemote() const;
    uint16_t getQtrCalibrationCountRemote() const;
    uint16_t getColorCalibrationCountRemote() const;
    uint16_t getEepromWriteCountRemote() const;
    uint32_t getLastEepromWriteMillisRemote() const;
    uint16_t getEepromUnlockRemainingMsRemote() const;

    static constexpr const char* colorStr[9] = {"RED", "GREEN", "BLUE", "WHITE", "BLACK", "YELLOW", "SILVER", "NO COLOR", "ERROR"};

  private:
  
    bool solicitarDados(uint8_t comando, uint8_t quantidade, unsigned long timeout, uint8_t expectedSeq);
    uint8_t processarComando(uint8_t comando);
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
    CalibrationData sensorCalibration = {};
    uint16_t sensorValue[QTRMaxSensors] = {0};
    uint16_t sensorValueRAW[QTRMaxSensors] = {0};
    uint16_t linePosition = 0;
    byte sensorBoolean = 0;
    uint8_t sensorCount = 0;

    
    uint16_t threshold = 500;
    uint8_t protocolVersion = 0;
    uint32_t capabilityFlags = 0;
    uint8_t remoteMode = CONTINUOUS;
    uint8_t remoteStatusFlags = 0;
    bool handshakeOk = false;
    uint8_t lastError = COMM_OK;
    unsigned long lastSuccess = 0;
    unsigned long nextHandshakeRetry = 0;
    bool thresholdDirty = false;
    uint8_t txSequence = 0;
    uint8_t lastAckStatus = ACK_OK;
    unsigned long stalenessTimeoutMs = 500;
    uint32_t i2cClockHz = 100000UL;

    uint32_t remoteUptimeMs = 0;
    uint16_t remoteRxCrcErrors = 0;
    uint16_t remoteRxFrameErrors = 0;
    uint16_t remoteRxUnknownCmd = 0;
    uint16_t remoteTxResponses = 0;
    uint16_t remoteQtrCalibrationCount = 0;
    uint16_t remoteColorCalibrationCount = 0;
    uint16_t remoteEepromWriteCount = 0;
    uint32_t remoteLastEepromWriteMs = 0;
    uint16_t remoteEepromUnlockRemainingMs = 0;
    
    uint8_t _slaveAddress;
    TwoWire* wire; // Ponteiro para o objeto Wire/I2C

};

#endif
