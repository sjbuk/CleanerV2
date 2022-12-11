#include <./lib/webhandler.h>

// WebHandler::WebHandler(Machine *machine)
// {
//    // AsyncWebServer _server(80);
// };

WebHandler::WebHandler(Machine *machine, NTPClient *ntpTimeClient)
{
  Serial.println("Web handler Constructor");
  _webserver = new AsyncWebServer(80);
  _websocketserver = new AsyncWebSocket("/log");
  _WebServerRoutes();
  _machine = machine;
  _ntpTimeClient = ntpTimeClient;
};

void WebHandler::_WebServerRoutes()
{
  Serial.println("Setting up routes.");
  _websocketserver->onEvent(_onWsEvent);
  _webserver->addHandler(_websocketserver);

  // Default Headers
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Methods", "GET,PUT,POST");
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Headers", "*");

  // Route for static content
  _webserver->serveStatic("/", SPIFFS, "/www/").setDefaultFile("index.html");

  // 404 Handler
  _webserver->onNotFound([](AsyncWebServerRequest *request)
                         {
    if (request->method() == HTTP_OPTIONS){
      request->send(200);
    }else{
      log_w("Not found 404 ");
      request->send(404,"text/plain","Content not found");
    } });

  _webserver->begin();
}

void WebHandler::LogPage(String Severity,String Message)
{
  char MessageFinal[128];
  String formattedDate;
  unsigned long epochTime;
  epochTime = _ntpTimeClient->getEpochTime();
  sprintf(MessageFinal,"{\"timestamp\":%lu,\"severity\":\"%s\",\"message\":\"%s\"}",epochTime,Severity,Message.substring(0,70));
  _websocketserver->textAll(MessageFinal );
}
void WebHandler::_onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
{
  Serial.println("WS EVENT");
  switch (type)
  {
  case WS_EVT_CONNECT:
    Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
    _wsClient = client;
    _websocketserver->textAll("Hello new client");
    break;
  case WS_EVT_DISCONNECT:
    Serial.printf("WebSocket client #%u disconnected\n", client->id());
    _wsClient = nullptr;
    break;
  case WS_EVT_DATA:
  case WS_EVT_PONG:
  case WS_EVT_ERROR:
    break;
  }
}
