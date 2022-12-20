#ifndef WEBHANDLER_H
#define WEBHANDLER_H

#include <ArduinoJson.h>
#include <./lib/machine.h>
#include <./lib/structs.cpp>
#include "ESPAsyncWebServer.h"
#include <FS.h> //this needs to be first, or it all crashes and burns...
#include "SPIFFS.h"
#include <string>
#include <NTPClient.h>

static AsyncWebSocketClient *_wsClient;
static AsyncWebSocket *_websocketserver;
static LOGFILTER _logfilter;

class WebHandler
{
private:
    StaticJsonDocument<250> _stateJson(MACHINESTATE state);
    Machine *_machine;
    AsyncWebServer *_webserver;
    NTPClient *_ntpTimeClient;
    void _WebServerRoutes();
    static void _onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);
    static void _processWsMessage(uint8_t *data);

public:
    //    WebHandler(Machine *machine);
    WebHandler(Machine *machine, NTPClient *ntpTimeClient);
    void LogPage(String Severity, const char *format, ...);
};

#endif
