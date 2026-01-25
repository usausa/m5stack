#include <M5Unified.h>
#include <ESP32Servo.h>
#include <Adafruit_NeoPixel.h>

// ====== ATOM LED (NeoPixel) ======
static constexpr int LED_PIN = 27;
static constexpr int LED_COUNT = 1;
Adafruit_NeoPixel pixels(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

// ====== Servo ======
Servo servo1;  // PIN26
Servo servo2;  // PIN32

const int SERVO1_PIN = 26;
const int SERVO2_PIN = 32;

int servo1Value = 90;
int servo2Value = 90;

// シリアル受信バッファ
String receivedData = "";

static void setLed(uint8_t r, uint8_t g, uint8_t b) {
  pixels.setPixelColor(0, pixels.Color(r, g, b));
  pixels.show();
}

static int clampInt(int v, int lo, int hi) {
  if (v < lo) return lo;
  if (v > hi) return hi;
  return v;
}

void processCommand(String command);

// ====== setup ======
void setup() {
  esp_log_level_set("*", ESP_LOG_NONE);

  Serial.begin(115200);
  Serial.setDebugOutput(false);

  // LED init
  pixels.begin();
  setLed(0, 0, 0);

  // Servo init
  servo1.attach(SERVO1_PIN);
  servo2.attach(SERVO2_PIN);

  servo1.write(servo1Value);
  servo2.write(servo2Value);
}

// ====== loop ======
void loop() {
  M5.update();

  // Aボタンでリセット
  if (M5.BtnA.wasPressed()) {
    servo1Value = 90;
    servo2Value = 90;
    servo1.write(servo1Value);
    servo2.write(servo2Value);
    setLed(0, 0, 0);
  }

  // シリアルコマンド処理
  while (Serial.available() > 0) {
    char c = (char)Serial.read();
    if (c == '\n' || c == '\r') {
      if (receivedData.length() > 0) {
        processCommand(receivedData);
        receivedData = "";
      }
    } else {
      receivedData += c;
      // バッファ暴走対策
      if (receivedData.length() > 200) receivedData = "";
    }
  }

  delay(10);
}

// ====== コマンド処理 ======
void processCommand(String command) {
  command.trim();
  if (command.length() == 0) return;

  // SERVO1 n
  if (command.startsWith("SERVO1")) {
    String arg = command.substring(strlen("SERVO1"));
    arg.trim();
    int n = clampInt(arg.toInt(), 0, 180);
    servo1Value = n;
    servo1.write(servo1Value);
    return;
  }

  // SERVO2 n
  if (command.startsWith("SERVO2")) {
    String arg = command.substring(strlen("SERVO2"));
    arg.trim();
    int n = clampInt(arg.toInt(), 0, 180);
    servo2Value = n;
    servo2.write(servo2Value);
    return;
  }

  // LED r g b
  if (command.startsWith("LED")) {
    int r, g, b;
    int matched = sscanf(command.c_str(), "LED %d %d %d", &r, &g, &b);
    if (matched != 3) {
      return;
    }

    r = clampInt(r, 0, 255);
    g = clampInt(g, 0, 255);
    b = clampInt(b, 0, 255);
    setLed((uint8_t)r, (uint8_t)g, (uint8_t)b);
    return;
  }
}
