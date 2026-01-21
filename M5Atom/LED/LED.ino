#include <Adafruit_NeoPixel.h>

#define LED_PIN   27      // ATOM LITEのRGB LED
#define LED_COUNT 1

Adafruit_NeoPixel pixels(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  Serial.begin(115200);
  delay(200);

  pixels.begin();
  pixels.clear();
  pixels.show();

  Serial.println("ATOM LITE test start");
}

void loop() {
  // 赤
  pixels.setPixelColor(0, pixels.Color(50, 0, 0));
  pixels.show();
  Serial.println("RED");
  delay(500);

  // 消灯
  pixels.clear();
  pixels.show();
  delay(200);

  // 緑
  pixels.setPixelColor(0, pixels.Color(0, 50, 0));
  pixels.show();
  Serial.println("GREEN");
  delay(500);

  // 消灯
  pixels.clear();
  pixels.show();
  delay(200);

  // 青
  pixels.setPixelColor(0, pixels.Color(0, 0, 50));
  pixels.show();
  Serial.println("BLUE");
  delay(500);

  // 消灯
  pixels.clear();
  pixels.show();
  delay(500);
}
