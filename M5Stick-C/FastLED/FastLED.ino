#include <M5StickC.h>
#include <Adafruit_NeoPixel.h>

// M5StickCのGROVEポート
#define LED_PIN 32        // GPIO32（SCL）- LED信号（黄色）
#define SWITCH_PIN 33     // GPIO33（SDA）- スイッチ入力（白色）
#define NUM_LEDS 1

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

bool lastSwitchState = HIGH;
int colorIndex = 0;

void setup() {
  M5.begin();
  M5.Lcd.setRotation(3);
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setTextSize(1);
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.println("U144 PORT.B Test");

  // スイッチピン設定（プルアップ）
  pinMode(SWITCH_PIN, INPUT_PULLUP);

  // NeoPixel初期化
  strip.begin();
  strip.setBrightness(100);
  strip.setPixelColor(0, strip.Color(255, 0, 0)); // 赤
  strip.show();

  M5.Lcd.printf("LED: GPIO%d\n", LED_PIN);
  M5.Lcd.printf("SW:  GPIO%d\n", SWITCH_PIN);
  M5.Lcd.println("Press Switch");
  
  delay(100);
  bool initState = digitalRead(SWITCH_PIN);
  M5.Lcd.printf("Init: %d\n", initState);
}

void loop() {
  M5.update();

  // スイッチ状態読み取り
  bool currentSwitchState = digitalRead(SWITCH_PIN);

  // デバッグ表示
  M5.Lcd.fillRect(0, 60, 160, 20, BLACK);
  M5.Lcd.setCursor(0, 60);
  M5.Lcd.printf("SW: %s (%d)", 
                currentSwitchState == LOW ? "PRESS" : "RELEASE",
                currentSwitchState);

  // スイッチが押された（HIGH→LOW）
  if (lastSwitchState == HIGH && currentSwitchState == LOW) {
    colorIndex++;
    if (colorIndex > 6) colorIndex = 0;

    changeLEDColor(colorIndex);

    M5.Lcd.fillRect(0, 40, 160, 20, BLACK);
    M5.Lcd.setCursor(0, 40);
    M5.Lcd.printf("Color: %d", colorIndex);
    
    delay(50); // チャタリング防止
  }

  lastSwitchState = currentSwitchState;
  delay(50);
}

void changeLEDColor(int index) {
  uint32_t color;
  
  switch (index) {
    case 0: color = strip.Color(255, 0, 0); break;     // 赤
    case 1: color = strip.Color(0, 255, 0); break;     // 緑
    case 2: color = strip.Color(0, 0, 255); break;     // 青
    case 3: color = strip.Color(255, 255, 0); break;   // 黄
    case 4: color = strip.Color(0, 255, 255); break;   // シアン
    case 5: color = strip.Color(255, 0, 255); break;   // マゼンタ
    case 6: color = strip.Color(255, 255, 255); break; // 白
  }
  
  strip.setPixelColor(0, color);
  strip.show();
}