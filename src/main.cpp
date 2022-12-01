#include <Arduino.h>
#include <WiFi.h>

#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

#include "FastAccelStepper.h"
FastAccelStepperEngine engine = FastAccelStepperEngine();
FastAccelStepper *stepper = NULL;

const int pinSpinMotorEnable = 27;
const int pinVerticalMotorEnable = 26;
const int pinHorizontalMotorEnable = 33;

const int pinTopLimitSwitch = 25;

const int pinDirection = 13;
const int pinStep = 32;
const unsigned long deBounceMs = 250;
unsigned long pressTimeTopLimitSwitch;
unsigned long pressLastTimeTopLimitSwitch;
void IRAM_ATTR TopLimit()
{
  pressTimeTopLimitSwitch = millis();
  if (pressTimeTopLimitSwitch - pressLastTimeTopLimitSwitch > deBounceMs)
  {
    pressLastTimeTopLimitSwitch = pressTimeTopLimitSwitch;
    stepper->forceStopAndNewPosition(0);
    ets_printf("Int:",millis());
  }
}

void setup()
{
  // put your setup code here, to run once:
  esp_log_level_set("*", ESP_LOG_VERBOSE);
  Serial.begin(115200);
  log_i("Booting!!!");

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

  // Setup Pins
  pinMode(pinSpinMotorEnable, OUTPUT);
  pinMode(pinVerticalMotorEnable, OUTPUT);
  pinMode(pinHorizontalMotorEnable, OUTPUT);
  pinMode(pinStep, OUTPUT);
  pinMode(pinDirection, OUTPUT);
  pinMode(pinTopLimitSwitch, INPUT_PULLUP);

  digitalWrite(pinSpinMotorEnable, LOW);
  digitalWrite(pinVerticalMotorEnable, HIGH);
  digitalWrite(pinHorizontalMotorEnable, HIGH);

  engine.init();
  stepper = engine.stepperConnectToPin(pinStep);
  attachInterrupt(pinTopLimitSwitch, TopLimit, FALLING);
}

void loop()
{
  ArduinoOTA.handle();
  if (stepper)
  {
    stepper->setDirectionPin(pinDirection);
    stepper->setEnablePin(pinSpinMotorEnable);
    stepper->setAutoEnable(true);

    stepper->setSpeedInHz(20000);   // 500 steps/s
    stepper->setAcceleration(5000); // 100 steps/s²
    stepper->move(1000000);
    while (stepper->isRunning())
    {
      delay(100);
    }
    stepper->move(-1000000);
    while (stepper->isRunning())
    {
      delay(10);
    }
  }
}