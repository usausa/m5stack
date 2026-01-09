#include <M5StickC.h>

void setup() {
  M5.begin();
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.setTextSize(2);
  M5.Lcd.println("Hello");
  M5.Lcd.println("M5Stick");
}

void loop() {
  // 何もしない
}
