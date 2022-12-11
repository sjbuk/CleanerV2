#ifndef WEBHANDLER_H
#define WEBHANDLER_H

#include <./lib/machine.h>
#include "ESPAsyncWebServer.h"
#include <FS.h> //this needs to be first, or it all crashes and burns...
#include "SPIFFS.h"
#include <string>
#include <NTPClient.h>



static AsyncWebSocketClient* _wsClient;
static AsyncWebSocket *_websocketserver;

class WebHandler
{
private:
    Machine *_machine;
    AsyncWebServer *_webserver;
    NTPClient *_ntpTimeClient;
    void _WebServerRoutes();
    static void _onWsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len);

public:
//    WebHandler(Machine *machine);
    WebHandler(Machine *machine, NTPClient *ntpTimeClient);
    void LogPage (String Severity,String Message);
};

#endif
