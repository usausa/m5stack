#include <M5StickC.h>
#include <driver/ledc.h>

#define SPEAKER_PIN 26
#define LEDC_CHANNEL 0

void setup() {
  M5.begin();
  M5.Lcd.setRotation(3);
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.println("SPK HAT Test");
  M5.Lcd.println("Press Button A");
  
  // LEDCの初期化
  ledcAttachChannel(SPEAKER_PIN, 1000, 8, LEDC_CHANNEL);
}

void loop() {
  M5.update();
  
  if (M5.BtnA.wasPressed()) {
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setCursor(0, 0);
    M5.Lcd.println("Playing Beep!");
    playBeep(1000, 200);
  }
  
  delay(10);
}

void playBeep(int frequency, int duration) {
  ledcChangeFrequency(SPEAKER_PIN, frequency, 8);
  ledcWrite(SPEAKER_PIN, 128);
  delay(duration);
  ledcWrite(SPEAKER_PIN, 0);
}