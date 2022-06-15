// I2C setting
#include <Wire.h>
 
// M5Unit OLED settings
#include <M5UnitOLED.h>
M5UnitOLED display;
M5Canvas canvas(&display);

#define GROVE_C_1 25
#define GROVE_C_2 26
#define GROVE_D_1 35 // analog in
#define GROVE_D_2 34 // analog in

// AquoWare Multipurpose Module settings
#include <multipurpose_module.h>
multipurpose_module MM;

// TDS Sensor settings
float tdsValue = 0;
float tdsVoltage = 0;

// pH Sensor settings
float phValue = 0;
float phVoltage = 0;

// M5Unit Button settings
int lastState = LOW;
int currentState;

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
float lat=36.66213044;
float lng=136.7382814;

// Water Temperature settings
#include <OneWire.h>
#include <DallasTemperature.h>
#define ONE_WIRE_BUS 32
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
float waterTemp = 0;

void setup(void) {
  
  Serial.begin(115200);
  MM.init();
  ss.begin(9600, SERIAL_8N1, RXD2, TXD2);
  Wire.begin();               // I2Cを初期化する
  // if (!bme.begin(0x76)) {  // BMP280を初期化する
  //   Serial.println("BMP280 init fail");
  // }
  qmp.init(); 
  sht30.Begin();
  sensors.begin();

  pinMode(GROVE_C_1, INPUT);

  display.init();
  display.setRotation(0);
  canvas.setColorDepth(1); // mono color
  // canvas.setFont(&fonts::lgfxJapanMinchoP_32);
  canvas.setTextWrap(false);
  canvas.setTextSize(1.2);
  canvas.setTextScroll(false);
  canvas.createSprite(display.width() , display.height());
}

void loop(void) {
  //Measure
  phVoltage = MM.ADCreadV(0);
  tdsVoltage = MM.ADCreadV(2);

  //Calcuration
  //Convert voltage value to TDS value and pH value
  phValue=2.405*phVoltage+3.9166; // pro v1
  // phValue=0.0018*MM.ADCread(0)+4.166;  // normal v1
  tdsValue=(133.42/tdsVoltage*tdsVoltage*tdsVoltage - 255.86*tdsVoltage*tdsVoltage + 857.39*tdsVoltage)*0.5;
  
  sht30.UpdateData();
  tmp = sht30.GetTemperature();
  hum = sht30.GetRelHumidity();
  //EnvⅠⅠ  
  //pressure = bme.readPressure();
  //EnvⅠⅠⅠ
  pressure = qmp.calcPressure();

  // water temperature
  sensors.requestTemperatures();
  waterTemp = sensors.getTempCByIndex(0);

  currentState = digitalRead(GROVE_C_1);
  
  while (ss.available()) {
    gps.encode(ss.read());
    lat=gps.location.lat();
    lng=gps.location.lng();
    // Serial.println(gps.location.lat(),6);
    // Serial.println(gps.location.lng(),6);
  }
  if (lastState == HIGH && currentState == LOW) {
    Serial.println("The button is pressed");
  }
  else if (lastState == LOW && currentState == HIGH){
    Serial.println("The button is released");
  }

  
  canvas.pushSprite(0, 0);
  
  // TDS
  canvas.setCursor(0, 0); 
  canvas.print("TDS");
  canvas.setCursor(25, 0); 
  canvas.print(String(int(round(tdsValue))));
  canvas.print("ppm ");
  
  // pH
  canvas.setCursor(0, 12); 
  canvas.print("pH");
  canvas.setCursor(35, 12);
  canvas.print(String(phValue));

  // Temperature
  canvas.setCursor(0, 24);
  canvas.print("Temp");    
  canvas.setCursor(40, 24);
  canvas.print(String(int(round(tmp))));
  canvas.print("C");
  
  // Humidity
  canvas.setCursor(0, 36); 
  canvas.print("Humid");
  canvas.setCursor(40, 36);    
  canvas.print(String(int(round(hum))));
  canvas.print("%");

  // Water Temperature
  canvas.setCursor(0, 48);
  canvas.print("WTemp");
  canvas.setCursor(40, 48);
  canvas.print(String(int(round(waterTemp))));
  canvas.print("C");

  // Lat
  canvas.setCursor(0, 60);
  canvas.print("Lat");
  canvas.setCursor(27, 60);
  canvas.print(String(round(lat),2));
  // Lng
  canvas.setCursor(0, 72);
  canvas.print("Lng");
  canvas.setCursor(27, 72);
  canvas.print(String(round(lng),2));
  
  // Air Pressure
  canvas.setCursor(0, 84); 
  canvas.print("Air");
  canvas.setCursor(15, 96);
  canvas.print(String(int(round(pressure/100))));
  canvas.print("hPa");
  
  // Button
  canvas.setCursor(0,108); 
  canvas.print("Button is");
  canvas.setCursor(10, 120); 
  if(lastState == HIGH && currentState == LOW){
    canvas.print("Pressed!");
  }else{
    canvas.print("Released");
  }


  // Serial
  Serial.print("TDS Value : ");
  Serial.print(tdsValue);
  Serial.println(" ppm ");
  Serial.print("pH Value ");
  Serial.println(phValue);
  Serial.print("Water temp: ");
  Serial.println(waterTemp);
  Serial.printf("temp: %4.1f'C\r\n", tmp);
  Serial.printf("humid:%4.1f%%\r\n", hum);
  Serial.printf("press:%4.0fhPa\r\n", pressure / 100);
  Serial.printf("Latitude:%4.2f%\r\n", lat);
  Serial.printf("Longitude:%4.2f\r\n", lng);
  Serial.println("*****************");
  // Serial.println(currentState);
  // Serial.println(lastState);
  // save the the last state
  lastState = currentState;

  delay(500);
}

