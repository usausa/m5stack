#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <Adafruit_NeoPixel.h>

// ---- ATOM Lite 内蔵RGB LED ----
#define LED_PIN   27     // ATOM Liteは多くの個体でGPIO27
#define LED_COUNT 1
Adafruit_NeoPixel pixels(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

// ---- Nordic UART Service (NUS) UUID ----
static BLEUUID NUS_SERVICE_UUID("6E400001-B5A3-F393-E0A9-E50E24DCCA9E");
static BLEUUID NUS_RX_UUID     ("6E400002-B5A3-F393-E0A9-E50E24DCCA9E"); // Central -> ATOM (Write)
static BLEUUID NUS_TX_UUID     ("6E400003-B5A3-F393-E0A9-E50E24DCCA9E"); // ATOM -> Central (Notify)

BLECharacteristic* txChar = nullptr;
bool deviceConnected = false;

static void setLed(uint8_t r, uint8_t g, uint8_t b) {
  pixels.setPixelColor(0, pixels.Color(r, g, b));
  pixels.show();
}

static void sendTx(const String& s) {
  if (!deviceConnected || txChar == nullptr) return;
  txChar->setValue((uint8_t*)s.c_str(), s.length());
  txChar->notify();
}

static String trimCopy(String s) {
  s.trim();
  return s;
}

// 受信コマンド例:
//   RED / GREEN / BLUE / OFF / WHITE
//   RGB 255 0 128
static void handleCommand(String cmd) {
  cmd = trimCopy(cmd);
  if (cmd.length() == 0) return;

  String u = cmd;
  u.toUpperCase();

  if (u == "RED")   { setLed(255, 0, 0);   sendTx("OK RED\n");   return; }
  if (u == "GREEN") { setLed(0, 255, 0);   sendTx("OK GREEN\n"); return; }
  if (u == "BLUE")  { setLed(0, 0, 255);   sendTx("OK BLUE\n");  return; }
  if (u == "WHITE") { setLed(255, 255, 255); sendTx("OK WHITE\n"); return; }
  if (u == "OFF")   { setLed(0, 0, 0);     sendTx("OK OFF\n");   return; }

  // RGB r g b
  if (u.startsWith("RGB")) {
    int r, g, b;
    // "RGB 1 2 3" を想定
    if (sscanf(cmd.c_str(), "RGB %d %d %d", &r, &g, &b) == 3) {
      r = constrain(r, 0, 255);
      g = constrain(g, 0, 255);
      b = constrain(b, 0, 255);
      setLed((uint8_t)r, (uint8_t)g, (uint8_t)b);
      sendTx("OK RGB\n");
    } else {
      sendTx("ERR Usage: RGB <r> <g> <b>\n");
    }
    return;
  }

  sendTx("ERR Unknown cmd. Try: RED/GREEN/BLUE/WHITE/OFF/RGB r g b\n");
}

class ServerCB : public BLEServerCallbacks {
  void onConnect(BLEServer* s) override {
    deviceConnected = true;
    Serial.println("BLE connected");
    sendTx("Hello from ATOM Lite\n");
  }
  void onDisconnect(BLEServer* s) override {
    deviceConnected = false;
    Serial.println("BLE disconnected");
    // 切断後に再広告（再接続できるように）
    BLEDevice::startAdvertising();
  }
};

class RxCB : public BLECharacteristicCallbacks {
  String buf;
  void onWrite(BLECharacteristic* c) override {
    String v = c->getValue();   // ← std::string ではなく String
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
  delay(200);

  pixels.begin();
  setLed(0, 0, 0);

  BLEDevice::init("ATOM-UART");
  BLEServer* server = BLEDevice::createServer();
  server->setCallbacks(new ServerCB());

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

  BLEAdvertising* adv = BLEDevice::getAdvertising();
  adv->addServiceUUID(NUS_SERVICE_UUID);
  adv->setScanResponse(true);
  adv->start();

  Serial.println("Advertising as: ATOM-UART");
  setLed(0, 0, 20); // 起動表示（薄青）
}

void loop() {
  delay(50);
}
