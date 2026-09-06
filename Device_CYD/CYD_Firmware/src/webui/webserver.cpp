#include "webserver.h"
#include "api.h"
#include <ESPAsyncWebServer.h>

static AsyncWebServer server(80);

void webserver_init() {
    api_setup_routes(server);
    server.begin();
}