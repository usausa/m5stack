#include <M5StickC.h>
#include <ESP32Servo.h>

// Servo Kit 180（位置指定サーボ）想定
Servo servo1;
Servo servo2;

const int SERVO1_PIN = 32;
const int SERVO2_PIN = 33;

// Aボタンで対象サーボを切替
enum TargetServo { TARGET_S1, TARGET_S2 };
TargetServo currentTarget = TARGET_S1;
const char* targetNames[] = {"S1", "S2"};

// Bボタンで 90→180→90→0→90… と切替（状態を持つ）
const int posSeq[] = {90, 180, 90, 0};
const int posSeqLen = sizeof(posSeq) / sizeof(posSeq[0]);
int posIndex = 0; // 現在のシーケンス位置（起動時は90）

void applyPositionToCurrentTarget() {
  int pos = posSeq[posIndex];
  if (currentTarget == TARGET_S1) {
    servo1.write(pos);
  } else {
    servo2.write(pos);
  }
}

void updateDisplay() {
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setRotation(1);
  M5.Lcd.setTextColor(CYAN);
  M5.Lcd.setTextSize(2);
  M5.Lcd.setCursor(10, 10);
  M5.Lcd.println("Servo180");

  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.setCursor(10, 35);
  M5.Lcd.print("Target:");
  M5.Lcd.setTextColor(currentTarget == TARGET_S1 ? BLUE : MAGENTA);
  M5.Lcd.println(targetNames[currentTarget]);

  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.setCursor(10, 60);
  M5.Lcd.print("Pos:");
  M5.Lcd.setTextColor(YELLOW);
  M5.Lcd.println(posSeq[posIndex]);

  M5.Lcd.setTextSize(1);
  M5.Lcd.setTextColor(LIGHTGREY);
  M5.Lcd.setCursor(5, 105);
  M5.Lcd.println("A: Switch servo");
  M5.Lcd.setCursor(5, 118);
  M5.Lcd.println("B: 90-180-90-0...");
}

void setup() {
  M5.begin();
  M5.Lcd.setRotation(1);
  M5.Lcd.fillScreen(BLACK);

  servo1.attach(SERVO1_PIN);
  servo2.attach(SERVO2_PIN);

  // 初期位置は両方90度
  servo1.write(90);
  servo2.write(90);

  // シーケンス初期値も90
  posIndex = 0;

  updateDisplay();
}

void loop() {
  M5.update();

  // Aボタン：対象サーボ切り替え
  if (M5.BtnA.wasPressed()) {
    currentTarget = (currentTarget == TARGET_S1) ? TARGET_S2 : TARGET_S1;
    updateDisplay();
  }

  // Bボタン：位置を 90→180→90→0→… と進めて反映
  if (M5.BtnB.wasPressed()) {
    posIndex = (posIndex + 1) % posSeqLen;
    applyPositionToCurrentTarget();
    updateDisplay();
  }

  delay(20);
}