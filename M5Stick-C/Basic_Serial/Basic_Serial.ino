#include <M5StickC.h>

// ESP32のログレベル設定に必要なヘッダー (M5.begin()実行前に設定)
#include "esp32-hal-log.h" 

// setup()の前に実行されるコンストラクターでの設定
// 実行前にロギングレベルを設定
__attribute__((constructor)) void pre_setup() {
  esp_log_level_set("*", (esp_log_level_t)ESP_LOG_NONE);
}

void setup() {
  // M5StickCの初期化
  // M5.begin(bool LCDEnable=true, bool PowerEnable=true, bool SerialEnable=true);
  // 第三引数の SerialEnable を false にすることで、M5ライブラリによる
  // 独自のシリアル初期化やデバッグ出力を抑制します。
  M5.begin(true, true, false); 

  // ★重要★: M5.begin()でSerialEnable=falseにしたため、
  // エコーバックに使用するシリアル通信はここで明示的に開始する必要があります。
  Serial.begin(115200);

  // M5StickCの画面にメッセージを表示
  M5.Lcd.setRotation(1); 
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setTextSize(2);
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.print("Ready");
}

void loop() {
  if (Serial.available() > 0) {
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setCursor(0, 0);
    // シリアルポートにデータが到着している間、繰り返す
    while (Serial.available() > 0) {
      // 受信バッファから1バイト（1文字）読み取る
      char receivedChar = Serial.read();
      // 読み取った文字をそのままシリアルポートに書き戻す（エコーバック）
      Serial.write(receivedChar);
    }
  }

  // 負荷軽減のためのウェイト
  delay(1);
}
