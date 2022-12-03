#include <Arduino.h>
#include <WiFi.h>

#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <./lib/machine.h>

Machine *machine = NULL;

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
}

void setup()
{
  // put your setup code here, to run once:
  esp_log_level_set("*", ESP_LOG_VERBOSE);
  Serial.begin(115200);
  log_i("Booting!!!");

  SetupOTA();
  ArduinoOTA.begin();
  delay(50);
  ArduinoOTA.handle();

  machine = new Machine();

}

void loop()
{
  ArduinoOTA.handle();
  machine->Action12SetSpinBothwise();
  
  delay(1000);
  machine->Action31MoveVertToTop();
  delay (1000);
  machine->Action14SetSpinSpeedTo2();
  machine->ActionSpin (15000);
  delay (1000);
  machine->Action32MoveVertToMid();
  delay (1000);
  machine->Action15SetSpinSpeedTo3();
  machine->ActionSpin (15000);
  delay (1000);
  machine->Action33MoveVertToBottom();
  delay (1000);
  machine->Action16SetSpinSpeedTo4();
  machine->ActionSpin (15000);
  machine->Action32MoveVertToMid();


}