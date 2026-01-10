#include <M5StickC.h>
#include <driver/ledc.h>

#define SPEAKER_PIN 26

// 音階の周波数定義
#define NOTE_C4  262
#define NOTE_D4  294
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_G4  392
#define NOTE_A4  440
#define NOTE_B4  494
#define NOTE_C5  523
#define NOTE_REST 0

// メロディデータ
int melody[][2] = {
  {NOTE_C4, 300},
  {NOTE_D4, 300},
  {NOTE_E4, 300},
  {NOTE_F4, 300},
  {NOTE_G4, 300},
  {NOTE_A4, 300},
  {NOTE_B4, 300},
  {NOTE_C5, 300}
};

void setup() {
  M5.begin();
  M5.Lcd.setRotation(3);
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setTextSize(2);
  M5.Lcd.setCursor(10, 20);
  M5.Lcd.println("SPK HAT");
  M5.Lcd.setTextSize(1);
  M5.Lcd.setCursor(5, 45);
  M5.Lcd.println("BtnA: Melody");
  
  ledcAttachChannel(SPEAKER_PIN, 1000, 8, 0);
}

void loop() {
  M5.update();
  
  if (M5.BtnA.wasPressed()) {
    playMelody();
  }
  
  delay(10);
}

void playMelody() {
  int melodyLength = sizeof(melody) / sizeof(melody[0]);
  
  for (int i = 0; i < melodyLength; i++) {
    playTone(melody[i][0], melody[i][1]);
    delay(50);
  }
}

void playTone(int frequency, int duration) {
  if (frequency > 0) {
    ledcChangeFrequency(SPEAKER_PIN, frequency, 8);
    ledcWrite(SPEAKER_PIN, 128);
    delay(duration);
    ledcWrite(SPEAKER_PIN, 0);
  } else {
    delay(duration);
  }
}