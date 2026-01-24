#include <M5Unified.h>
#include <ESP32Servo.h>

// 配線に合わせて変更してください（例）
const int SERVO1_PIN = 26; // S1: Servo Kit 180
const int SERVO2_PIN = 32; // S2: Servo Kit 360

Servo servo1;
Servo servo2;

// 90→180→90→0→90… を「押下ごと」に進めたいので、
// 初期90からスタートして、次の押下で180にしたい。
// そのため「押すたびに」適用するシーケンスをこれにする。
const int seq[] = {180, 90, 0, 90};
const int seqLen = sizeof(seq) / sizeof(seq[0]);

// どのサーボに対して何番目の指令を出すか
enum TargetServo { TARGET_S1, TARGET_S2 };
TargetServo currentTarget = TARGET_S1;

int idxS1 = 0;  // seqの次適用インデックス
int idxS2 = 0;  // seqの次適用インデックス

void printStatus(const char* reason, int appliedValue) {
  Serial.printf("[%s] Target=%s applied=%d | nextIdx S1=%d S2=%d\r\n",
                reason,
                (currentTarget == TARGET_S1) ? "S1(180)" : "S2(360)",
                appliedValue,
                idxS1, idxS2);
}

void stepBySingleButton() {
  int v;

  if (currentTarget == TARGET_S1) {
    v = seq[idxS1];
    servo1.write(v);                 // S1(180): 角度
    idxS1 = (idxS1 + 1) % seqLen;

    // S1を1ステップ進めたら、次はS2へ
    currentTarget = TARGET_S2;

    printStatus("Btn", v);
  } else {
    v = seq[idxS2];
    servo2.write(v);                 // S2(360): 90停止/180正転/0逆転（一般的）
    idxS2 = (idxS2 + 1) % seqLen;

    // S2を1ステップ進めたら、次はS1へ
    currentTarget = TARGET_S1;

    printStatus("Btn", v);
  }
}

void setup() {
  auto cfg = M5.config();
  M5.begin(cfg);

  Serial.begin(115200);
  delay(50);

  Serial.println("ATOM Lite: single-button servo sequencer");
  Serial.println("Start: S1=90, S2=90");
  Serial.println("Press order: S1:180,90,0,90, S2:180,90,0,90, ...");

  servo1.attach(SERVO1_PIN);
  servo2.attach(SERVO2_PIN);

  // 初期値
  servo1.write(90);
  servo2.write(90);

  // 次の押下でそれぞれ 180 になるように idx=0 のまま開始
  idxS1 = 0;
  idxS2 = 0;
  currentTarget = TARGET_S1;

  Serial.println("[Init] Ready");
}

void loop() {
  M5.update();

  // 押した瞬間だけ反応
  if (M5.BtnA.wasPressed()) {   // ATOM Liteの唯一のボタン
    stepBySingleButton();
  }

  delay(10);
}