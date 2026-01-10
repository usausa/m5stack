#include <M5StickC.h>
#include <ESP32Servo.h>

Servo servo360;

// サーボ接続ピン（M5StickCのGroveポート）
const int SERVO_PIN = 32;

void setup() {
  M5.begin();
  M5.Lcd.setRotation(3);
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.println("Servo 360 Test");
  
  // サーボの初期化
  servo360.attach(SERVO_PIN, 500, 2400);  // ピン, min, max パルス幅
  servo360.write(90);  // 停止位置
  
  delay(1000);
}

void loop() {
  M5.update();
  
  // ボタンAで正転
  if (M5.BtnA.isPressed()) {
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setCursor(0, 0);
    M5.Lcd.println("Forward");
    servo360.write(0);  // 最高速で正転
  }
  
  // ボタンBで停止
  else if (M5.BtnB.isPressed()) {
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setCursor(0, 0);
    M5.Lcd.println("Stop");
    servo360.write(90);  // 停止
  }
  
  // ボタン押されていない時は逆転
  else if (!M5.BtnA.isPressed() && !M5.BtnB.isPressed()) {
    static unsigned long lastChange = 0;
    if (millis() - lastChange > 2000) {
      M5.Lcd.fillScreen(BLACK);
      M5.Lcd.setCursor(0, 0);
      M5.Lcd.println("Reverse");
      servo360.write(180);  // 最高速で逆転
      lastChange = millis();
    }
  }
  
  delay(50);
}