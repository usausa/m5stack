#include <M5StickC.h>
#include <ESP32Servo.h>

Servo servo1;
Servo servo2;

const int SERVO1_PIN = 32;
const int SERVO2_PIN = 33;

enum ControlMode { SERVO1_ONLY, SERVO2_ONLY, BOTH_SERVOS };
ControlMode currentMode = SERVO1_ONLY;
const char* modeNames[] = {"S1", "S2", "ALL"};

bool isRotating = false;
unsigned long buttonPressTime = 0;

void setup() {
  M5.begin();
  M5.Lcd.setRotation(1);  // 縦向き（80x160）
  M5.Lcd.fillScreen(BLACK);
  
  servo1.attach(SERVO1_PIN);
  servo2.attach(SERVO2_PIN);
  servo1.write(90);
  servo2.write(90);
  
  updateDisplay();
}

void loop() {
  M5.update();
  
  // ボタンA処理
  if (M5.BtnA.wasPressed()) {
    buttonPressTime = millis();
  }
  
  if (M5.BtnA.wasReleased()) {
    unsigned long duration = millis() - buttonPressTime;
    if (duration < 500) {
      // 短押し：モード切り替え
      currentMode = (ControlMode)((currentMode + 1) % 3);
      stopServos();
      updateDisplay();
    } else if (isRotating) {
      // 長押し後離す：停止
      stopServos();
      isRotating = false;
    }
  }
  
  // 長押し：逆回転
  if (M5.BtnA.pressedFor(500) && !isRotating) {
    rotateBackward();
    isRotating = true;
  }
  
  // ボタンB：正回転
  if (M5.BtnB.isPressed()) {
    rotateForward();
    isRotating = true;
  }
  
  if (M5.BtnB.wasReleased()) {
    stopServos();
    isRotating = false;
  }
  
  delay(50);
}

void rotateForward() {
  switch (currentMode) {
    case SERVO1_ONLY:
      servo1.write(180); servo2.write(90);
      break;
    case SERVO2_ONLY:
      servo1.write(90); servo2.write(180);
      break;
    case BOTH_SERVOS:
      servo1.write(180); servo2.write(180);
      break;
  }
  showStatus("FWD", GREEN);
}

void rotateBackward() {
  switch (currentMode) {
    case SERVO1_ONLY:
      servo1.write(0); servo2.write(90);
      break;
    case SERVO2_ONLY:
      servo1.write(90); servo2.write(0);
      break;
    case BOTH_SERVOS:
      servo1.write(0); servo2.write(0);
      break;
  }
  showStatus("REV", RED);
}

void stopServos() {
  servo1.write(90);
  servo2.write(90);
  showStatus("STOP", YELLOW);
}

void updateDisplay() {
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setTextSize(2);
  
  // タイトル
  M5.Lcd.setCursor(10, 10);
  M5.Lcd.setTextColor(CYAN);
  M5.Lcd.println("Servo");
  
  // モード
  M5.Lcd.setCursor(10, 35);
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.print("Mode:");
  M5.Lcd.setTextColor(getModeColor());
  M5.Lcd.println(modeNames[currentMode]);
  
  // ガイド
  M5.Lcd.setTextSize(1);
  M5.Lcd.setTextColor(LIGHTGREY);
  M5.Lcd.setCursor(5, 100);
  M5.Lcd.println("A:Mode");
  M5.Lcd.setCursor(5, 110);
  M5.Lcd.println("Hold A:REV");
  M5.Lcd.setCursor(5, 120);
  M5.Lcd.println("Hold B:FWD");
  
  M5.Lcd.setTextSize(2);
  showStatus("STOP", YELLOW);
}

void showStatus(const char* text, uint16_t color) {
  M5.Lcd.fillRect(5, 65, 70, 20, BLACK);
  M5.Lcd.setCursor(5, 65);
  M5.Lcd.setTextColor(color);
  M5.Lcd.setTextSize(2);
  M5.Lcd.println(text);
}

uint16_t getModeColor() {
  switch (currentMode) {
    case SERVO1_ONLY: return BLUE;
    case SERVO2_ONLY: return MAGENTA;
    case BOTH_SERVOS: return GREEN;
    default: return WHITE;
  }
}