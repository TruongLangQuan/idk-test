#include "api.h"
#include "pages.h"
#include "../storage/preferences.h"
#include "../hardware/sdcard.h"

String processor(const String& var) {
    if (var == "NOTE") return prefs_get_note();
    if (var == "TIMETABLE") return prefs_get_timetable();
    return String();
}

void api_setup_routes(AsyncWebServer& server) {
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send_P(200, "text/html", PAGE_INDEX);
    });
    
    server.on("/note", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send_P(200, "text/html", PAGE_NOTE, processor);
    });
    
    server.on("/note", HTTP_POST, [](AsyncWebServerRequest *request){
        if (request->hasParam("note", true)) {
            String note = request->getParam("note", true)->value();
            prefs_set_note(note);
            request->redirect("/");
        } else {
            request->send(400, "text/plain", "Missing note");
        }
    });
    
    server.on("/timetable", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send_P(200, "text/html", PAGE_TIMETABLE, processor);
    });
    
    server.on("/timetable", HTTP_POST, [](AsyncWebServerRequest *request){
        if (request->hasParam("timetable", true)) {
            String json = request->getParam("timetable", true)->value();
            prefs_set_timetable(json);
            request->redirect("/");
        } else {
            request->send(400, "text/plain", "Missing timetable json");
        }
    });
    
    server.on("/files", HTTP_GET, [](AsyncWebServerRequest *request){
        String html = "<!DOCTYPE html><html><body><h1>SD Card Files</h1><ul>";
        std::vector<String> files;
        sd_list_dir("/media", files);
        for(const auto& f : files) {
            html += "<li>" + f + "</li>";
        }
        html += "</ul><p><a href='/'>Back</a></p></body></html>";
        request->send(200, "text/html", html);
    });
}