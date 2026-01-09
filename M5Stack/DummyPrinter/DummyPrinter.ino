#include <M5Stack.h>
#include "BluetoothSerial.h"
#include "esp_gap_bt_api.h"

BluetoothSerial SerialBT;

bool ok = true;

void setup() {
  M5.begin();
  M5.Lcd.setTextSize(2);

  //SerialBT.setPin("8888");
  SerialBT.begin("DummyPrinter");

  clear();
}

void loop() {
  M5.update();

  if (SerialBT.available()) {
    String msg = SerialBT.readString();
    M5.Lcd.println("Received: " + msg);
    if (ok) {
      SerialBT.println("OK");
    } else {
      SerialBT.println("ERR");
    }
  }

  if (M5.BtnB.wasPressed()) {
    ok = !ok;
  }

  if (M5.BtnC.wasPressed()) {
    clear();
  }
  
  delay(20);
}

void clear() {
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setTextColor(ORANGE);
  M5.Lcd.println("[Dummy Printer]");
  M5.Lcd.setTextColor(WHITE);
}
