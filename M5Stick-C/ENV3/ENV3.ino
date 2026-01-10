#include <M5Unified.h>
#include "M5UnitENV.h"

SHT3X sht3x;
QMP6988 qmp;

bool qmpOK = false;
bool sht3xOK = false;
int loopCount = 0;

void setup() {
    auto cfg = M5.config();
    M5.begin(cfg);
    
    // シリアル通信の初期化を明示的に
    Serial.begin(115200);
    delay(1000);  // シリアル接続待ち
    
    // LCD初期化
    M5.Display.setRotation(1);
    M5.Display.fillScreen(BLACK);
    M5.Display.setTextSize(1);
    M5.Display.setTextColor(WHITE);
    M5.Display.setCursor(0, 0);
    
    Serial.println("\n=== ENV Unit Test ===");
    M5.Display.println("ENV Unit Test");
    M5.Display.println("");
    
    // QMP6988初期化
    Serial.println("Init QMP6988...");
    M5.Display.println("Init QMP6988...");
    
    if (!qmp.begin(&Wire, QMP6988_SLAVE_ADDRESS_L, 32, 33, 400000U)) {
        Serial.println("QMP6988 FAILED!");
        M5.Display.setTextColor(RED);
        M5.Display.println("QMP6988 FAILED!");
        qmpOK = false;
    } else {
        Serial.println("QMP6988 OK!");
        M5.Display.setTextColor(GREEN);
        M5.Display.println("QMP6988 OK!");
        qmpOK = true;
    }
    
    delay(500);
    
    // SHT3X初期化
    Serial.println("Init SHT3X...");
    M5.Display.setTextColor(WHITE);
    M5.Display.println("Init SHT3X...");
    
    if (!sht3x.begin(&Wire, SHT3X_I2C_ADDR, 32, 33, 400000U)) {
        Serial.println("SHT3X FAILED!");
        M5.Display.setTextColor(RED);
        M5.Display.println("SHT3X FAILED!");
        sht3xOK = false;
    } else {
        Serial.println("SHT3X OK!");
        M5.Display.setTextColor(GREEN);
        M5.Display.println("SHT3X OK!");
        sht3xOK = true;
    }
    
    delay(2000);
    
    Serial.println("\n=== Starting Loop ===\n");
}

void loop() {
    M5.update();
    loopCount++;
    
    // LCD表示クリア
    M5.Display.fillScreen(BLACK);
    M5.Display.setCursor(0, 0);
    M5.Display.setTextSize(1);
    M5.Display.setTextColor(WHITE);
    
    // ループカウント表示
    M5.Display.printf("Loop: %d\n", loopCount);
    Serial.printf("=== Loop %d ===\n", loopCount);
    
    // SHT3X読み取り
    if (sht3xOK) {
        bool sht3xUpdated = sht3x.update();
        Serial.printf("SHT3X update: %s\n", sht3xUpdated ? "SUCCESS" : "FAILED");
        
        if (sht3xUpdated) {
            M5.Display.setTextColor(CYAN);
            M5.Display.println("\n--- SHT3X ---");
            M5.Display.setTextColor(RED);
            M5.Display.printf("Temp: %.1fC\n", sht3x.cTemp);
            M5.Display.setTextColor(CYAN);
            M5.Display.printf("Humi: %.1f%%\n", sht3x.humidity);
            
            Serial.println("-----SHT3X-----");
            Serial.print("Temperature: ");
            Serial.print(sht3x.cTemp);
            Serial.println(" degrees C");
            Serial.print("Humidity: ");
            Serial.print(sht3x.humidity);
            Serial.println("% rH");
            Serial.println("-------------");
        } else {
            M5.Display.setTextColor(YELLOW);
            M5.Display.println("\nSHT3X: No Update");
            Serial.println("SHT3X: Update failed");
        }
    } else {
        M5.Display.setTextColor(RED);
        M5.Display.println("\nSHT3X: Not Init");
        Serial.println("SHT3X: Not initialized");
    }
    
    // QMP6988読み取り
    if (qmpOK) {
        bool qmpUpdated = qmp.update();
        Serial.printf("QMP6988 update: %s\n", qmpUpdated ? "SUCCESS" : "FAILED");
        
        if (qmpUpdated) {
            M5.Display.setTextColor(GREEN);
            M5.Display.println("\n--- QMP6988 ---");
            M5.Display.setTextColor(YELLOW);
            M5.Display.printf("Temp: %.1fC\n", qmp.cTemp);
            M5.Display.setTextColor(GREEN);
            M5.Display.printf("Pres: %.0fhPa\n", qmp.pressure / 100.0);
            M5.Display.printf("Alt : %.0fm\n", qmp.altitude);
            
            Serial.println("-----QMP6988-----");
            Serial.print("Temperature: ");
            Serial.print(qmp.cTemp);
            Serial.println(" *C");
            Serial.print("Pressure: ");
            Serial.print(qmp.pressure);
            Serial.println(" Pa");
            Serial.print("Approx altitude: ");
            Serial.print(qmp.altitude);
            Serial.println(" m");
            Serial.println("-------------");
        } else {
            M5.Display.setTextColor(YELLOW);
            M5.Display.println("\nQMP: No Update");
            Serial.println("QMP6988: Update failed");
        }
    } else {
        M5.Display.setTextColor(RED);
        M5.Display.println("\nQMP: Not Init");
        Serial.println("QMP6988: Not initialized");
    }
    
    Serial.println("");
    delay(1000);
}