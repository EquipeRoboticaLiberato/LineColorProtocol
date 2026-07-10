#ifndef LINE_COLOR_PROTOCOL_VIEW_H
#define LINE_COLOR_PROTOCOL_VIEW_H

#include <Arduino.h>
#include <LineColorProtocol.h>

namespace LineColorProtocolView {

enum ColorViewMode : uint8_t {
  COLOR_VIEW_RAW_RGBW = 0,
  COLOR_VIEW_CALIBRATION,
  COLOR_VIEW_CORRECTED_RAW,
  COLOR_VIEW_RGB,
  COLOR_VIEW_HSV
};

inline const __FlashStringHelper* statusName(ColorSensorI2C::ConnectionStatus status)
{
  switch (status) {
    case ColorSensorI2C::DISCONNECTED: return F("DISCONNECTED");
    case ColorSensorI2C::CONNECTED: return F("CONNECTED");
    case ColorSensorI2C::LOST_CONNECTION: return F("LOST_CONNECTION");
    default: return F("UNKNOWN");
  }
}

inline const char* colorName(uint8_t color)
{
  if (color <= _ERROR) return ColorSensorI2C::colorStr[color];
  return "INVALID";
}

inline bool connectionAvailable(Stream &out, ColorSensorI2C &sensor, const __FlashStringHelper *stage)
{
  ColorSensorI2C::ConnectionStatus currentStatus = sensor.status();
  if (currentStatus == ColorSensorI2C::CONNECTED) return true;

  out.print(stage);
  out.print(F(" status="));
  out.print(statusName(currentStatus));
  out.print(F(" erro="));
  out.println(sensor.getLastError());
  return false;
}

inline void printBasicStatus(Stream &out, ColorSensorI2C &sensor, ColorSensorI2C::ConnectionStatus currentStatus)
{
  out.print(F("Status: "));
  out.println(statusName(currentStatus));
  out.print(F("SensorCount remoto: "));
  out.println(sensor.getSensorCount());
  out.print(F("Versao protocolo remoto: "));
  out.println(sensor.getProtocolVersion());
  out.print(F("Versao protocolo atual (lib): "));
  out.println(LineColorProtocol::PROTOCOL_VERSION_CURRENT);
  out.print(F("Ultimo erro: "));
  out.println(sensor.getLastError());
  out.println();
}

inline void printLineValues(Stream &out, ColorSensorI2C &sensor)
{
  out.print(F("Pos: "));
  out.print(sensor.getPosition());
  out.print(F(" | Bool: 0b"));
  LineColorProtocol::printSensorBoolean(out, sensor.getBoolean());

  out.print(F(" | Sensores: "));
  for (uint8_t i = 0; i < sensor.getSensorCount(); i++) {
    out.print(sensor.getSingleSensor(i));
    if (i + 1 < sensor.getSensorCount()) out.print('\t');
  }

  out.print(F(" | RAW: "));
  for (uint8_t i = 0; i < sensor.getSensorCount(); i++) {
    out.print(sensor.getSingleSensorRaw(i));
    if (i + 1 < sensor.getSensorCount()) out.print('\t');
  }
  out.println();
}

inline void printRawRGBW(Stream &out, ColorSensorI2C &sensor, uint8_t lado)
{
  out.print(sensor.getRawRGBW(lado, _RED));
  out.print('/');
  out.print(sensor.getRawRGBW(lado, _GREEN));
  out.print('/');
  out.print(sensor.getRawRGBW(lado, _BLUE));
  out.print('/');
  out.print(sensor.getRawRGBW(lado, _WHITE));
}

inline void printCalibration(Stream &out, ColorSensorI2C &sensor, uint8_t lado)
{
  out.print(sensor.getColorCalibrationMin(lado, RGB_RED));
  out.print('/');
  out.print(sensor.getColorCalibrationMin(lado, RGB_GREEN));
  out.print('/');
  out.print(sensor.getColorCalibrationMin(lado, RGB_BLUE));
  out.print('|');
  out.print(sensor.getColorCalibrationMax(lado, RGB_RED));
  out.print('/');
  out.print(sensor.getColorCalibrationMax(lado, RGB_GREEN));
  out.print('/');
  out.print(sensor.getColorCalibrationMax(lado, RGB_BLUE));
}

inline void printCorrectedRawRGB(Stream &out, ColorSensorI2C &sensor, uint8_t lado)
{
  out.print(sensor.getCorrectedRawRGB(lado, RGB_RED));
  out.print('/');
  out.print(sensor.getCorrectedRawRGB(lado, RGB_GREEN));
  out.print('/');
  out.print(sensor.getCorrectedRawRGB(lado, RGB_BLUE));
}

inline void printRGB(Stream &out, ColorSensorI2C &sensor, uint8_t lado)
{
  out.print(sensor.getRGB(lado, RGB_RED));
  out.print('/');
  out.print(sensor.getRGB(lado, RGB_GREEN));
  out.print('/');
  out.print(sensor.getRGB(lado, RGB_BLUE));
}

inline void printHSV(Stream &out, ColorSensorI2C &sensor, uint8_t lado)
{
  out.print(F("H="));
  out.print(sensor.getHSV(lado, HSV_HUE));
  out.print(F(" S="));
  out.print(sensor.getHSV(lado, HSV_SATURATION));
  out.print(F("% V="));
  out.print(sensor.getHSV(lado, HSV_VALUE));
  out.print('%');
}

inline void printColorValues(Stream &out, ColorSensorI2C &sensor, uint8_t lado, ColorViewMode mode)
{
  switch (mode) {
    case COLOR_VIEW_RAW_RGBW:
      out.print(F(" RAW="));
      printRawRGBW(out, sensor, lado);
      break;
    case COLOR_VIEW_CALIBRATION:
      out.print(F(" CAL="));
      printCalibration(out, sensor, lado);
      break;
    case COLOR_VIEW_CORRECTED_RAW:
      out.print(F(" RAW_CORR="));
      printCorrectedRawRGB(out, sensor, lado);
      break;
    case COLOR_VIEW_RGB:
      out.print(F(" RGB="));
      printRGB(out, sensor, lado);
      break;
    case COLOR_VIEW_HSV:
      out.print(F(" HSV="));
      printHSV(out, sensor, lado);
      break;
  }
}

inline void printColorViewHelp(Stream &out)
{
  out.println(F("r RAW | l calibracao | c corrigido | g RGB | h HSV"));
}

inline bool setColorViewMode(char command, ColorViewMode &mode)
{
  if (command == 'r' || command == 'R') {
    mode = COLOR_VIEW_RAW_RGBW;
    return true;
  }
  if (command == 'l' || command == 'L') {
    mode = COLOR_VIEW_CALIBRATION;
    return true;
  }
  if (command == 'c' || command == 'C') {
    mode = COLOR_VIEW_CORRECTED_RAW;
    return true;
  }
  if (command == 'g' || command == 'G') {
    mode = COLOR_VIEW_RGB;
    return true;
  }
  if (command == 'h' || command == 'H') {
    mode = COLOR_VIEW_HSV;
    return true;
  }
  return false;
}

inline void updateColorViewMode(Stream &in, ColorViewMode &mode)
{
  while (in.available()) {
    (void)setColorViewMode((char)in.read(), mode);
  }
}

inline bool readColorDetails(ColorSensorI2C &sensor, ColorViewMode mode)
{
  switch (mode) {
    case COLOR_VIEW_RAW_RGBW:
      sensor.readColorRaw();
      return true;
    case COLOR_VIEW_CALIBRATION:
      return sensor.readColorCalibration();
    case COLOR_VIEW_CORRECTED_RAW:
      return sensor.readColorCorrectedRaw();
    case COLOR_VIEW_RGB:
      return sensor.readColorRGB();
    case COLOR_VIEW_HSV:
      return sensor.readColorHSV();
  }
  return false;
}

inline void printLineAndColor(Stream &out, ColorSensorI2C &sensor, ColorViewMode mode)
{
  out.print(F("Linha pos="));
  out.print(sensor.getPosition());
  out.print(F(" bool=0b"));
  LineColorProtocol::printSensorBoolean(out, sensor.getBoolean());
  out.print(F(" onLine="));
  out.print(sensor.onLine() ? F("1") : F("0"));

  out.print(F(" | Esq="));
  out.print(colorName(sensor.getColor(ESQUERDA)));
  printColorValues(out, sensor, ESQUERDA, mode);

  out.print(F(" | Dir="));
  out.print(colorName(sensor.getColor(DIREITA)));
  printColorValues(out, sensor, DIREITA, mode);
  out.println();
}

inline void printStats(Stream &out, const LineColorRemoteStats &stats)
{
  out.print(F("Uptime(ms): ")); out.println(stats.uptimeMs);
  out.print(F("RX CRC err: ")); out.println(stats.rxCrcErrors);
  out.print(F("RX frame err: ")); out.println(stats.rxFrameErrors);
  out.print(F("RX unknown cmd: ")); out.println(stats.rxUnknownCommands);
  out.print(F("TX responses: ")); out.println(stats.txResponses);
  out.print(F("QTR calib count: ")); out.println(stats.qtrCalibrationCount);
  out.print(F("Color calib count: ")); out.println(stats.colorCalibrationCount);
  out.print(F("EEPROM writes: ")); out.println(stats.eepromWriteCount);
  out.print(F("Last EEPROM write(ms): ")); out.println(stats.lastEepromWriteMs);
  out.print(F("Unlock restante(ms): ")); out.println(stats.eepromUnlockRemainingMs);
}

inline void printAckError(Stream &out, ColorSensorI2C &sensor)
{
  out.print(F("erro="));
  out.print(sensor.getLastError());
  out.print(F(" ack="));
  out.print(sensor.getLastAckStatus());
  out.print(F(" "));
  out.println(LineColorProtocol::ackStatusName(sensor.getLastAckStatus()));
}

}  // namespace LineColorProtocolView

#endif
