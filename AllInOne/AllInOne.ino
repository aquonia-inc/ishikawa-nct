// 青いボタンで計測(OLEDに表示し、thingsboardへ飛ばす)

// 赤いボタンで下へ遷移（ない場合は一番上にいく）

#include "multipurpose_module.h"

#include <ThingsBoard.h>
#include <WiFi.h>

#include <SHT3x.h>
#include <Wire.h>
#include "Adafruit_Sensor.h"
#include <Adafruit_BMP280.h>

const char ssid[] = "Waterize-wifi";
const char pass[] = "w9200856";
 
#define TOKEN               "NITTESTDEVICE00" // "ESP32TEST1CORE2"
#define THINGSBOARD_SERVER  "thingsboard.cloud"   // ThingsBoard server

#include <M5UnitOLED.h>
M5UnitOLED display;
M5Canvas canvas(&display);
 
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
float tdsValue = 0;
float Voltage = 0;

// button setting
int lastState = LOW;
int currentState;

SHT3x sht30;
Adafruit_BMP280 bme;

void setup(void) {
  
  Serial.begin(115200);
  MM.init();
  Wire.begin();               // I2Cを初期化する
  while (!bme.begin(0x76)) {  // BMP280を初期化する
    Serial.println("BMP280 init fail");
  }
  sht30.Begin();

  pinMode(GROVE_C_1, INPUT);
  pinMode(GROVE_D_1, INPUT);

  display.init();
  display.setRotation(1);
  canvas.setColorDepth(1); // mono color
  canvas.setFont(&fonts::lgfxJapanMinchoP_32);
  canvas.setTextWrap(false);
  canvas.setTextSize(2);
  canvas.createSprite(display.width() + 64, 72);
  
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
  //Measure
  Voltage = MM.ADCreadV(3);

  //Calcuration
  //Convert voltage value to TDS value
  tdsValue=(133.42/Voltage*Voltage*Voltage - 255.86*Voltage*Voltage + 857.39*Voltage)*0.5;
  sht30.UpdateData();
  float tmp = sht30.GetTemperature();
  float hum = sht30.GetRelHumidity();
  float pressure = bme.readPressure();

  currentState = digitalRead(GROVE_C_1);
  currentState = digitalRead(GROVE_D_1);
    
  Serial.printf("temp: %4.1f'C\r\n", tmp);
  Serial.printf("humid:%4.1f%%\r\n", hum);
  Serial.printf("press:%4.0fhPa\r\n", pressure / 100);
  
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
  if (lastState == HIGH && currentState == LOW) {
    Serial.println("The button is pressed");
    tb.sendTelemetryFloat("TDS", tdsValue); 
    tb.sendTelemetryFloat("Temperature", tmp);
    tb.sendTelemetryFloat("Humidity", hum);
    tb.sendTelemetryFloat("AirPressure", pressure/100);
    canvas.print(str(tdsValue));
  }
    
  else if (lastState == LOW && currentState == HIGH){
    Serial.println("The button is released");
  }

  // save the the last state
  lastState = currentState;

  // event loop for MQTT client
  tb.loop(); 

  //Serial
  Serial.print("Voltage : "); 
  Serial.println(Voltage);
  Serial.print(" TDS Value : ");
  Serial.print(tdsValue);
  Serial.println(" ppm ");
  Serial.println("*****************");

  delay(1000);
}

