#include <Arduino.h>
#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <string>

#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <./lib/machine.h>
#include <./lib/webhandler.h>

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

Machine *machine = NULL;
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
  Serial.begin(115200);
  log_i("Booting!!!");

  SetupOTA();
  InitTime();

  // Initialize SPIFFS
  if (!SPIFFS.begin(true))
  {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  Serial.println("WEBHANDLER from Main.cpp");
  machine = new Machine();
  web = new WebHandler(machine,&timeClient);
}

void loop()
{
  ArduinoOTA.handle();
  delay(1000);
  web->LogPage("LOW","Log Message");
}