#include <M5StickC.h>
#include <ESP32Servo.h>

// サーボオブジェクト
Servo servo1;  // PIN32
Servo servo2;  // PIN33

// サーボピン定義
const int SERVO1_PIN = 32;
const int SERVO2_PIN = 33;

// サーボの現在値
int servo1Value = 90;  // 初期値：停止
int servo2Value = 90;  // 初期値：停止

// シリアル受信バッファ
String receivedData = "";

void setup() {
  esp_log_level_set("*", ESP_LOG_NONE);

  // M5StickC初期化
  M5.begin(true, true, false);  // LCD=true, Power=true, Serial=false
  // シリアル通信初期化
  Serial.begin(115200);
  Serial.setDebugOutput(false);

  M5.Lcd.setRotation(1);
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setTextSize(2);
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.println("Servo Control");
  
  // サーボ初期化
  servo1.attach(SERVO1_PIN);
  servo2.attach(SERVO2_PIN);
  
  // 初期位置設定（停止）
  servo1.write(servo1Value);
  servo2.write(servo2Value);
  
  // 初期表示
  updateDisplay();
}

void loop() {
  M5.update();
  
  // Aボタンで緊急停止
  if (M5.BtnA.wasPressed()) {
    emergencyStop();
  }
  
  // シリアルコマンド処理
  while (Serial.available() > 0) {
    char c = Serial.read();
    
    if (c == '\n' || c == '\r') {
      if (receivedData.length() > 0) {
        processCommand(receivedData);
        receivedData = "";
      }
    } else {
      receivedData += c;
    }
  }
  
  delay(10);
}

// コマンド処理
void processCommand(String command) {
  command.trim();
  
  // +Wコマンドのチェック
  if (command.startsWith("+W ")) {
    String params = command.substring(3);
    
    // カンマで分割
    int commaIndex = params.indexOf(',');
    if (commaIndex > 0) {
      String servoStr = params.substring(0, commaIndex);
      String speedStr = params.substring(commaIndex + 1);
      
      int servoNum = servoStr.toInt();
      int speed = speedStr.toInt();
      
      // 速度の範囲チェック
      if (speed < 0) speed = 0;
      if (speed > 180) speed = 180;
      
      // サーボ制御
      if (servoNum == 1) {
        servo1Value = speed;
        servo1.write(servo1Value);
        Serial.print("Servo1(PIN32) set to: ");
        Serial.println(servo1Value);
      } else if (servoNum == 2) {
        servo2Value = speed;
        servo2.write(servo2Value);
        Serial.print("Servo2(PIN33) set to: ");
        Serial.println(servo2Value);
      } else {
        Serial.println("Error: Invalid servo number (use 1 or 2)");
        return;
      }
      
      // 画面更新
      updateDisplay();
    } else {
      Serial.println("Error: Invalid command format");
    }
  } else {
    Serial.println("Error: Unknown command");
  }
}

// 緊急停止
void emergencyStop() {
  servo1Value = 90;
  servo2Value = 90;
  servo1.write(servo1Value);
  servo2.write(servo2Value);
  
  // 画面に停止表示
  M5.Lcd.fillScreen(RED);
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.setCursor(10, 10);
  M5.Lcd.println("STOPPED!");
  delay(500);
  
  updateDisplay();
}

// 画面更新
void updateDisplay() {
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.setTextSize(1);
  
  // タイトル
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.println("Servo Control");
  M5.Lcd.drawLine(0, 12, 160, 12, WHITE);
  
  // Servo1の表示
  M5.Lcd.setCursor(0, 20);
  M5.Lcd.print("S1(32):");
  M5.Lcd.setTextSize(2);
  M5.Lcd.setCursor(50, 18);
  M5.Lcd.printf("%3d", servo1Value);
  
  // 状態表示
  M5.Lcd.setTextSize(1);
  M5.Lcd.setCursor(100, 20);
  if (servo1Value < 85) {
    M5.Lcd.print("BACK");
  } else if (servo1Value > 95) {
    M5.Lcd.print("FWD");
  } else {
    M5.Lcd.print("STOP");
  }
  
  // Servo2の表示
  M5.Lcd.setTextSize(1);
  M5.Lcd.setCursor(0, 40);
  M5.Lcd.print("S2(33):");
  M5.Lcd.setTextSize(2);
  M5.Lcd.setCursor(50, 38);
  M5.Lcd.printf("%3d", servo2Value);
  
  // 状態表示
  M5.Lcd.setTextSize(1);
  M5.Lcd.setCursor(100, 40);
  if (servo2Value < 85) {
    M5.Lcd.print("BACK");
  } else if (servo2Value > 95) {
    M5.Lcd.print("FWD");
  } else {
    M5.Lcd.print("STOP");
  }
  
  // 操作説明
  M5.Lcd.setCursor(0, 60);
  M5.Lcd.println("BtnA: STOP");
}
