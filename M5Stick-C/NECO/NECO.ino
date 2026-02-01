#include <M5StickC.h>
#include <Adafruit_NeoPixel.h>

#define LED_PIN    32
#define BUTTON_PIN 33

#define NUM_LEDS_PER_UNIT 35
#define NUM_UNITS 2
#define NUM_LEDS (NUM_LEDS_PER_UNIT * NUM_UNITS)

Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

enum AnimationMode {
  SOLID,              // 0: 単色固定
  RAINBOW,            // 1: レインボー固定
  RAINBOW_CYCLE,      // 2: レインボー回転
  CHASE,              // 3: 単色チェイス
  RAINBOW_CHASE,      // 4: レインボーチェイス
  TWINKLE,            // 5: 単色キラキラ
  RAINBOW_TWINKLE,    // 6: レインボーキラキラ
  BREATH,             // 7: 単色呼吸
  RAINBOW_BREATH,     // 8: レインボー呼吸
  WAVE,               // 9: 単色ウェーブ
  RAINBOW_WAVE,       // 10: レインボーウェーブ
  THEATER_CHASE,      // 11: シアターチェイス
  RAINBOW_THEATER,    // 12: レインボーシアター
  COMET,              // 13: 単色コメット
  RAINBOW_COMET,      // 14: レインボーコメット
  FIRE,               // 15: 炎エフェクト
  OCEAN,              // 16: 海エフェクト
  FOREST,             // 17: 森エフェクト
  STROBE,             // 18: ストロボ
  SCANNER,            // 19: スキャナー（ナイトライダー風）
  RAINBOW_SCANNER,    // 20: レインボースキャナー
  FADE_INOUT,         // 21: フェードイン・アウト
  COLOR_WIPE,         // 22: カラーワイプ
  RAINBOW_PULSE,      // 23: レインボーパルス
  CONFETTI,           // 24: 紙吹雪
  MODE_COUNT          // モード数カウント用
};

enum TargetMode {
  BOTH,
  UNIT1_ONLY,
  UNIT2_ONLY
};

struct UnitState {
  AnimationMode mode;
  int brightness;
  uint32_t baseColor;
  uint16_t animationStep;
};

UnitState unit1 = {SOLID, 100, 0xFF0000, 0};
UnitState unit2 = {SOLID, 100, 0x0000FF, 0};

TargetMode currentTarget = BOTH;
bool lastButtonState = HIGH;

void setup() {
  M5.begin();
  M5.Lcd.setRotation(3);
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setTextSize(1);
  
  strip.begin();
  strip.show();
  
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  
  Serial.begin(115200);
  Serial.println("Cat Unit - Enhanced Animations");
  
  showStartupAnimation();
}

void showStartupAnimation() {
  // レインボーで起動アニメーション
  for(int j = 0; j < 256; j += 5) {
    for(int i = 0; i < NUM_LEDS; i++) {
      strip.setPixelColor(i, Wheel((i + j) & 255));
    }
    strip.show();
    delay(10);
  }
  
  strip.clear();
  strip.show();
}

void loop() {
  M5.update();
  
  checkUnitButton();
  
  if (M5.BtnA.wasPressed()) {
    currentTarget = (TargetMode)((currentTarget + 1) % 3);
    Serial.printf("Target Mode: %d\n", currentTarget);
    showTargetFeedback();
  }
  
  if (M5.BtnB.wasPressed()) {
    changeBrightness();
  }
  
  animateUnit(1, &unit1);
  animateUnit(2, &unit2);
  
  strip.show();
  updateDisplay();
  
  delay(20);
}

void checkUnitButton() {
  bool currentButtonState = digitalRead(BUTTON_PIN);
  
  if (lastButtonState == HIGH && currentButtonState == LOW) {
    changeAnimationMode();
  }
  
  lastButtonState = currentButtonState;
}

void changeAnimationMode() {
  switch(currentTarget) {
    case BOTH:
      {
        AnimationMode nextMode = (AnimationMode)((unit1.mode + 1) % MODE_COUNT);
        unit1.mode = nextMode;
        unit2.mode = nextMode;
        unit1.animationStep = 0;
        unit2.animationStep = 0;
        Serial.printf("BOTH - Mode: %d\n", nextMode);
      }
      break;
      
    case UNIT1_ONLY:
      unit1.mode = (AnimationMode)((unit1.mode + 1) % MODE_COUNT);
      unit1.animationStep = 0;
      Serial.printf("Unit1 - Mode: %d\n", unit1.mode);
      break;
      
    case UNIT2_ONLY:
      unit2.mode = (AnimationMode)((unit2.mode + 1) % MODE_COUNT);
      unit2.animationStep = 0;
      Serial.printf("Unit2 - Mode: %d\n", unit2.mode);
      break;
  }
}

void changeBrightness() {
  switch(currentTarget) {
    case BOTH:
      unit1.brightness += 50;
      if (unit1.brightness > 255) unit1.brightness = 50;
      unit2.brightness = unit1.brightness;
      Serial.printf("BOTH - Brightness: %d\n", unit1.brightness);
      break;
      
    case UNIT1_ONLY:
      unit1.brightness += 50;
      if (unit1.brightness > 255) unit1.brightness = 50;
      Serial.printf("Unit1 - Brightness: %d\n", unit1.brightness);
      break;
      
    case UNIT2_ONLY:
      unit2.brightness += 50;
      if (unit2.brightness > 255) unit2.brightness = 50;
      Serial.printf("Unit2 - Brightness: %d\n", unit2.brightness);
      break;
  }
}

void showTargetFeedback() {
  strip.clear();
  
  switch(currentTarget) {
    case BOTH:
      for(int i = 0; i < NUM_LEDS; i++) {
        strip.setPixelColor(i, strip.Color(255, 255, 255));
      }
      break;
      
    case UNIT1_ONLY:
      for(int i = 0; i < NUM_LEDS_PER_UNIT; i++) {
        strip.setPixelColor(i, strip.Color(255, 255, 255));
      }
      break;
      
    case UNIT2_ONLY:
      for(int i = NUM_LEDS_PER_UNIT; i < NUM_LEDS; i++) {
        strip.setPixelColor(i, strip.Color(255, 255, 255));
      }
      break;
  }
  
  strip.show();
  delay(200);
  strip.clear();
  strip.show();
}

void animateUnit(int unitNum, UnitState* unit) {
  int startLED = (unitNum - 1) * NUM_LEDS_PER_UNIT;
  int endLED = startLED + NUM_LEDS_PER_UNIT;
  
  switch(unit->mode) {
    case SOLID:
      animateSolid(startLED, endLED, unit);
      break;
    case RAINBOW:
      animateRainbow(startLED, endLED, unit);
      break;
    case RAINBOW_CYCLE:
      animateRainbowCycle(startLED, endLED, unit);
      break;
    case CHASE:
      animateChase(startLED, endLED, unit);
      break;
    case RAINBOW_CHASE:
      animateRainbowChase(startLED, endLED, unit);
      break;
    case TWINKLE:
      animateTwinkle(startLED, endLED, unit);
      break;
    case RAINBOW_TWINKLE:
      animateRainbowTwinkle(startLED, endLED, unit);
      break;
    case BREATH:
      animateBreath(startLED, endLED, unit);
      break;
    case RAINBOW_BREATH:
      animateRainbowBreath(startLED, endLED, unit);
      break;
    case WAVE:
      animateWave(startLED, endLED, unit);
      break;
    case RAINBOW_WAVE:
      animateRainbowWave(startLED, endLED, unit);
      break;
    case THEATER_CHASE:
      animateTheaterChase(startLED, endLED, unit);
      break;
    case RAINBOW_THEATER:
      animateRainbowTheater(startLED, endLED, unit);
      break;
    case COMET:
      animateComet(startLED, endLED, unit);
      break;
    case RAINBOW_COMET:
      animateRainbowComet(startLED, endLED, unit);
      break;
    case FIRE:
      animateFire(startLED, endLED, unit);
      break;
    case OCEAN:
      animateOcean(startLED, endLED, unit);
      break;
    case FOREST:
      animateForest(startLED, endLED, unit);
      break;
    case STROBE:
      animateStrobe(startLED, endLED, unit);
      break;
    case SCANNER:
      animateScanner(startLED, endLED, unit);
      break;
    case RAINBOW_SCANNER:
      animateRainbowScanner(startLED, endLED, unit);
      break;
    case FADE_INOUT:
      animateFadeInOut(startLED, endLED, unit);
      break;
    case COLOR_WIPE:
      animateColorWipe(startLED, endLED, unit);
      break;
    case RAINBOW_PULSE:
      animateRainbowPulse(startLED, endLED, unit);
      break;
    case CONFETTI:
      animateConfetti(startLED, endLED, unit);
      break;
  }
  
  unit->animationStep++;
}

// ========== アニメーション関数 ==========

void animateSolid(int start, int end, UnitState* unit) {
  uint32_t color = applyBrightness(unit->baseColor, unit->brightness);
  for(int i = start; i < end; i++) {
    strip.setPixelColor(i, color);
  }
}

void animateRainbow(int start, int end, UnitState* unit) {
  for(int i = start; i < end; i++) {
    int hue = ((i - start) * 256 / (end - start)) % 256;
    uint32_t color = Wheel(hue);
    color = applyBrightness(color, unit->brightness);
    strip.setPixelColor(i, color);
  }
}

void animateRainbowCycle(int start, int end, UnitState* unit) {
  for(int i = start; i < end; i++) {
    int hue = ((i - start) * 256 / (end - start) + unit->animationStep) % 256;
    uint32_t color = Wheel(hue);
    color = applyBrightness(color, unit->brightness);
    strip.setPixelColor(i, color);
  }
}

void animateChase(int start, int end, UnitState* unit) {
  int pos = (unit->animationStep / 2) % (end - start);
  for(int i = start; i < end; i++) {
    int distance = abs((i - start) - pos);
    if (distance < 3) {
      int brightness = unit->brightness * (3 - distance) / 3;
      strip.setPixelColor(i, applyBrightness(unit->baseColor, brightness));
    } else {
      strip.setPixelColor(i, 0);
    }
  }
}

void animateRainbowChase(int start, int end, UnitState* unit) {
  int pos = (unit->animationStep / 2) % (end - start);
  for(int i = start; i < end; i++) {
    int distance = abs((i - start) - pos);
    if (distance < 5) {
      int hue = (unit->animationStep * 3) % 256;
      int brightness = unit->brightness * (5 - distance) / 5;
      strip.setPixelColor(i, applyBrightness(Wheel(hue), brightness));
    } else {
      strip.setPixelColor(i, 0);
    }
  }
}

void animateTwinkle(int start, int end, UnitState* unit) {
  for(int i = start; i < end; i++) {
    uint32_t c = strip.getPixelColor(i);
    uint8_t r = ((c >> 16) & 0xFF) * 0.9;
    uint8_t g = ((c >> 8) & 0xFF) * 0.9;
    uint8_t b = (c & 0xFF) * 0.9;
    strip.setPixelColor(i, strip.Color(r, g, b));
  }
  
  if (unit->animationStep % 3 == 0) {
    int pos = start + random(end - start);
    strip.setPixelColor(pos, applyBrightness(unit->baseColor, unit->brightness));
  }
}

void animateRainbowTwinkle(int start, int end, UnitState* unit) {
  for(int i = start; i < end; i++) {
    uint32_t c = strip.getPixelColor(i);
    uint8_t r = ((c >> 16) & 0xFF) * 0.9;
    uint8_t g = ((c >> 8) & 0xFF) * 0.9;
    uint8_t b = (c & 0xFF) * 0.9;
    strip.setPixelColor(i, strip.Color(r, g, b));
  }
  
  if (unit->animationStep % 3 == 0) {
    int pos = start + random(end - start);
    int hue = random(256);
    strip.setPixelColor(pos, applyBrightness(Wheel(hue), unit->brightness));
  }
}

void animateBreath(int start, int end, UnitState* unit) {
  float breath = (sin(unit->animationStep * 0.05) + 1.0) / 2.0;
  int brightness = unit->brightness * breath;
  uint32_t color = applyBrightness(unit->baseColor, brightness);
  
  for(int i = start; i < end; i++) {
    strip.setPixelColor(i, color);
  }
}

void animateRainbowBreath(int start, int end, UnitState* unit) {
  float breath = (sin(unit->animationStep * 0.05) + 1.0) / 2.0;
  int brightness = unit->brightness * breath;
  
  for(int i = start; i < end; i++) {
    int hue = ((i - start) * 256 / (end - start) + unit->animationStep / 2) % 256;
    strip.setPixelColor(i, applyBrightness(Wheel(hue), brightness));
  }
}

void animateWave(int start, int end, UnitState* unit) {
  for(int i = start; i < end; i++) {
    float wave = (sin((i - start) * 0.3 + unit->animationStep * 0.1) + 1.0) / 2.0;
    int brightness = unit->brightness * wave;
    strip.setPixelColor(i, applyBrightness(unit->baseColor, brightness));
  }
}

void animateRainbowWave(int start, int end, UnitState* unit) {
  for(int i = start; i < end; i++) {
    float wave = (sin((i - start) * 0.3 + unit->animationStep * 0.1) + 1.0) / 2.0;
    int brightness = unit->brightness * wave;
    int hue = ((i - start) * 256 / (end - start) + unit->animationStep) % 256;
    strip.setPixelColor(i, applyBrightness(Wheel(hue), brightness));
  }
}

void animateTheaterChase(int start, int end, UnitState* unit) {
  for(int i = start; i < end; i++) {
    if ((i + unit->animationStep / 3) % 3 == 0) {
      strip.setPixelColor(i, applyBrightness(unit->baseColor, unit->brightness));
    } else {
      strip.setPixelColor(i, 0);
    }
  }
}

void animateRainbowTheater(int start, int end, UnitState* unit) {
  for(int i = start; i < end; i++) {
    if ((i + unit->animationStep / 3) % 3 == 0) {
      int hue = (i * 256 / (end - start) + unit->animationStep) % 256;
      strip.setPixelColor(i, applyBrightness(Wheel(hue), unit->brightness));
    } else {
      strip.setPixelColor(i, 0);
    }
  }
}

void animateComet(int start, int end, UnitState* unit) {
  int pos = (unit->animationStep / 2) % (end - start);
  
  for(int i = start; i < end; i++) {
    strip.setPixelColor(i, 0);
  }
  
  for(int i = 0; i < 10; i++) {
    int ledPos = pos - i;
    if (ledPos >= 0 && ledPos < (end - start)) {
      int brightness = unit->brightness * (10 - i) / 10;
      strip.setPixelColor(start + ledPos, applyBrightness(unit->baseColor, brightness));
    }
  }
}

void animateRainbowComet(int start, int end, UnitState* unit) {
  int pos = (unit->animationStep / 2) % (end - start);
  
  for(int i = start; i < end; i++) {
    strip.setPixelColor(i, 0);
  }
  
  for(int i = 0; i < 10; i++) {
    int ledPos = pos - i;
    if (ledPos >= 0 && ledPos < (end - start)) {
      int brightness = unit->brightness * (10 - i) / 10;
      int hue = (unit->animationStep * 2 + i * 10) % 256;
      strip.setPixelColor(start + ledPos, applyBrightness(Wheel(hue), brightness));
    }
  }
}

void animateFire(int start, int end, UnitState* unit) {
  for(int i = start; i < end; i++) {
    int flicker = random(50, 150);
    int r = min(255, flicker + 100);
    int g = flicker / 3;
    int b = 0;
    strip.setPixelColor(i, applyBrightness(strip.Color(r, g, b), unit->brightness));
  }
}

void animateOcean(int start, int end, UnitState* unit) {
  for(int i = start; i < end; i++) {
    float wave = (sin((i - start) * 0.2 + unit->animationStep * 0.05) + 1.0) / 2.0;
    int b = 100 + wave * 155;
    int g = 50 + wave * 100;
    strip.setPixelColor(i, applyBrightness(strip.Color(0, g, b), unit->brightness));
  }
}

void animateForest(int start, int end, UnitState* unit) {
  for(int i = start; i < end; i++) {
    float wave = (sin((i - start) * 0.15 + unit->animationStep * 0.03) + 1.0) / 2.0;
    int g = 100 + wave * 155;
    int r = wave * 50;
    strip.setPixelColor(i, applyBrightness(strip.Color(r, g, 0), unit->brightness));
  }
}

void animateStrobe(int start, int end, UnitState* unit) {
  if (unit->animationStep % 10 < 2) {
    for(int i = start; i < end; i++) {
      strip.setPixelColor(i, applyBrightness(unit->baseColor, unit->brightness));
    }
  } else {
    for(int i = start; i < end; i++) {
      strip.setPixelColor(i, 0);
    }
  }
}

void animateScanner(int start, int end, UnitState* unit) {
  int pos = unit->animationStep % ((end - start) * 2);
  if (pos >= (end - start)) {
    pos = (end - start) * 2 - pos - 1;
  }
  
  for(int i = start; i < end; i++) {
    int distance = abs((i - start) - pos);
    if (distance < 5) {
      int brightness = unit->brightness * (5 - distance) / 5;
      strip.setPixelColor(i, applyBrightness(unit->baseColor, brightness));
    } else {
      strip.setPixelColor(i, 0);
    }
  }
}

void animateRainbowScanner(int start, int end, UnitState* unit) {
  int pos = unit->animationStep % ((end - start) * 2);
  if (pos >= (end - start)) {
    pos = (end - start) * 2 - pos - 1;
  }
  
  for(int i = start; i < end; i++) {
    int distance = abs((i - start) - pos);
    if (distance < 5) {
      int brightness = unit->brightness * (5 - distance) / 5;
      int hue = (unit->animationStep * 3) % 256;
      strip.setPixelColor(i, applyBrightness(Wheel(hue), brightness));
    } else {
      strip.setPixelColor(i, 0);
    }
  }
}

void animateFadeInOut(int start, int end, UnitState* unit) {
  int cycle = unit->animationStep % 100;
  int brightness;
  if (cycle < 50) {
    brightness = unit->brightness * cycle / 50;
  } else {
    brightness = unit->brightness * (100 - cycle) / 50;
  }
  
  for(int i = start; i < end; i++) {
    strip.setPixelColor(i, applyBrightness(unit->baseColor, brightness));
  }
}

void animateColorWipe(int start, int end, UnitState* unit) {
  int pos = (unit->animationStep / 2) % (end - start);
  
  for(int i = start; i < end; i++) {
    if ((i - start) <= pos) {
      strip.setPixelColor(i, applyBrightness(unit->baseColor, unit->brightness));
    } else {
      strip.setPixelColor(i, 0);
    }
  }
}

void animateRainbowPulse(int start, int end, UnitState* unit) {
  float pulse = (sin(unit->animationStep * 0.1) + 1.0) / 2.0;
  int brightness = unit->brightness * pulse;
  int hue = (unit->animationStep * 2) % 256;
  
  for(int i = start; i < end; i++) {
    strip.setPixelColor(i, applyBrightness(Wheel(hue), brightness));
  }
}

void animateConfetti(int start, int end, UnitState* unit) {
  for(int i = start; i < end; i++) {
    uint32_t c = strip.getPixelColor(i);
    uint8_t r = ((c >> 16) & 0xFF) * 0.95;
    uint8_t g = ((c >> 8) & 0xFF) * 0.95;
    uint8_t b = (c & 0xFF) * 0.95;
    strip.setPixelColor(i, strip.Color(r, g, b));
  }
  
  if (random(10) < 3) {
    int pos = start + random(end - start);
    int hue = random(256);
    strip.setPixelColor(pos, applyBrightness(Wheel(hue), unit->brightness));
  }
}

// ========== ユーティリティ関数 ==========

uint32_t applyBrightness(uint32_t color, int brightness) {
  uint8_t r = ((color >> 16) & 0xFF) * brightness / 255;
  uint8_t g = ((color >> 8) & 0xFF) * brightness / 255;
  uint8_t b = (color & 0xFF) * brightness / 255;
  return strip.Color(r, g, b);
}

uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

void updateDisplay() {
  M5.Lcd.fillScreen(BLACK);
  
  const char* modeNames[] = {
    "SOLID", "RAINBOW", "R.CYCLE", "CHASE", "R.CHASE",
    "TWINKLE", "R.TWINKL", "BREATH", "R.BREATH", "WAVE",
    "R.WAVE", "THEATER", "R.THEATR", "COMET", "R.COMET",
    "FIRE", "OCEAN", "FOREST", "STROBE", "SCANNER",
    "R.SCANNR", "FADE", "WIPE", "R.PULSE", "CONFETTI"
  };
  const char* targetNames[] = {"BOTH", "UNIT1", "UNIT2"};
  
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.setTextColor(YELLOW);
  M5.Lcd.setTextSize(1);
  M5.Lcd.printf("Target:%s [%d/%d]\n", targetNames[currentTarget], 
                unit1.mode + 1, MODE_COUNT);
  M5.Lcd.drawLine(0, 10, 160, 10, YELLOW);
  
  if (currentTarget == BOTH) {
    M5.Lcd.setCursor(0, 15);
    M5.Lcd.setTextColor(CYAN);
    M5.Lcd.println("Synchronized:");
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.printf(" %s\n", modeNames[unit1.mode]);
    M5.Lcd.printf(" Bri:%d\n", unit1.brightness);
    
    M5.Lcd.setCursor(0, 50);
    M5.Lcd.setTextColor(RED);
    M5.Lcd.print("U1");
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.print(" / ");
    M5.Lcd.setTextColor(BLUE);
    M5.Lcd.println("U2");
  } else {
    M5.Lcd.setCursor(0, 15);
    if (currentTarget == UNIT1_ONLY) {
      M5.Lcd.setTextColor(RED);
    } else {
      M5.Lcd.setTextColor(DARKGREY);
    }
    M5.Lcd.println("Unit1:");
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.printf(" %s\n", modeNames[unit1.mode]);
    M5.Lcd.printf(" Bri:%d\n", unit1.brightness);
    
    M5.Lcd.setCursor(0, 50);
    if (currentTarget == UNIT2_ONLY) {
      M5.Lcd.setTextColor(BLUE);
    } else {
      M5.Lcd.setTextColor(DARKGREY);
    }
    M5.Lcd.println("Unit2:");
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.printf(" %s\n", modeNames[unit2.mode]);
    M5.Lcd.printf(" Bri:%d\n", unit2.brightness);
  }
}