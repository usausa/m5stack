#include <Arduino.h>
#include <Wire.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <BLEAdvertising.h>
#include <Adafruit_NeoPixel.h>

// ---- ATOM Lite 内蔵RGB LED ----
#define LED_PIN   27
#define LED_COUNT 1
Adafruit_NeoPixel pixels(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

// ---- BLE設定 ----
static const char*    ADV_NAME = "ATOM-ENV3";
static const uint16_t MFG_ID_TEST = 0xFFFF;
static const uint32_t ADV_UPDATE_MS = 2000;

// ---- Nordic UART Service (NUS) UUID ----
static BLEUUID NUS_SERVICE_UUID("6E400001-B5A3-F393-E0A9-E50E24DCCA9E");
static BLEUUID NUS_RX_UUID     ("6E400002-B5A3-F393-E0A9-E50E24DCCA9E"); // Central -> ATOM (Write)
static BLEUUID NUS_TX_UUID     ("6E400003-B5A3-F393-E0A9-E50E24DCCA9E"); // ATOM -> Central (Notify)

BLECharacteristic* txChar = nullptr;
bool deviceConnected = false;

// ---- I2C設定（ATOM Lite Groveで一般的）----
static const int I2C_SDA = 26;
static const int I2C_SCL = 32;
static const uint32_t I2C_CLOCK = 100000;

// ---- SHT3X ----
static const uint8_t SHT_ADDR = 0x44;
static const uint16_t CMD_SOFT_RESET  = 0x30A2;
static const uint16_t CMD_READ_STATUS = 0xF32D;
static const uint16_t CMD_MEAS_HIGHREP_NOCS = 0x2400;

// ---- 最新の温湿度データ ----
static float lastTemp = -999.0f;
static float lastHumi = -999.0f;

// ---- LED制御 ----
static void setLed(uint8_t r, uint8_t g, uint8_t b) {
  pixels.setPixelColor(0, pixels.Color(r, g, b));
  pixels.show();
}

// ---- UART送信 ----
static void sendTx(const String& s) {
  if (!deviceConnected || txChar == nullptr) return;
  txChar->setValue((uint8_t*)s.c_str(), s.length());
  txChar->notify();
}

static String trimCopy(String s) {
  s.trim();
  return s;
}

// ---- SHT3X関数 ----
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
  uint8_t crc = 0xFF;
  for (int i = 0; i < len; i++) {
    crc ^= data[i];
    for (int b = 0; b < 8; b++) {
      if (crc & 0x80) crc = (uint8_t)((crc << 1) ^ 0x31);
      else            crc = (uint8_t)(crc << 1);
    }
  }
  return crc;
}

static bool readSHT3X(float& tempC, float& humi) {
  if (!writeCmd16(SHT_ADDR, CMD_MEAS_HIGHREP_NOCS)) return false;
  delay(20);
  uint8_t b[6];
  int got = readBytes(SHT_ADDR, b, 6);
  if (got != 6) return false;
  if (crc8_sht(&b[0], 2) != b[2]) return false;
  if (crc8_sht(&b[3], 2) != b[5]) return false;
  uint16_t rawT = ((uint16_t)b[0] << 8) | b[1];
  uint16_t rawH = ((uint16_t)b[3] << 8) | b[4];
  tempC = -45.0f + 175.0f * ((float)rawT / 65535.0f);
  humi  = 100.0f * ((float)rawH / 65535.0f);
  if (!isfinite(tempC) || !isfinite(humi)) return false;
  if (tempC < -40.0f || tempC > 125.0f) return false;
  if (humi < 0.0f || humi > 100.0f) return false;
  return true;
}

// ---- Manufacturer Data作成 ----
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
  advData.setCompleteServices(BLEUUID(NUS_SERVICE_UUID));
  advData.setManufacturerData(makeMfgString(t_x100, h_x100));
  
  BLEAdvertising* adv = BLEDevice::getAdvertising();
  adv->stop();
  adv->setAdvertisementData(advData);
  
  // Scan Response側にも名前を入れておく
  BLEAdvertisementData scanData;
  scanData.setName(ADV_NAME);
  adv->setScanResponseData(scanData);
  
  adv->start();
}

// ---- コマンド処理 ----
// 受信コマンド例:
//   RED / GREEN / BLUE / OFF / WHITE
//   RGB 255 0 128
//   TEMP (温度取得)
static void handleCommand(String cmd) {
  cmd = trimCopy(cmd);
  if (cmd.length() == 0) return;
  String u = cmd;
  u.toUpperCase();
  
  // LED制御コマンド
  if (u == "RED")   { setLed(255, 0, 0);     return; }
  if (u == "GREEN") { setLed(0, 255, 0);     return; }
  if (u == "BLUE")  { setLed(0, 0, 255);     return; }
  if (u == "WHITE") { setLed(255, 255, 255); return; }
  if (u == "OFF")   { setLed(0, 0, 0);       return; }
  
  // RGB r g b
  if (u.startsWith("RGB")) {
    int r, g, b;
    if (sscanf(cmd.c_str(), "RGB %d %d %d", &r, &g, &b) == 3) {
      r = constrain(r, 0, 255);
      g = constrain(g, 0, 255);
      b = constrain(b, 0, 255);
      setLed((uint8_t)r, (uint8_t)g, (uint8_t)b);
    }
    return;
  }
  
  // 温度取得コマンド
  if (u == "TEMP") {
    float t, h;
    if (readSHT3X(t, h)) {
      lastTemp = t;
      lastHumi = h;
      char buf[64];
      snprintf(buf, sizeof(buf), "TEMP %.2f HUMI %.2f\n", t, h);
      sendTx(buf);
    }
    return;
  }
}

// ---- BLEコールバック ----
class ServerCB : public BLEServerCallbacks {
  void onConnect(BLEServer* s) override {
    deviceConnected = true;
    Serial.println("BLE connected");
  }
  void onDisconnect(BLEServer* s) override {
    deviceConnected = false;
    Serial.println("BLE disconnected");
    // 切断後に再広告（再接続できるように）
    delay(100);
    BLEDevice::startAdvertising();
  }
};

class RxCB : public BLECharacteristicCallbacks {
  String buf;
  void onWrite(BLECharacteristic* c) override {
    String v = c->getValue();
    if (v.length() == 0) return;
    for (size_t i = 0; i < (size_t)v.length(); i++) {
      char ch = v[i];
      if (ch == '\r') continue;
      if (ch == '\n') {
        handleCommand(buf);
        buf = "";
      } else {
        if (buf.length() < 200) buf += ch;
      }
    }
  }
};

void setup() {
  Serial.begin(115200);
  delay(500);
  
  // LED初期化
  pixels.begin();
  setLed(0, 0, 0);
  
  // I2C初期化
  Wire.begin(I2C_SDA, I2C_SCL);
  Wire.setClock(I2C_CLOCK);
  Serial.printf("I2C SDA=%d SCL=%d clock=%u addr=0x%02X\n",
                I2C_SDA, I2C_SCL, (unsigned)I2C_CLOCK, SHT_ADDR);
  
  // SHT3X soft reset
  if (writeCmd16(SHT_ADDR, CMD_SOFT_RESET)) {
    Serial.println("SHT soft reset OK");
    delay(20);
  } else {
    Serial.println("SHT soft reset FAILED");
  }
  
  // BLE初期化
  BLEDevice::init(ADV_NAME);
  BLEServer* server = BLEDevice::createServer();
  server->setCallbacks(new ServerCB());
  
  // NUS Service作成
  BLEService* service = server->createService(NUS_SERVICE_UUID);
  
  txChar = service->createCharacteristic(
    NUS_TX_UUID,
    BLECharacteristic::PROPERTY_NOTIFY
  );
  txChar->addDescriptor(new BLE2902());
  
  BLECharacteristic* rxChar = service->createCharacteristic(
    NUS_RX_UUID,
    BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_WRITE_NR
  );
  rxChar->setCallbacks(new RxCB());
  
  service->start();
  
  // 初回広告（無効値）
  updateAdvertising((int16_t)-32768, (uint16_t)65535);
  
  Serial.print("Advertising as: ");
  Serial.println(ADV_NAME);
  
  setLed(0, 0, 20); // 起動表示（薄青）
}

void loop() {
  static uint32_t last = 0;
  uint32_t now = millis();
  
  // 定期的に温湿度を読んで広告更新
  if (now - last >= ADV_UPDATE_MS) {
    last = now;
    
    float t, h;
    if (readSHT3X(t, h)) {
      lastTemp = t;
      lastHumi = h;
      int16_t t_x100 = (int16_t)lroundf(t * 100.0f);
      uint16_t h_x100 = (uint16_t)lroundf(h * 100.0f);
      Serial.printf("T=%.2fC H=%.2f%% -> t_x100=%d h_x100=%u\n", t, h, t_x100, h_x100);
      updateAdvertising(t_x100, h_x100);
    } else {
      Serial.println("SHT read failed (skip adv update)");
    }
  }
  
  delay(50);
}