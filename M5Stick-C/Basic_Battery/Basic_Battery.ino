#include <M5StickC.h>

void setup() {
  M5.begin();
  M5.Lcd.setRotation(3);
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setTextSize(2);
}

void loop() {
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.fillScreen(BLACK);
  
  // バッテリー電圧
  float vbat = M5.Axp.GetVbatData() * 1.1 / 1000;
  M5.Lcd.printf("Bat: %.2fV\n", vbat);
  
  // バッテリー電流
  float current = M5.Axp.GetIchargeData() / 2.0;
  M5.Lcd.printf("Cur: %.0fmA\n", current);
  
  delay(1000);
}
