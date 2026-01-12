#include <M5StickC.h>

// Grove LEDを接続するピン（M5StickCのGroveポート）
#define LED_PIN 33  // GPIO32 または GPIO33

void setup() {
  M5.begin();
  M5.Lcd.setRotation(3);
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.println("Grove LED Test");
  
  pinMode(LED_PIN, OUTPUT);
}

void loop() {
  // LED点灯
  digitalWrite(LED_PIN, HIGH);
  M5.Lcd.fillScreen(RED);
  M5.Lcd.setCursor(10, 30);
  M5.Lcd.println("LED ON");
  delay(1000);
  
  // LED消灯
  digitalWrite(LED_PIN, LOW);
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setCursor(10, 30);
  M5.Lcd.println("LED OFF");
  delay(1000);
}