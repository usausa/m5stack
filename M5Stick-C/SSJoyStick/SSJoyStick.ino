#include <M5StickC.h>

#define ANALOG_PIN1 33
#define ANALOG_PIN2 32

void setup() {
  M5.begin();
  M5.Lcd.setRotation(3);
  M5.Lcd.fillScreen(BLACK);
  
  // プルダウンを有効化（内部抵抗）
  pinMode(ANALOG_PIN1, INPUT_PULLDOWN);
  pinMode(ANALOG_PIN2, INPUT_PULLDOWN);
  
  analogReadResolution(12);
  analogSetAttenuation(ADC_11db);
}

void loop() {
  int value33 = analogRead(ANALOG_PIN1);
  int value32 = analogRead(ANALOG_PIN2);
  
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setTextSize(2);
  
  M5.Lcd.setTextColor(CYAN);
  M5.Lcd.setCursor(10, 15);
  M5.Lcd.printf("33:%4d", value33);
  
  M5.Lcd.setTextColor(YELLOW);
  M5.Lcd.setCursor(10, 45);
  M5.Lcd.printf("32:%4d", value32);
  
  delay(100);
}