#include <Arduino.h>
#include <Wire.h>

#include <BLEDevice.h>
#include <BLEAdvertising.h>

// ---- BLE設定 ----
static const char*    ADV_NAME = "ATOM-ENV3";
static const uint16_t MFG_ID_TEST = 0xFFFF;
static const uint32_t ADV_UPDATE_MS = 2000;

// ---- I2C設定（ATOM Lite Groveで一般的）----
static const int I2C_SDA = 26;
static const int I2C_SCL = 32;
static const uint32_t I2C_CLOCK = 100000;

// ---- SHT3X ----
static const uint8_t SHT_ADDR = 0x44;

// SHT3X コマンド
static const uint16_t CMD_SOFT_RESET  = 0x30A2;
static const uint16_t CMD_READ_STATUS = 0xF32D;
static const uint16_t CMD_MEAS_HIGHREP_NOCS = 0x2400; // single shot, high repeatability, no clock stretching

static bool writeCmd16(uint8_t addr, uint16_t cmd) {
  Wire.beginTransmission(addr);
  Wire.write((uint8_t)(cmd >> 8));
  Wire.write((uint8_t)(cmd & 0xFF));
  return (Wire.endTransmission() == 0);
}

static int readBytes(uint8_t addr, uint8_t* buf, size_t n) {
  int got = Wire.requestFrom((int)addr, (int)n);
  for (int i = 0; i < got; i++) buf[i] = Wire.read();
  return got;
}

static uint8_t crc8_sht(const uint8_t* data, int len) {
  // SHT3X CRC-8: poly 0x31, init 0xFF
  uint8_t crc = 0xFF;
  for (int i = 0; i < len; i++) {
    crc ^= data[i];
    for (int b = 0; b < 8; b++) {
      if (crc & 0x80) crc = (uint8_t)((crc << 1) ^ 0x31);
      else           crc = (uint8_t)(crc << 1);
    }
  }
  return crc;
}

static bool readSHT3X(float& tempC, float& humi) {
  // 測定開始
  if (!writeCmd16(SHT_ADDR, CMD_MEAS_HIGHREP_NOCS)) return false;
  delay(20);

  uint8_t b[6];
  int got = readBytes(SHT_ADDR, b, 6);
  if (got != 6) return false;

  // CRCチェック
  if (crc8_sht(&b[0], 2) != b[2]) return false;
  if (crc8_sht(&b[3], 2) != b[5]) return false;

  uint16_t rawT = ((uint16_t)b[0] << 8) | b[1];
  uint16_t rawH = ((uint16_t)b[3] << 8) | b[4];

  // 変換式（Sensirion公式）
  tempC = -45.0f + 175.0f * ((float)rawT / 65535.0f);
  humi  = 100.0f * ((float)rawH / 65535.0f);

  if (!isfinite(tempC) || !isfinite(humi)) return false;
  if (tempC < -40.0f || tempC > 125.0f) return false;
  if (humi < 0.0f || humi > 100.0f) return false;

  return true;
}

// Manufacturer Data: [FFFF]['E''3'][temp int16LE][hum uint16LE]
static String makeMfgString(int16_t temp_c_x100, uint16_t humi_x100) {
  uint8_t d[8];
  d[0] = (uint8_t)(MFG_ID_TEST & 0xFF);
  d[1] = (uint8_t)(MFG_ID_TEST >> 8);
  d[2] = 'E';
  d[3] = '3';
  d[4] = (uint8_t)(temp_c_x100 & 0xFF);
  d[5] = (uint8_t)((temp_c_x100 >> 8) & 0xFF);
  d[6] = (uint8_t)(humi_x100 & 0xFF);
  d[7] = (uint8_t)((humi_x100 >> 8) & 0xFF);

  String s;
  s.reserve(sizeof(d));
  for (size_t i = 0; i < sizeof(d); i++) s += (char)d[i];
  return s;
}

static void updateAdvertising(int16_t t_x100, uint16_t h_x100) {
  BLEAdvertisementData advData;
  advData.setName(ADV_NAME);
  advData.setManufacturerData(makeMfgString(t_x100, h_x100));

  BLEAdvertising* adv = BLEDevice::getAdvertising();
  adv->stop();
  adv->setAdvertisementData(advData);
  adv->setScanResponse(false);
  adv->start();
}

void setup() {
  Serial.begin(115200);
  delay(500);

  Wire.begin(I2C_SDA, I2C_SCL);
  Wire.setClock(I2C_CLOCK);

  Serial.printf("I2C SDA=%d SCL=%d clock=%u addr=0x%02X\n",
                I2C_SDA, I2C_SCL, (unsigned)I2C_CLOCK, SHT_ADDR);

  // SHT3X soft reset（任意だが安定化に効くことあり）
  if (writeCmd16(SHT_ADDR, CMD_SOFT_RESET)) {
    Serial.println("SHT soft reset OK");
    delay(20);
  } else {
    Serial.println("SHT soft reset FAILED");
  }

  // BLE初期化（サービスなし）
  BLEDevice::init(ADV_NAME);
  BLEAdvertising* adv = BLEDevice::getAdvertising();
  adv->setAdvertisementType(ADV_TYPE_IND);

  // 初回は無効値を広告
  updateAdvertising((int16_t)-32768, (uint16_t)65535);
  Serial.println("BLE advertising started");
}

void loop() {
  static uint32_t last = 0;
  uint32_t now = millis();
  if (now - last < ADV_UPDATE_MS) {
    delay(10);
    return;
  }
  last = now;

  float t, h;
  if (!readSHT3X(t, h)) {
    Serial.println("SHT read failed (skip adv update)");
    return;
  }

  int16_t t_x100 = (int16_t)lroundf(t * 100.0f);
  uint16_t h_x100 = (uint16_t)lroundf(h * 100.0f);

  Serial.printf("T=%.2fC H=%.2f%% -> t_x100=%d h_x100=%u\n", t, h, t_x100, h_x100);
  updateAdvertising(t_x100, h_x100);
}