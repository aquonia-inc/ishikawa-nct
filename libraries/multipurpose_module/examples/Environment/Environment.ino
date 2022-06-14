#include <SHT3x.h>
#include <Wire.h>
#include "Adafruit_Sensor.h"
#include <Adafruit_BMP280.h>

SHT3x sht30;
Adafruit_BMP280 bme;

void setup() {
    Serial.begin(115200);
    Wire.begin();               // I2Cを初期化する
    while (!bme.begin(0x76)) {  // BMP280を初期化する
        Serial.println("BMP280 init fail");
    }
    sht30.Begin();
}

void loop() {
    sht30.UpdateData();
    float tmp = sht30.GetTemperature();
    float hum = sht30.GetRelHumidity();
    float pressure = bme.readPressure();
    
    Serial.printf("temp: %4.1f'C\r\n", tmp);
    Serial.printf("humid:%4.1f%%\r\n", hum);
    Serial.printf("press:%4.0fhPa\r\n", pressure / 100);
    delay(1000);
}