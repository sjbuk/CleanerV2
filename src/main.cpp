#include <Arduino.h>
#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <string>
#include "esp_log.h"

#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <./lib/machine.h>
#include <./lib/webhandler.h>

static const char *TAG = "CleanerV2";
static char log_print_buffer[512];
static char filePath[] = "/LOGS.txt";
QueueHandle_t qCommands, qEvents;

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

WebHandler *web = NULL;

void SetupOTA()
{
  // Connect WIFI
  WiFi.mode(WIFI_STA);
  WiFi.setHostname("ESPWATCH");
  WiFi.begin("borg", "mainhousewifi");
  while (WiFi.waitForConnectResult() != WL_CONNECTED)
  {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // Init OTA
  ArduinoOTA
      .onStart([]()
               {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH)
        type = "sketch";
      else // U_SPIFFS
        type = "filesystem";

      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      Serial.println("Start updating " + type); })
      .onEnd([]()
             { Serial.println("\nEnd"); })
      .onProgress([](unsigned int progress, unsigned int total)
                  { Serial.printf("Progress: %u%%\r", (progress / (total / 100))); })
      .onError([](ota_error_t error)
               {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed"); });
  ArduinoOTA.begin();
}

int redirectToSpiffs(const char *fmt, va_list args)
{
  // write evaluated format string into buffer
  Serial.println("Redirected logger");
  web->LogPage("WARN", "Redirected");
  return vprintf(fmt, args);
}

void InitTime()
{
  String formattedDate;
  timeClient.begin();
  while (!timeClient.update())
  {
    timeClient.forceUpdate();
  }
  formattedDate = timeClient.getFormattedTime();
  Serial.println(formattedDate);
}
void setup()
{
  // put your setup code here, to run once:
  esp_log_level_set("*", ESP_LOG_VERBOSE);


  //Create Queues
  qCommands = xQueueCreate(30, sizeof(struct msgCommand));
    if(qCommands == NULL){
    Serial.println("Error creating the queue");
  }
  qEvents = xQueueCreate(10,sizeof(EVENTS));

  MachineStart();
  SetupOTA();
  InitTime();

  // Initialize SPIFFS
  if (!SPIFFS.begin(true))
  {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  web = new WebHandler(&timeClient);
  web->LogPage("INFO", "Startup Complete");

  //TEST
  struct msgCommand x;
  x.action = ACTIONS::StartSpinMotor;
  x.value = 100;
  xQueueSend(qCommands,&x, ( TickType_t )10);
}

void loop()
{
  ArduinoOTA.handle();
}