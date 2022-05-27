#include "multipurpose_module.h"

multipurpose_module MM;
float tdsValue = 0;
float Voltage = 0;

void setup(void) {
  Serial.begin(115200);
  MM.init();
}

void loop(void) {
  //Measure
  Voltage = MM.ADCreadV(3);

  //Calcuration
  //Convert voltage value to TDS value
  tdsValue=(133.42/Voltage*Voltage*Voltage - 255.86*Voltage*Voltage + 857.39*Voltage)*0.5; 

  //Serial
  Serial.print("Voltage : "); 
  Serial.println(Voltage);
  Serial.print(" TDS Value : ");
  Serial.print(tdsValue);
  Serial.println(" ppm ");
  Serial.println("*****************");

  delay(1000);
}
