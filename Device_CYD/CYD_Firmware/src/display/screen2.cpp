#include "screen2.h"
#include "ui_common.h"
#include "../storage/preferences.h"
#include <ArduinoJson.h>

#define HEADER_H 26
#define ROW_H 30
#define COL_0_W 30
#define COL_W 48

void screen2_init() {
    ui_clear();
    screen2_draw();
}

void screen2_draw() {
    ui_clear();
    tft.setTextColor(COLOR_TEXT);
    tft.setTextSize(1);
    
    // Header text
    const char* days[] = {"Time", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
    int x_cols[] = {1, 50, 99, 161, 214, 257, 318};
    
    for (int i = 0; i < 7; i++) {
        tft.drawString(days[i], x_cols[i] + 5, 10);
    }
    
    // Grid Lines
    tft.drawRect(0, 0, 320, 240, COLOR_TEXT); // Outer border
    tft.drawLine(0, 30, 319, 30, COLOR_TEXT); // Header horizontal line
    
    // Vertical Lines
    for (int i = 1; i < 6; i++) {
        tft.drawLine(x_cols[i], 0, x_cols[i], 239, COLOR_TEXT);
    }
    
    // Horizontal Lines for Rows
    int row_h = 30;
    for (int i = 1; i < 8; i++) {
        tft.drawLine(0, 30 + i * row_h, 319, 30 + i * row_h, COLOR_TEXT);
    }
    
    // Load and draw timetable data
    int y_start = 30;
    
    String timetable_json = prefs_get_timetable();
    if(timetable_json.length() > 0) {
        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, timetable_json);
        if(!error) {
            JsonArray table = doc["table"];
            for(int r = 0; r < 7 && r < (int)table.size(); r++) {
                JsonArray row = table[r];
                for(int c = 0; c < 6 && c < (int)row.size(); c++) {
                    String subject = row[c].as<String>();
                    if(subject.length() > 0) {
                        tft.setCursor(x_cols[c+1] + 4, y_start + r*row_h + 8);
                        tft.print(subject.substring(0, 8)); // clip text
                    }
                }
            }
        }
    }
}

void screen2_update() {
    // Only update if timetable changed via flag or web ui
}