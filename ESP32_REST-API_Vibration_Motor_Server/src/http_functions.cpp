#include <Arduino.h>
// We will use wifi
#include <WiFi.h>

// We will use SPIFFS and FS
#include <SPIFFS.h>
#include <FS.h>

// We use JSON as data format. Make sure to have the lib available
#include <ArduinoJson.h>

// Working with c++ strings
#include <string>
/* ------------------------------ */
// TODO: Configure your WiFi here
// #define WIFI_SSID "TP-LINK_CBDE"
// #define WIFI_PSK  "50268238"

#define WIFI_SSID "Baggie"
#define WIFI_PSK  "12345680"

// String HOSTNAME = "Right_1";
String HOSTNAME = "Left_1";

/*---------------------------------------------*/
void connectToWiFi() {
  Serial.println("Setting up WiFi");
  WiFi.mode(WIFI_STA);
  WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, INADDR_NONE);
  WiFi.setHostname(HOSTNAME.c_str()); //define hostname

  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PSK);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
    // we can even make the ESP32 to sleep
  }
  Serial.print(WiFi.getHostname());
  Serial.print(" Connected. IP=");
  Serial.println(WiFi.localIP());
}

void mountSPIFF() {
  // Try to mount SPIFFS without formatting on failure
  if (!SPIFFS.begin(false)) {
    // If SPIFFS does not work, we wait for serial connection...
    while(!Serial);
    delay(1000);

    // Ask to format SPIFFS using serial interface
    Serial.print("Mounting SPIFFS failed. Try formatting? (y/n): ");
    while(!Serial.available());
    Serial.println();

    // If the user did not accept to try formatting SPIFFS or formatting failed:
    if (Serial.read() != 'y' || !SPIFFS.begin(true)) {
      Serial.println("SPIFFS not available. Stop.");
      while(true);
    }
    Serial.println("SPIFFS has been formated.");
  }
  Serial.println("SPIFFS has been mounted.");
}