#include "LineColorProtocol.h"

namespace LineColorProtocol {

// Descricao: Imprime o byte booleano do sensor sempre com 8 caracteres fixos.
// Entradas: `out`, `sensorBoolean`.
// Quando usar: quando quiser mostrar no Serial/Display o mapa booleano sem deslocar as colunas.
// Exemplo: `LineColorProtocol::printSensorBoolean(Serial, sensor.getBoolean());`
void printSensorBoolean(Stream &out, uint8_t sensorBoolean)
{
  for (uint8_t i = 0; i < 8; i++) {
    out.print(bitRead(sensorBoolean, i) ? '1' : '0');
  }
}

// Descricao: Calcula o CRC-8 (polinomio 0x07) de um buffer.
// Entradas: `data`, `len`.
// Quando usar: quando estiver montando ou validando manualmente um frame do protocolo.
// Exemplo: `uint8_t c = LineColorProtocol::crc8(buf, len);`
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

// Descricao: Confere se o ultimo byte do frame bate com o CRC-8 calculado.
// Entradas: `data`, `len`.
// Quando usar: ao receber um frame bruto e precisar verificar se ele chegou integro.
// Exemplo: `bool ok = LineColorProtocol::frameCRCValid(frame, frameLen);`
bool frameCRCValid(const uint8_t *data, uint8_t len)
{
  if (len < 2) return false;
  return crc8(data, (uint8_t)(len - 1)) == data[len - 1];
}

// Descricao: Calcula e adiciona o CRC-8 no final do frame.
// Entradas: `data`, `len`.
// Quando usar: no fim da montagem de qualquer request/response do protocolo.
// Exemplo: `LineColorProtocol::appendCRC(frame, frameLen);`
void appendCRC(uint8_t *data, uint8_t &len)
{
  const uint8_t crc = crc8(data, len);
  data[len] = crc;
  len++;
}

// Descricao: Adiciona um valor de 16 bits em formato big-endian no buffer.
// Entradas: `data`, `len`, `value`.
// Quando usar: ao serializar campos de 16 bits em requests/responses do protocolo.
// Exemplo: `LineColorProtocol::appendU16BE(frame, frameLen, 950);`
void appendU16BE(uint8_t *data, uint8_t &len, uint16_t value)
{
  data[len++] = (uint8_t)(value >> 8);
  data[len++] = (uint8_t)(value & 0xFF);
}

// Descricao: Adiciona um valor assinado de 16 bits em formato big-endian no buffer.
// Entradas: `data`, `len`, `value`.
// Quando usar: ao serializar limites calibrados que podem ficar negativos.
// Exemplo: `LineColorProtocol::appendI16BE(frame, frameLen, -12);`
void appendI16BE(uint8_t *data, uint8_t &len, int16_t value)
{
  appendU16BE(data, len, (uint16_t)value);
}

// Descricao: Adiciona um valor de 32 bits em formato big-endian no buffer.
// Entradas: `data`, `len`, `value`.
// Quando usar: ao serializar tempos, contadores ou mascaras de 32 bits.
// Exemplo: `LineColorProtocol::appendU32BE(frame, frameLen, 123456UL);`
void appendU32BE(uint8_t *data, uint8_t &len, uint32_t value)
{
  data[len++] = (uint8_t)(value >> 24);
  data[len++] = (uint8_t)(value >> 16);
  data[len++] = (uint8_t)(value >> 8);
  data[len++] = (uint8_t)(value & 0xFF);
}

// Descricao: Le um inteiro de 16 bits em formato big-endian a partir de um buffer.
// Entradas: `data`.
// Quando usar: ao interpretar payloads recebidos com campos de 16 bits.
// Exemplo: `uint16_t v = LineColorProtocol::readU16BE(&frame[0]);`
uint16_t readU16BE(const uint8_t *data)
{
  return ((uint16_t)data[0] << 8) | data[1];
}

// Descricao: Le um inteiro assinado de 16 bits em formato big-endian a partir de um buffer.
// Entradas: `data`.
// Quando usar: ao interpretar limites calibrados de cor.
// Exemplo: `int16_t v = LineColorProtocol::readI16BE(&frame[0]);`
int16_t readI16BE(const uint8_t *data)
{
  return (int16_t)readU16BE(data);
}

// Descricao: Le um inteiro de 32 bits em formato big-endian a partir de um buffer.
// Entradas: `data`.
// Quando usar: ao interpretar payloads recebidos com tempos ou contadores de 32 bits.
// Exemplo: `uint32_t v = LineColorProtocol::readU32BE(&frame[0]);`
uint32_t readU32BE(const uint8_t *data)
{
  return ((uint32_t)data[0] << 24) |
         ((uint32_t)data[1] << 16) |
         ((uint32_t)data[2] << 8)  |
         ((uint32_t)data[3]);
}

// Descricao: Verifica se o comando pertence ao grupo de escrita/configuracao.
// Entradas: `command`.
// Quando usar: para diferenciar comandos que devem retornar ACK dos comandos de leitura.
// Exemplo: `bool wr = LineColorProtocol::isWriteCommand(LineColorProtocol::SET_THRESHOLD);`
bool isWriteCommand(uint8_t command)
{
  switch (command) {
    case SET_THRESHOLD:
    case QTR_CALIBRATE:
    case CALIBRATE_COLOR:
    case ARM_EEPROM_WRITE:
      return true;
    default:
      return false;
  }
}

// Descricao: Verifica se o comando pertence ao grupo de leitura.
// Entradas: `command`.
// Quando usar: para classificar rapidamente um comando recebido no parser do protocolo.
// Exemplo: `bool rd = LineColorProtocol::isReadCommand(LineColorProtocol::READ_STATS);`
bool isReadCommand(uint8_t command)
{
  switch (command) {
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
      return true;
    default:
      return false;
  }
}

// Descricao: Verifica se o comando existe no protocolo conhecido.
// Entradas: `command`.
// Quando usar: para validar comandos antes de processar ou registrar diagnosticos.
// Exemplo: `bool known = LineColorProtocol::isKnownCommand(cmd);`
bool isKnownCommand(uint8_t command)
{
  return isReadCommand(command) || isWriteCommand(command);
}

// Descricao: Retorna o tamanho esperado da resposta para um comando e quantidade de sensores.
// Entradas: `command`, `sensorCount`.
// Quando usar: ao reservar buffers ou validar frames antes de parsear.
// Exemplo: `uint8_t n = LineColorProtocol::expectedResponseLength(LineColorProtocol::READ_STATS, 6);`
uint8_t expectedResponseLength(uint8_t command, uint8_t sensorCount)
{
  if (sensorCount > 8) sensorCount = 8;

  switch (command) {
    case READ_COLOR:               return 6;                              // seq + 4 payload + crc
    case READ_RAW_RGBW:            return 18;                             // seq + 16 payload + crc
    case READ_IR_RAW:              return (uint8_t)(sensorCount * 2 + 2); // seq + N*2 + crc
    case READ_LINE_SNAPSHOT:       return (uint8_t)(sensorCount * 2 + 7);  // seq + (1+1+2+1+N*2) + crc
    case READ_LINE_COLOR_SNAPSHOT: return (uint8_t)(sensorCount * 2 + 11); // seq + line + colors(4) + crc
    case READ_DEVICE_INFO:         return 10;                              // seq + 8 payload + crc
    case READ_STATS:               return 26;                              // seq + 24 payload + crc
    case READ_COLOR_RGB:           return 8;                               // seq + 2 sensores*RGB(3) + crc
    case READ_COLOR_HSV:           return 10;                              // seq + 2 sensores*(H u16 + S% + V%) + crc
    case READ_COLOR_CORRECTED_RGB: return 14;                              // seq + 2 sensores*RGB corrigido(3*u16) + crc
    case READ_COLOR_CALIBRATION:   return 26;                              // seq + 2 sensores*(minRGB + maxRGB)(6*i16) + crc
    case SET_THRESHOLD:
    case QTR_CALIBRATE:
    case CALIBRATE_COLOR:
    case ARM_EEPROM_WRITE:         return 4;                               // ACK: seq + cmd + status + crc
    default:                       return 0;
  }
}

// Descricao: Monta a mascara de status do protocolo a partir dos estados do sensor.
// Entradas: estados resumidos de linha, cor e calibracao em andamento.
// Quando usar: no escravo, para publicar estado resumido nos snapshots e no device info.
// Exemplo: `uint8_t flags = LineColorProtocol::protocolStatusFlags(true, false, true);`
uint8_t protocolStatusFlags(bool qtrCalibrated,
                            bool onLine,
                            bool colorCalibrated,
                            bool busy,
                            bool lineCalibrating,
                            bool colorCalibrating)
{
  uint8_t flags = 0;
  if (qtrCalibrated)     flags |= STATUS_QTR_CALIBRATED;
  if (onLine)            flags |= STATUS_ON_LINE;
  if (colorCalibrated)   flags |= STATUS_COLOR_CALIBRATED;
  if (busy)              flags |= STATUS_BUSY;
  if (lineCalibrating)   flags |= STATUS_LINE_CALIBRATING;
  if (colorCalibrating)  flags |= STATUS_COLOR_CALIBRATING;
  return flags;
}

// Descricao: Retorna o nome textual de um comando do protocolo.
// Entradas: `command`.
// Quando usar: em logs, menus seriais ou telas de diagnostico.
// Exemplo: `const char *name = LineColorProtocol::commandName(LineColorProtocol::READ_COLOR);`
const char *commandName(uint8_t command)
{
  switch (command) {
    case READ_COLOR: return "READ_COLOR";
    case READ_RAW_RGBW: return "READ_RAW_RGBW";
    case READ_IR_RAW: return "READ_IR_RAW";
    case SET_THRESHOLD: return "SET_THRESHOLD";
    case QTR_CALIBRATE: return "QTR_CALIBRATE";
    case CALIBRATE_COLOR: return "CALIBRATE_COLOR";
    case READ_LINE_SNAPSHOT: return "READ_LINE_SNAPSHOT";
    case READ_DEVICE_INFO: return "READ_DEVICE_INFO";
    case ARM_EEPROM_WRITE: return "ARM_EEPROM_WRITE";
    case READ_LINE_COLOR_SNAPSHOT: return "READ_LINE_COLOR_SNAPSHOT";
    case READ_STATS: return "READ_STATS";
    case READ_COLOR_RGB: return "READ_COLOR_RGB";
    case READ_COLOR_HSV: return "READ_COLOR_HSV";
    case READ_COLOR_CORRECTED_RGB: return "READ_COLOR_CORRECTED_RGB";
    case READ_COLOR_CALIBRATION: return "READ_COLOR_CALIBRATION";
    default: return "UNKNOWN";
  }
}

// Descricao: Retorna o nome textual de um codigo ACK do protocolo.
// Entradas: `status`.
// Quando usar: para mostrar no Serial o significado de um ACK sem consultar a tabela manualmente.
// Exemplo: `const char *name = LineColorProtocol::ackStatusName(LineColorProtocol::ACK_OK);`
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
