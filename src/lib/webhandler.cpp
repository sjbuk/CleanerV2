
#include <./lib/webhandler.h>

// WebHandler::WebHandler(Machine *machine)
// {
//    // AsyncWebServer _server(80);
// };
void WebHandler::handleCommand(AsyncWebServerRequest *request, JsonVariant &json)
{
  ESP_LOGI("Web", "Message received from WEB");
  JsonObject doc = json.as<JsonObject>();
  JsonArray arr = doc["actions"].as<JsonArray>();
  msgCommand command;
  for (JsonVariant value : arr)
  {
    command.action = (ACTIONS)value["action"].as<int>();
    command.value = value["value"].as<int>();
    xQueueSend(qCommands,&command,10);
  }

  request->send(200, "text/plain", "{\"result\" :\"OK\"}");
};

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

  // Add handlers
  AsyncCallbackJsonWebHandler *commandHandler = new AsyncCallbackJsonWebHandler("/api/command", handleCommand);
  _webserver->addHandler(commandHandler);

  // Route for static content
  _webserver->serveStatic("/", SPIFFS, "/www/").setDefaultFile("index.html");

  // Route to return settings values
  _webserver->on("/api/state", HTTP_GET, [this](AsyncWebServerRequest *request)
                 {
                   ESP_LOGI("Web","api/state called");
                   AsyncResponseStream *response = request->beginResponseStream("application/json");
                   serializeJson( _stateJson(_machine->getState()), *response);
                   request->send(response); });

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

void WebHandler::LogPage(String Severity, const char *format, ...)
{
  if (Severity == "ERROR" && _logfilter.Errors || Severity == "WARN" && _logfilter.Warnings || Severity == "INFO" && _logfilter.Information || Severity == "VERBOSE" && _logfilter.Verbose || Severity == "DEBUG" && _logfilter.Debug)
  {
    char MessageFinal[256];

    char buffer[200];
    va_list args;
    va_start(args, format);
    vsprintf(buffer, format, args);
    perror(buffer);
    va_end(args);

    unsigned long epochTime;
    epochTime = _ntpTimeClient->getEpochTime();
    snprintf(MessageFinal, 256, "{\"timestamp\":%lu,\"severity\":\"%s\",\"message\":\"%s\"}", epochTime, Severity.c_str(), buffer);
    _websocketserver->textAll(MessageFinal);
  }
}
void WebHandler::_onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
{
  switch (type)
  {
  case WS_EVT_CONNECT:
    Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
    _wsClient = client;
    break;
  case WS_EVT_DISCONNECT:
    Serial.printf("WebSocket client #%u disconnected\n", client->id());
    _wsClient = nullptr;
    break;
  case WS_EVT_DATA:
    _processWsMessage(data);
    break;
  case WS_EVT_PONG:
    break;
  case WS_EVT_ERROR:
    break;
  }
}

StaticJsonDocument<250> WebHandler::_stateJson(MACHINESTATE state)
{
  StaticJsonDocument<250> json;
  json["motor"]["selected"] = (int)state.selectedMotor;
  json["motor"]["selectedName"] = _machine->GetMotorName(state.selectedMotor);
  json["motor"]["Running"] = (int)state.motorRunning ? "true" : "false";

  json["vertical"]["Position"] = (int)state.verticalPosition;
  json["vertical"]["Name"] = _machine->GetVerticalName(state.verticalPosition);
  json["vertical"]["TargetPosition"] = (int)state.verticalTargetPosition;
  json["vertical"]["TargetName"] = _machine->GetVerticalName(state.verticalTargetPosition);
  json["vertical"]["CurrentStep"] = (int)state.verticalCurrentStep;

  json["horizontal"]["CurrentStep"] = (int)state.horizontalCurrentStep;
  json["horizontal"]["Position"] = (int)state.horizontalPosition;
  json["horizontal"]["Name"] = _machine->GetHorizontalName(state.horizontalPosition);
  json["horizontal"]["TargetPosition"] = (int)state.horizontalTargetPosition;
  json["horizontal"]["Name"] = _machine->GetHorizontalName(state.horizontalTargetPosition);
  json["horizontal"]["CurrentStep"] = (int)state.horizontalCurrentStep;

  return json;
}

void WebHandler::_processWsMessage(uint8_t *data)
{
  StaticJsonDocument<200> doc;
  DeserializationError error = deserializeJson(doc, data);
  if (error)
  {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    return;
  }

  String msgType = doc["messageType"];
  // Message type = logfilter
  if (msgType == "logfilter")
  {
    _logfilter.Debug = (bool)doc["message"]["Debug"];
    _logfilter.Errors = (bool)doc["message"]["Errors"];
    _logfilter.Warnings = (bool)doc["message"]["Warnings"];
    _logfilter.Information = (bool)doc["message"]["Information"];
    _logfilter.Verbose = (bool)doc["message"]["Verbose"];
  }
}