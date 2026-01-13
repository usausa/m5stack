#include <M5StickC.h>
#include <ESP32Servo.h>  // ライブラリマネージャーから "ESP32Servo" をインストール

Servo myServo;

const int SERVO_PIN = 32;  // サーボ信号線をつないだピン番号

void setup() {
  M5.begin();
  M5.Lcd.setRotation(1);
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setTextColor(WHITE, BLACK);
  M5.Lcd.setTextSize(2);
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.println("SG90 Test");

  // ESP32Servo の推奨設定
  // attach(pin, minPulseWidth, maxPulseWidth)
  // SG90 の目安: 500〜2400us 程度
  myServo.setPeriodHertz(50);  // サーボは 50Hz
  myServo.attach(SERVO_PIN, 500, 2400);

  // 初期位置
  myServo.write(90);
  delay(500);
}

void loop() {
  // 0度へ
  myServo.write(0);
  M5.Lcd.setCursor(0, 20);
  M5.Lcd.printf("Angle: %3d", 0);
  delay(5000);

  // 90度へ
  myServo.write(90);
  M5.Lcd.setCursor(0, 20);
  M5.Lcd.printf("Angle: %3d", 90);
  delay(5000);

  // 180度へ
  myServo.write(180);
  M5.Lcd.setCursor(0, 20);
  M5.Lcd.printf("Angle: %3d", 180);
  delay(5000);

  // 戻す
  myServo.write(90);
  M5.Lcd.setCursor(0, 20);
  M5.Lcd.printf("Angle: %3d", 90);
  delay(5000);
}