#include <M5Unified.h>
#include <SPIFFS.h>

/**
 * idk-pdf: Safe Version (No SD conflicts)
 */

int current_page = 0;
int max_pages = 0;

void countPages() {
    max_pages = 0;
    while (max_pages < 400) {
        String filename = "/p" + String(max_pages) + ".jpg";
        if (SPIFFS.exists(filename)) {
            max_pages++;
        } else {
            break;
        }
    }
    Serial.printf("Found %d pages\n", max_pages);
}

bool loadPage(int page) {
    if (page < 0 || page >= max_pages) return false;
    
    String filename = "/p" + String(page) + ".jpg";
    Serial.printf("Loading %s\n", filename.c_str());
    
    File file = SPIFFS.open(filename, "r");
    if (!file) {
        Serial.println("Failed to open file");
        return false;
    }

    size_t size = file.size();
    uint8_t* buf = (uint8_t*)malloc(size);
    if (!buf) {
        Serial.println("Malloc failed");
        file.close();
        return false;
    }

    file.read(buf, size);
    file.close();

    M5.Display.startWrite();
    M5.Display.drawJpg(buf, size, 0, 0);
    free(buf);
    
    M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
    M5.Display.setCursor(5, 5);
    M5.Display.printf("P%d/%d", page + 1, max_pages);
    M5.Display.endWrite();
    
    return true;
}

void setup() {
    auto cfg = M5.config();
    M5.begin(cfg);
    
    M5.Display.setRotation(0); 
    M5.Display.setBrightness(128);
    M5.Display.fillScreen(TFT_GREEN); // Turn GREEN on start
    
    Serial.begin(115200);
    delay(500);
    Serial.println("PDF Reader starting...");

    if (!SPIFFS.begin(true)) {
        Serial.println("SPIFFS Mount Failed");
        M5.Display.fillScreen(TFT_RED);
        M5.Display.setCursor(10, 10);
        M5.Display.println("SPIFFS FAIL");
        return;
    }

    countPages();
    if (max_pages > 0) {
        loadPage(0);
    } else {
        M5.Display.fillScreen(TFT_ORANGE);
        M5.Display.println("No /p0.jpg");
    }
}

void loop() {
    M5.update();
    if (M5.BtnA.wasClicked()) {
        if (max_pages > 0) {
            current_page = (current_page + 1) % max_pages;
            loadPage(current_page);
        }
    }
    if (M5.BtnB.wasClicked()) {
        if (max_pages > 0) {
            current_page = (current_page - 1 + max_pages) % max_pages;
            loadPage(current_page);
        }
    }
    delay(10);
}
