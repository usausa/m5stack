#include <M5StickC.h>

#define PIR_PIN 36

bool motionDetected = false;
unsigned long lastMotionTime = 0;
unsigned long motionStartTime = 0;
int detectionCount = 0;
unsigned long totalMotionTime = 0;

void setup() {
  M5.begin();
  M5.Lcd.setRotation(3);
  M5.Lcd.fillScreen(BLACK);
  
  pinMode(PIR_PIN, INPUT);
  
  drawUI();
  
  Serial.begin(115200);
  delay(2000); // センサー安定化待ち
}

void drawUI() {
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setTextSize(1);
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.setCursor(5, 5);
  M5.Lcd.println("PIR Sensor Monitor");
  M5.Lcd.drawLine(0, 15, 160, 15, WHITE);
}

void displayStatus() {
  M5.Lcd.fillRect(0, 20, 160, 60, BLACK);
  M5.Lcd.setTextSize(1);
  
  // 検知状態表示
  M5.Lcd.setCursor(5, 25);
  if (motionDetected) {
    M5.Lcd.setTextColor(RED);
    M5.Lcd.print("Status: DETECTED!");
    unsigned long duration = (millis() - motionStartTime) / 1000;
    M5.Lcd.setCursor(5, 40);
    M5.Lcd.setTextColor(YELLOW);
    M5.Lcd.printf("Duration: %lus", duration);
  } else {
    M5.Lcd.setTextColor(GREEN);
    M5.Lcd.print("Status: Standby");
  }
  
  // 検知回数
  M5.Lcd.setCursor(5, 55);
  M5.Lcd.setTextColor(CYAN);
  M5.Lcd.printf("Count: %d", detectionCount);
  
  // 最終検知時刻
  if (lastMotionTime > 0) {
    unsigned long elapsed = (millis() - lastMotionTime) / 1000;
    M5.Lcd.setCursor(5, 70);
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.printf("Last: %lus ago", elapsed);
  }
}

void loop() {
  M5.update();
  
  int pirValue = digitalRead(PIR_PIN);
  
  if (pirValue == HIGH && !motionDetected) {
    // 動き検知開始
    motionDetected = true;
    motionStartTime = millis();
    lastMotionTime = millis();
    detectionCount++;
    
    Serial.printf("Motion detected! Count: %d\n", detectionCount);
    
  } else if (pirValue == LOW && motionDetected) {
    // 動き検知終了
    motionDetected = false;
    unsigned long duration = millis() - motionStartTime;
    totalMotionTime += duration;
    
    Serial.printf("Motion ended. Duration: %lums\n", duration);
  }
  
  displayStatus();
  
  // ボタンAでリセット
  if (M5.BtnA.wasPressed()) {
    detectionCount = 0;
    totalMotionTime = 0;
    lastMotionTime = 0;
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setCursor(40, 40);
    M5.Lcd.setTextColor(YELLOW);
    M5.Lcd.println("RESET!");
    delay(1000);
    drawUI();
  }
  
  delay(100);
}