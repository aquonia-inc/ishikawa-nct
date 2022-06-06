// I2C setting
#include <Wire.h>

// Wifi settings
#include <WiFi.h>
#include <WireGuard-ESP32.h>
#include <HTTPClient.h>

// WiFi configuration --- UPDATE this configuration for your WiFi AP
char ssid[] = "******";
char password[] = "******";

// WireGuard configuration --- UPDATE this configuration from JSON
char private_key[] = "***";  // [Interface] PrivateKey
IPAddress local_ip(10,253,247,83);            // [Interface] Address
char public_key[] = "***";     // [Peer] PublicKey
char endpoint_address[] = "***";    // [Peer] Endpoint
int endpoint_port = 11010;              // [Peer] Endpoint
static constexpr const uint32_t UPDATE_INTERVAL_MS = 5000;
static WireGuard wg;
static HTTPClient httpClient;

// M5Unit OLED settings
#include <M5UnitOLED.h>
M5UnitOLED display;
M5Canvas canvas(&display);

// LED Button settings
#define LEDbuttonLEDPin 27
#define LEDbuttonButtonPin 4
int LEDbuttonStatus = LOW;
unsigned long pushTimebuffer = 500;      //change accordingly
unsigned long LEDbutton_toggleTime = 0;

// AquoWare Multipurpose Module settings
#include "multipurpose_module.h"
multipurpose_module MM;

// TDS Sensor settings
float tdsValue = 0;
float tdsVoltage = 0;

// pH Sensor settings
float phValue = 0;
float phVoltage = 0;

// M5Unit EnvⅠⅠ & EnvⅠⅠⅠ settings
#include <SHT3x.h>
SHT3x sht30;
#include "Adafruit_Sensor.h"
// EnvⅠⅠ
// #include <Adafruit_BMP280.h>
// Adafruit_BMP280 bme;
// EnvⅠⅠⅠ
#include "QMP6988.h"
QMP6988 qmp;
float tmp = 0.00;
float hum = 0.00;
float pressure = 0.00;

// M5Unit GPS settings
#include <TinyGPS++.h>
TinyGPSPlus gps;
HardwareSerial ss(2);
#define RXD2 13
#define TXD2 14
float lat=35.170915;
float lng=136.881537;

// Water Temperature settings
#include <OneWire.h>
#include <DallasTemperature.h>
#define ONE_WIRE_BUS 33
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
float waterTemp = 0;

void setup(void) {
  
  Serial.begin(115200);
  MM.init();
  ss.begin(9600, SERIAL_8N1, RXD2, TXD2);
  Wire.begin(); 
  qmp.init(); 
  sht30.Begin();
  sensors.begin();

  pinMode(LEDbuttonButtonPin, INPUT);
  pinMode(LEDbuttonLEDPin, OUTPUT);
  digitalWrite(LEDbuttonLEDPin, LEDbuttonStatus);

  display.init();
  display.setRotation(0);
  canvas.setColorDepth(1);
  canvas.setTextWrap(false);
  canvas.setTextSize(1.2);
  canvas.setTextScroll(false);
  canvas.createSprite(display.width() , display.height());
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("WiFi Failed");
    while(1) {
        delay(1000);
    }
  }
  Serial.println("Adjusting system time...");
  configTime(9 * 60 * 60, 0, "ntp.jst.mfeed.ad.jp", "ntp.nict.jp", "time.google.com");

  Serial.println("Connected. Initializing WireGuard...");
  wg.begin(
    local_ip,
    private_key,
    endpoint_address,
    public_key,
    endpoint_port
  );
}

void loop(void) {
  //Measure
  phVoltage = MM.ADCreadV(0);
  tdsVoltage = MM.ADCreadV(2);

  //Calcuration
  //Convert voltage value to TDS value and pH value
  phValue=2.405*phVoltage+3.9166;
  tdsValue=(133.42/tdsVoltage*tdsVoltage*tdsVoltage - 255.86*tdsVoltage*tdsVoltage + 857.39*tdsVoltage)*0.5;
  
  sht30.UpdateData();
  tmp = sht30.GetTemperature();
  hum = sht30.GetRelHumidity();
  pressure = qmp.calcPressure();

  // water temperature
  sensors.requestTemperatures();
  waterTemp = sensors.getTempCByIndex(0);

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
    WiFi.begin(ssid,password);
    return;
  }
  
  while (ss.available()) {
    gps.encode(ss.read());
    lat=gps.location.lat();
    lng=gps.location.lng();
    // Serial.println(gps.location.lat(),6);
    // Serial.println(gps.location.lng(),6);
  }

   
  WiFiClient client;

  if( !client.connect("uni.soracom.io", 80) ) {
    Serial.println("Failed to connect...");
    delay(5000);
    return;
  }

  uint64_t uptime_msec = millis();
  Serial.printf("Sending uptime %lu [ms]\r\n", uptime_msec);
  String json;
  json += "{\"pH\":";
  json.concat(phValue); //json.concat(static_cast<unsigned long>(phValue));
  json += ",\"TDS\":";
  json.concat(tdsValue);
  json += ",\"Tempreture\":";
  json.concat(tmp);
  json += ",\"WaterTempreture\":";
  json.concat(waterTemp);
  json += ",\"Humidity\":";
  json.concat(hum);
  json += ",\"AirPressure\":";
  json.concat(pressure);
  json += ",\"Latitude\":";
  json.concat(lat);
  json += ",\"Longitude\":";
  json.concat(lng);
  json += "}";
  Serial.printf("payload: %s\r\n", json.c_str());
  
  
  delay(UPDATE_INTERVAL_MS);

  int reading = digitalRead(LEDbuttonButtonPin);
  
  if(reading==LOW && millis()-LEDbutton_toggleTime>pushTimebuffer){
    LEDbuttonStatus = !LEDbuttonStatus;
    LEDbutton_toggleTime = millis();
    Serial.println("The button is pressed");
    client.write("POST / HTTP/1.1\r\n");
    client.write("Host: harvest.soracom.io\r\n");
    client.write("Connection: Keep-Alive\r\n");
    client.write("Keep-Alive: timeout=5, max=2\r\n");
    client.write("Content-Type: application/json\r\n");
    client.write("Content-Length: ");
    client.write(String(json.length(), 10).c_str());
    client.write("\r\n\r\n");
    client.write(json.c_str());

    while(client.connected()) {
      auto line = client.readStringUntil('\n');
      Serial.write(line.c_str());
      Serial.write("\n");
      if( line == "\r" ) break;
    }
    if(client.connected()) {
      uint8_t buffer[256];
      size_t bytesToRead = 0;
      while((bytesToRead = client.available()) > 0) {
        bytesToRead = bytesToRead > sizeof(buffer) ? sizeof(buffer) : bytesToRead;
        auto bytesRead = client.readBytes(buffer, bytesToRead); 
        Serial.write(buffer, bytesRead);
      }
    }
    canvas.pushSprite(0, 5);
    // TDS
    canvas.setCursor(0, 0); 
    canvas.print("TDS");
    canvas.setCursor(15, 10); 
    canvas.print(String(int(round(tdsValue))));
    canvas.print("ppm");
    
    // pH
    canvas.setCursor(0, 20); 
    canvas.print("pH");
    canvas.setCursor(15, 30);
    canvas.print(String(int(round(phValue))));

    // Temperature
    canvas.setCursor(0, 40);
    canvas.print("Temp");    
    canvas.setCursor(15, 50);
    canvas.print(String(int(round(tmp))));
    canvas.print("C");
    
    // Humidity
    canvas.setCursor(0, 60); 
    canvas.print("Humidity");
    canvas.setCursor(15, 70);    
    canvas.print(String(int(round(hum))));
    canvas.print("%");

    // Water Temperature
    canvas.setCursor(0, 80);
    canvas.print("WaterTemp");
    canvas.setCursor(15, 90);
    canvas.print(String(int(round(waterTemp))));
    canvas.print("C");
    
    // Air Pressure
    canvas.setCursor(0, 100); 
    canvas.print("Air");
    canvas.setCursor(15, 110);
    canvas.print(String(int(round(pressure/100))));
    canvas.print("hPa");
    
  }
  if(reading==HIGH) {
    digitalWrite(LEDbuttonLEDPin, LOW);
  }

  //Serial
  Serial.print("tdsVoltage : "); 
  Serial.println(tdsVoltage);
  Serial.print(" TDS Value : ");
  Serial.print(tdsValue);
  Serial.println(" ppm ");
  Serial.println("*****************");

  delay(500);
}

