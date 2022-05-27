#include "multipurpose_module.h"
#include <ThingsBoard.h>
#include <WiFi.h>

const char ssid[] = "{SSID}";
const char pass[] = "{PASSWORD}";
 
#define TOKEN               "{TOKEN}" // "ESP32TEST1CORE2"
#define THINGSBOARD_SERVER  "thingsboard.cloud"   // ThingsBoard server
 
WiFiClient espClient;   // using ESP32 for MQTT client
ThingsBoard tb(espClient);  // init instance
bool subscribed = false;

#define GROVE_A_1 33 // SCL
#define GROVE_A_2 32 // SDA
#define GROVE_C_1 13 // RXD2
#define GROVE_C_2 14 // TXD2
#define GROVE_D_1 35 // analog in
#define GROVE_D_2 34 // analog in

multipurpose_module MM;

void setup(void) {
  // thingsboard ここから
  int wifi_connection_tries = 10;
 
  WiFi.begin(ssid,pass);
  Serial.print("Connecting to " + String(WIFI_AP));
  for (wifi_connection_tries; wifi_connection_tries < 1; wifi_connection_tries--) {
    if (WiFi.isConnected() != true) {
      Serial.println("\nConnected to AP");
      break;
    }
    Serial.print(".");
    delay(500);
  }
  // end to Thingsboard
}

void loop(void) {
  // connect to Wi-Fi
  if (WiFi.status() == WL_IDLE_STATUS) {
    return;
  }
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Connecting to AP ...");
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(WIFI_AP);
    WiFi.begin(ssid,pass);
    return;
  }
  // connect to ThingsBoard server
  if (!tb.connected()) {
    subscribed = false;
    Serial.print("Connecting to: ");
    Serial.print(THINGSBOARD_SERVER);
    Serial.print(" with token ");
    Serial.println(TOKEN);
    if (!tb.connect(THINGSBOARD_SERVER, TOKEN)) {
      Serial.println("Failed to connect");
      return;
    }
  }
  /**
  tb.sendTelemetryFloat("TDS", TDS); 
  tb.sendTelemetryFloat("flow_value", flow_value);
  tb.sendTelemetryFloat("flow", flow);
  tb.sendTelemetryFloat("temp", temp);
  tb.sendTelemetryFloat("pressure", pressure);
  **/
  
  // event loop for MQTT client
  tb.loop();

  delay(1000);
}
