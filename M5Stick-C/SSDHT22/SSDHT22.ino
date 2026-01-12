#include <M5StickC.h>
// DHT sensor library by Adafruit
#include <DHT.h>

#define DHTPIN 33
#define DHTTYPE DHT22

DHT dht(DHTPIN, DHTTYPE);

void setup() {
  M5.begin();
  M5.Lcd.setRotation(1);
  M5.Lcd.fillScreen(BLACK);
  
  dht.begin();
  delay(1000);
}

void loop() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  
  if (isnan(h) || isnan(t)) {
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setCursor(10, 30);
    M5.Lcd.println("Error!");
    delay(2000);
    return;
  }
  
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setTextSize(2);
  
  // 温度
  M5.Lcd.setTextColor(RED);
  M5.Lcd.setCursor(10, 20);
  M5.Lcd.printf("%.1fC", t);
  
  // 湿度
  M5.Lcd.setTextColor(CYAN);
  M5.Lcd.setCursor(10, 50);
  M5.Lcd.printf("%.1f%%", h);
  
  delay(2000);
}