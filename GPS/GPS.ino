#include <TinyGPS++.h>
TinyGPSPlus gps;
HardwareSerial ss(2);

#include <Wire.h>

#define RXD2 13
#define TXD2 14

void setup(){
  Serial.begin(115200);
  ss.begin(9600, SERIAL_8N1, RXD2, TXD2);
  Serial.println("Setup");
}

float lat=35.170915;
float lng=136.881537;
void loop(){
  while (ss) {
    gps.encode(ss.read());
    lat=gps.location.lat();
    lng=gps.location.lng();
    Serial.println(gps.location.lat(),6);
    Serial.println(gps.location.lng(),6);
    delay(1000);
  }
}