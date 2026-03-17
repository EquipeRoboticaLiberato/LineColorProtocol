#ifndef LINE_COLOR_PROTOCOL_H
#define LINE_COLOR_PROTOCOL_H

#include <Arduino.h>

namespace LineColorProtocol {

// ---------------------------------------------------------------------------
// Core protocol constants
// ---------------------------------------------------------------------------

static constexpr uint8_t DEFAULT_SLAVE_ADDRESS = 0x04;
static constexpr uint8_t MAX_FRAME_BUFFER = 33;
static constexpr uint8_t MAX_BUFFER = MAX_FRAME_BUFFER;

static constexpr uint8_t PROTOCOL_VERSION_CURRENT = 0x03;

static constexpr uint8_t CONTINUOUS = 0;
static constexpr uint8_t REQUEST = 1;

static constexpr uint8_t EEPROM_UNLOCK_KEY_1 = 0xA5;
static constexpr uint8_t EEPROM_UNLOCK_KEY_2 = 0x5A;
static constexpr unsigned long EEPROM_UNLOCK_WINDOW_MS = 20000UL;

// ---------------------------------------------------------------------------
// Commands
// ---------------------------------------------------------------------------

enum Command : uint8_t {
  READ_COLOR = 0x01,
  READ_RAW_RGBW = 0x02,
  READ_POSITION = 0x03,
  // 0x04 reserved (legacy READ_IR_RAW)
  SET_MODE = 0x05,
  READ_IR_BOOLEAN = 0x06,
  SET_THRESHOLD = 0x07,
  READ_CALIBRATION_MIN = 0x08,
  READ_CALIBRATION_MAX = 0x09,
  QTR_CALIBRATE = 0x0A,
  CALIBRATE_COLOR = 0x0B,
  READ_IR_CALIBRATED = 0x0C,
  READ_LINE_SNAPSHOT = 0x0D,
  READ_DEVICE_INFO = 0x0E,
  ARM_EEPROM_WRITE = 0x0F,
  READ_LINE_COLOR_SNAPSHOT = 0x10,
  READ_STATS = 0x11
};

// ---------------------------------------------------------------------------
// Capability bits and status flags
// ---------------------------------------------------------------------------

static constexpr uint32_t CAP_LINE_SNAPSHOT       = 0x00000001UL;
static constexpr uint32_t CAP_DEVICE_INFO         = 0x00000002UL;
static constexpr uint32_t CAP_LINE_COLOR_SNAPSHOT = 0x00000004UL;
static constexpr uint32_t CAP_READ_STATS          = 0x00000008UL;
static constexpr uint32_t CAP_WRITE_ACK           = 0x00000010UL;
static constexpr uint32_t CAP_EEPROM_UNLOCK       = 0x00000020UL;
static constexpr uint32_t CAP_CRC8_SEQ            = 0x00000040UL;

static constexpr uint32_t CAPABILITIES_DEFAULT =
    CAP_LINE_SNAPSHOT |
    CAP_DEVICE_INFO |
    CAP_LINE_COLOR_SNAPSHOT |
    CAP_READ_STATS |
    CAP_WRITE_ACK |
    CAP_EEPROM_UNLOCK |
    CAP_CRC8_SEQ;

static constexpr uint8_t STATUS_QTR_CALIBRATED = 0x01;
static constexpr uint8_t STATUS_ON_LINE        = 0x02;

// ---------------------------------------------------------------------------
// ACK status
// ---------------------------------------------------------------------------

enum AckStatus : uint8_t {
  ACK_OK = 0x00,
  ACK_ERR_BAD_LENGTH = 0x01,
  ACK_ERR_CRC = 0x02,
  ACK_ERR_BAD_VALUE = 0x03,
  ACK_ERR_LOCK_REQUIRED = 0x04,
  ACK_ERR_BUSY = 0x05,
  ACK_ERR_UNKNOWN_CMD = 0x06
};

// ---------------------------------------------------------------------------
// Helpers recomendados para sketches e blocos visuais
// ---------------------------------------------------------------------------

void printSensorBoolean(Stream &out, uint8_t sensorBoolean);

// ---------------------------------------------------------------------------
// Helpers de protocolo / uso avancado
// ---------------------------------------------------------------------------

uint8_t crc8(const uint8_t *data, uint8_t len);
bool frameCRCValid(const uint8_t *data, uint8_t len);
void appendCRC(uint8_t *data, uint8_t &len);
void appendU16BE(uint8_t *data, uint8_t &len, uint16_t value);
void appendU32BE(uint8_t *data, uint8_t &len, uint32_t value);
uint16_t readU16BE(const uint8_t *data);
uint32_t readU32BE(const uint8_t *data);

bool isWriteCommand(uint8_t command);
bool isReadCommand(uint8_t command);
bool isKnownCommand(uint8_t command);
bool requiresRefreshInRequestMode(uint8_t command);

uint8_t expectedResponseLength(uint8_t command, uint8_t sensorCount);
uint8_t protocolStatusFlags(bool qtrCalibrated, bool onLine);

const char *commandName(uint8_t command);
const char *ackStatusName(uint8_t status);

}  // namespace LineColorProtocol

// Camada cliente I2C (mestre) opcional, exposta pelo mesmo include para
// simplificar distribuição/uso da biblioteca.
#ifndef LINECOLORPROTOCOL_NO_I2C_CLIENT
#include "LineColorProtocolClient.h"
#endif

#endif
