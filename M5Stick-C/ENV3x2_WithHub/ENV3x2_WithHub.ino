#include <M5Unified.h>
#include "M5UnitENV.h"

#define PAHUB_ADDR 0x70
#define SDA_PIN 32
#define SCL_PIN 33

SHT3X sht3x_ch0, sht3x_ch1;
QMP6988 qmp_ch0, qmp_ch1;

void selectPaHubChannel(uint8_t channel) {
    Wire.beginTransmission(PAHUB_ADDR);
    Wire.write(1 << channel);
    Wire.endTransmission();
    delay(5);
}

void setup() {
    M5.begin();
    Serial.begin(115200);
    Wire.begin(SDA_PIN, SCL_PIN);
    
    M5.Display.setRotation(1);
    M5.Display.fillScreen(BLACK);
    M5.Display.setTextSize(1);
    
    Serial.println("Multi ENV Test");
    
    // チャンネル0のENVユニット初期化
    selectPaHubChannel(0);
    qmp_ch0.begin(&Wire, QMP6988_SLAVE_ADDRESS_L, SDA_PIN, SCL_PIN, 400000U);
    sht3x_ch0.begin(&Wire, SHT3X_I2C_ADDR, SDA_PIN, SCL_PIN, 400000U);
    Serial.println("CH0 initialized");
    
    // チャンネル1のENVユニット初期化
    selectPaHubChannel(1);
    qmp_ch1.begin(&Wire, QMP6988_SLAVE_ADDRESS_L, SDA_PIN, SCL_PIN, 400000U);
    sht3x_ch1.begin(&Wire, SHT3X_I2C_ADDR, SDA_PIN, SCL_PIN, 400000U);
    Serial.println("CH1 initialized");
    
    delay(1000);
}

void loop() {
    M5.Display.fillScreen(BLACK);
    M5.Display.setCursor(0, 0);
    M5.Display.setTextColor(WHITE);
    M5.Display.println("Multi ENV");
    
    // チャンネル0読み取り
    selectPaHubChannel(0);
    if (sht3x_ch0.update() && qmp_ch0.update()) {
        M5.Display.setTextColor(YELLOW);
        M5.Display.println("\n[CH0]");
        M5.Display.setTextColor(RED);
        M5.Display.printf("%.1fC ", sht3x_ch0.cTemp);
        M5.Display.setTextColor(CYAN);
        M5.Display.printf("%.0f%%\n", sht3x_ch0.humidity);
        M5.Display.setTextColor(GREEN);
        M5.Display.printf("%.0fhPa\n", qmp_ch0.pressure/100.0);
        
        Serial.printf("CH0: %.1fC %.0f%% %.0fhPa\n", 
                      sht3x_ch0.cTemp, sht3x_ch0.humidity, 
                      qmp_ch0.pressure/100.0);
    }
    
    // チャンネル1読み取り
    selectPaHubChannel(1);
    if (sht3x_ch1.update() && qmp_ch1.update()) {
        M5.Display.setTextColor(YELLOW);
        M5.Display.println("\n[CH1]");
        M5.Display.setTextColor(RED);
        M5.Display.printf("%.1fC ", sht3x_ch1.cTemp);
        M5.Display.setTextColor(CYAN);
        M5.Display.printf("%.0f%%\n", sht3x_ch1.humidity);
        M5.Display.setTextColor(GREEN);
        M5.Display.printf("%.0fhPa\n", qmp_ch1.pressure/100.0);
        
        Serial.printf("CH1: %.1fC %.0f%% %.0fhPa\n", 
                      sht3x_ch1.cTemp, sht3x_ch1.humidity, 
                      qmp_ch1.pressure/100.0);
    }
    
    delay(2000);
}