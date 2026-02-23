#include "LineColorProtocol.h"

namespace LineColorProtocol {

uint8_t crc8(const uint8_t *data, uint8_t len)
{
  uint8_t crc = 0x00;
  while (len--) {
    crc ^= *data++;
    for (uint8_t i = 0; i < 8; i++) {
      if (crc & 0x80) {
        crc = (uint8_t)((crc << 1) ^ 0x07);
      } else {
        crc <<= 1;
      }
    }
  }
  return crc;
}

bool frameCRCValid(const uint8_t *data, uint8_t len)
{
  if (len < 2) return false;
  return crc8(data, (uint8_t)(len - 1)) == data[len - 1];
}

void appendCRC(uint8_t *data, uint8_t &len)
{
  data[len++] = crc8(data, len);
}

void appendU16BE(uint8_t *data, uint8_t &len, uint16_t value)
{
  data[len++] = (uint8_t)(value >> 8);
  data[len++] = (uint8_t)(value & 0xFF);
}

void appendU32BE(uint8_t *data, uint8_t &len, uint32_t value)
{
  data[len++] = (uint8_t)(value >> 24);
  data[len++] = (uint8_t)(value >> 16);
  data[len++] = (uint8_t)(value >> 8);
  data[len++] = (uint8_t)(value & 0xFF);
}

uint16_t readU16BE(const uint8_t *data)
{
  return ((uint16_t)data[0] << 8) | data[1];
}

uint32_t readU32BE(const uint8_t *data)
{
  return ((uint32_t)data[0] << 24) |
         ((uint32_t)data[1] << 16) |
         ((uint32_t)data[2] << 8)  |
         ((uint32_t)data[3]);
}

bool isWriteCommand(uint8_t command)
{
  switch (command) {
    case SET_MODE:
    case SET_THRESHOLD:
    case QTR_CALIBRATE:
    case CALIBRATE_COLOR:
    case ARM_EEPROM_WRITE:
      return true;
    default:
      return false;
  }
}

bool isReadCommand(uint8_t command)
{
  switch (command) {
    case READ_COLOR:
    case READ_RAW_RGBW:
    case READ_POSITION:
    case READ_IR_BOOLEAN:
    case READ_CALIBRATION_MIN:
    case READ_CALIBRATION_MAX:
    case READ_IR_CALIBRATED:
    case READ_LINE_SNAPSHOT:
    case READ_DEVICE_INFO:
    case READ_LINE_COLOR_SNAPSHOT:
    case READ_STATS:
      return true;
    default:
      return false;
  }
}

bool isKnownCommand(uint8_t command)
{
  return isReadCommand(command) || isWriteCommand(command);
}

bool requiresRefreshInRequestMode(uint8_t command)
{
  switch (command) {
    case READ_COLOR:
    case READ_RAW_RGBW:
    case READ_POSITION:
    case READ_IR_BOOLEAN:
    case READ_IR_CALIBRATED:
    case READ_LINE_SNAPSHOT:
    case READ_LINE_COLOR_SNAPSHOT:
      return true;
    default:
      return false;
  }
}

uint8_t expectedResponseLength(uint8_t command, uint8_t sensorCount)
{
  if (sensorCount > 8) sensorCount = 8;

  switch (command) {
    case READ_COLOR:               return 6;                              // seq + 4 payload + crc
    case READ_RAW_RGBW:            return 18;                             // seq + 16 payload + crc
    case READ_POSITION:            return 4;                              // seq + 2 payload + crc
    case READ_IR_BOOLEAN:          return 3;                              // seq + 1 payload + crc
    case READ_IR_CALIBRATED:       return (uint8_t)(sensorCount * 2 + 2); // seq + N*2 + crc
    case READ_CALIBRATION_MIN:     return (uint8_t)(sensorCount * 2 + 2);
    case READ_CALIBRATION_MAX:     return (uint8_t)(sensorCount * 2 + 2);
    case READ_LINE_SNAPSHOT:       return (uint8_t)(sensorCount * 2 + 7);  // seq + (1+1+2+1+N*2) + crc
    case READ_LINE_COLOR_SNAPSHOT: return (uint8_t)(sensorCount * 2 + 11); // seq + line + colors(4) + crc
    case READ_DEVICE_INFO:         return 10;                              // seq + 8 payload + crc
    case READ_STATS:               return 26;                              // seq + 24 payload + crc
    case SET_MODE:
    case SET_THRESHOLD:
    case QTR_CALIBRATE:
    case CALIBRATE_COLOR:
    case ARM_EEPROM_WRITE:         return 4;                               // ACK: seq + cmd + status + crc
    default:                       return 0;
  }
}

uint8_t protocolStatusFlags(bool qtrCalibrated, bool onLine)
{
  uint8_t flags = 0;
  if (qtrCalibrated) flags |= STATUS_QTR_CALIBRATED;
  if (onLine)        flags |= STATUS_ON_LINE;
  return flags;
}

const char *commandName(uint8_t command)
{
  switch (command) {
    case READ_COLOR: return "READ_COLOR";
    case READ_RAW_RGBW: return "READ_RAW_RGBW";
    case READ_POSITION: return "READ_POSITION";
    case SET_MODE: return "SET_MODE";
    case READ_IR_BOOLEAN: return "READ_IR_BOOLEAN";
    case SET_THRESHOLD: return "SET_THRESHOLD";
    case READ_CALIBRATION_MIN: return "READ_CALIBRATION_MIN";
    case READ_CALIBRATION_MAX: return "READ_CALIBRATION_MAX";
    case QTR_CALIBRATE: return "QTR_CALIBRATE";
    case CALIBRATE_COLOR: return "CALIBRATE_COLOR";
    case READ_IR_CALIBRATED: return "READ_IR_CALIBRATED";
    case READ_LINE_SNAPSHOT: return "READ_LINE_SNAPSHOT";
    case READ_DEVICE_INFO: return "READ_DEVICE_INFO";
    case ARM_EEPROM_WRITE: return "ARM_EEPROM_WRITE";
    case READ_LINE_COLOR_SNAPSHOT: return "READ_LINE_COLOR_SNAPSHOT";
    case READ_STATS: return "READ_STATS";
    default: return "UNKNOWN";
  }
}

const char *ackStatusName(uint8_t status)
{
  switch (status) {
    case ACK_OK: return "ACK_OK";
    case ACK_ERR_BAD_LENGTH: return "ACK_ERR_BAD_LENGTH";
    case ACK_ERR_CRC: return "ACK_ERR_CRC";
    case ACK_ERR_BAD_VALUE: return "ACK_ERR_BAD_VALUE";
    case ACK_ERR_LOCK_REQUIRED: return "ACK_ERR_LOCK_REQUIRED";
    case ACK_ERR_BUSY: return "ACK_ERR_BUSY";
    case ACK_ERR_UNKNOWN_CMD: return "ACK_ERR_UNKNOWN_CMD";
    default: return "ACK_UNKNOWN";
  }
}

}  // namespace LineColorProtocol

