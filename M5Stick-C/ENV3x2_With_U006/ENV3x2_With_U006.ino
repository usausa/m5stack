#include <M5Unified.h>
#include "M5UnitENV.h"

// TCA9548A I2C Multiplexer
#define TCA9548A_ADDR 0x70
#define SDA_PIN 32
#define SCL_PIN 33

SHT3X sht3x_port0, sht3x_port1;
QMP6988 qmp_port0, qmp_port1;

// I2Cハブのポート選択 (0-2)
void selectI2CPort(uint8_t port) {
    if (port > 2) return;
    
    Wire.beginTransmission(TCA9548A_ADDR);
    Wire.write(1 << port);  // ポートをビットで指定
    Wire.endTransmission();
    delay(10);
}

// 全ポート無効化
void disableAllPorts() {
    Wire.beginTransmission(TCA9548A_ADDR);
    Wire.write(0);
    Wire.endTransmission();
}

void setup() {
    M5.begin();
    Serial.begin(115200);
    delay(2000);
    
    M5.Display.setRotation(1);
    M5.Display.fillScreen(BLACK);
    M5.Display.setTextSize(1);
    M5.Display.setCursor(0, 0);
    
    Serial.println("\n=== I2C Hub (U006) + ENV3 x2 ===");
    M5.Display.println("I2C Hub U006");
    M5.Display.println("ENV3 x2");
    
    // I2C初期化
    Wire.begin(SDA_PIN, SCL_PIN);
    delay(100);
    
    // I2Cハブの確認
    Wire.beginTransmission(TCA9548A_ADDR);
    if (Wire.endTransmission() == 0) {
        Serial.println("I2C Hub found!");
        M5.Display.println("\nHub: OK");
    } else {
        Serial.println("I2C Hub NOT found!");
        M5.Display.println("\nHub: NG");
        while(1) delay(100);
    }
    
    delay(500);
    
    // ポート0のENV3初期化
    Serial.println("\n--- Port 0 Init ---");
    M5.Display.println("\nPort0 Init...");
    selectI2CPort(0);
    delay(50);
    
    // I2Cスキャン
    Serial.println("Port0 I2C Scan:");
    for (byte addr = 1; addr < 127; addr++) {
        Wire.beginTransmission(addr);
        if (Wire.endTransmission() == 0) {
            Serial.printf("  Found: 0x%02X\n", addr);
        }
    }
    
    if (qmp_port0.begin(&Wire, QMP6988_SLAVE_ADDRESS_L, SDA_PIN, SCL_PIN, 400000U)) {
        Serial.println("Port0: QMP OK");
        M5.Display.println("Port0: QMP OK");
    } else {
        Serial.println("Port0: QMP FAILED");
        M5.Display.println("Port0: QMP NG");
    }
    
    if (sht3x_port0.begin(&Wire, SHT3X_I2C_ADDR, SDA_PIN, SCL_PIN, 400000U)) {
        Serial.println("Port0: SHT OK");
        M5.Display.println("Port0: SHT OK");
    } else {
        Serial.println("Port0: SHT FAILED");
        M5.Display.println("Port0: SHT NG");
    }
    
    delay(500);
    
    // ポート1のENV3初期化
    Serial.println("\n--- Port 1 Init ---");
    M5.Display.println("\nPort1 Init...");
    selectI2CPort(1);
    delay(50);
    
    // I2Cスキャン
    Serial.println("Port1 I2C Scan:");
    for (byte addr = 1; addr < 127; addr++) {
        Wire.beginTransmission(addr);
        if (Wire.endTransmission() == 0) {
            Serial.printf("  Found: 0x%02X\n", addr);
        }
    }
    
    if (qmp_port1.begin(&Wire, QMP6988_SLAVE_ADDRESS_L, SDA_PIN, SCL_PIN, 400000U)) {
        Serial.println("Port1: QMP OK");
        M5.Display.println("Port1: QMP OK");
    } else {
        Serial.println("Port1: QMP FAILED");
        M5.Display.println("Port1: QMP NG");
    }
    
    if (sht3x_port1.begin(&Wire, SHT3X_I2C_ADDR, SDA_PIN, SCL_PIN, 400000U)) {
        Serial.println("Port1: SHT OK");
        M5.Display.println("Port1: SHT OK");
    } else {
        Serial.println("Port1: SHT FAILED");
        M5.Display.println("Port1: SHT NG");
    }
    
    delay(2000);
    Serial.println("\n=== Starting Loop ===\n");
}

void loop() {
    M5.Display.fillScreen(BLACK);
    M5.Display.setCursor(0, 0);
    M5.Display.setTextColor(WHITE);
    M5.Display.setTextSize(1);
    
    // ポート0読み取り
    Serial.println("--- Port 0 ---");
    selectI2CPort(0);
    delay(20);
    
    if (sht3x_port0.update() && qmp_port0.update()) {
        float temp = sht3x_port0.cTemp;
        float humi = sht3x_port0.humidity;
        float pres = qmp_port0.pressure / 100.0;
        
        M5.Display.setTextColor(YELLOW);
        M5.Display.println("\n[Port 0]");
        M5.Display.setTextColor(RED);
        M5.Display.printf("T: %.1fC\n", temp);
        M5.Display.setTextColor(CYAN);
        M5.Display.printf("H: %.0f%%\n", humi);
        M5.Display.setTextColor(GREEN);
        M5.Display.printf("P: %.0fhPa\n", pres);
        
        Serial.printf("Port0: T=%.1fC H=%.0f%% P=%.0fhPa\n", 
                      temp, humi, pres);
    } else {
        M5.Display.setTextColor(RED);
        M5.Display.println("\n[Port 0] ERROR");
        Serial.println("Port0: Read error");
    }
    
    delay(100);
    
    // ポート1読み取り
    Serial.println("--- Port 1 ---");
    selectI2CPort(1);
    delay(20);
    
    if (sht3x_port1.update() && qmp_port1.update()) {
        float temp = sht3x_port1.cTemp;
        float humi = sht3x_port1.humidity;
        float pres = qmp_port1.pressure / 100.0;
        
        M5.Display.setTextColor(YELLOW);
        M5.Display.println("\n[Port 1]");
        M5.Display.setTextColor(RED);
        M5.Display.printf("T: %.1fC\n", temp);
        M5.Display.setTextColor(CYAN);
        M5.Display.printf("H: %.0f%%\n", humi);
        M5.Display.setTextColor(GREEN);
        M5.Display.printf("P: %.0fhPa\n", pres);
        
        Serial.printf("Port1: T=%.1fC H=%.0f%% P=%.0fhPa\n", 
                      temp, humi, pres);
    } else {
        M5.Display.setTextColor(RED);
        M5.Display.println("\n[Port 1] ERROR");
        Serial.println("Port1: Read error");
    }
    
    Serial.println("");
    delay(2000);
}