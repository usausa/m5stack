#include <M5Unified.h>

#define LIGHT_PIN 33
#define GRAPH_WIDTH 160
#define GRAPH_HEIGHT 50

int brightnessHistory[GRAPH_WIDTH];
int historyIndex = 0;

void setup() {
    M5.begin();
    Serial.begin(115200);
    
    M5.Display.setRotation(1);
    M5.Display.fillScreen(BLACK);
    
    pinMode(LIGHT_PIN, INPUT);
    analogReadResolution(12);
    analogSetAttenuation(ADC_11db);
    
    // 履歴初期化
    for (int i = 0; i < GRAPH_WIDTH; i++) {
        brightnessHistory[i] = 0;
    }
    
    Serial.println("Light Sensor Graph");
}

void loop() {
    int rawValue = analogRead(LIGHT_PIN);
    int brightness = 4095 - rawValue;  // 反転
    
    // 履歴に追加
    brightnessHistory[historyIndex] = brightness;
    historyIndex = (historyIndex + 1) % GRAPH_WIDTH;
    
    // 画面クリア
    M5.Display.fillScreen(BLACK);
    
    // 数値表示
    M5.Display.setTextSize(2);
    M5.Display.setTextColor(YELLOW);
    M5.Display.setCursor(0, 0);
    M5.Display.printf("%d\n", brightness);
    
    M5.Display.setTextSize(1);
    M5.Display.setTextColor(WHITE);
    M5.Display.printf("%.0f%%\n", (brightness / 4095.0) * 100);
    
    // グラフ描画
    for (int i = 0; i < GRAPH_WIDTH - 1; i++) {
        int idx1 = (historyIndex + i) % GRAPH_WIDTH;
        int idx2 = (historyIndex + i + 1) % GRAPH_WIDTH;
        
        int y1 = GRAPH_HEIGHT - (brightnessHistory[idx1] * GRAPH_HEIGHT / 4095);
        int y2 = GRAPH_HEIGHT - (brightnessHistory[idx2] * GRAPH_HEIGHT / 4095);
        
        // 色を明るさによって変える
        uint16_t color;
        if (brightnessHistory[idx1] < 1000) color = BLUE;
        else if (brightnessHistory[idx1] < 2000) color = GREEN;
        else if (brightnessHistory[idx1] < 3000) color = YELLOW;
        else color = RED;
        
        M5.Display.drawLine(i, 30 + y1, i + 1, 30 + y2, color);
    }
    
    // グラフ枠
    M5.Display.drawRect(0, 30, GRAPH_WIDTH, GRAPH_HEIGHT, WHITE);
    
    Serial.printf("Brightness: %d\n", brightness);
    
    delay(50);
}