/*
 * multipurpose_module.cpp
 */
#include <Arduino.h>

#include <SPI.h>
#include <driver/pcnt.h>
#include <Wire.h>

#include "multipurpose_module.h"

multipurpose_module::multipurpose_module(){
  
}

void multipurpose_module::init(void){
  pinMode(ADC_CSb, OUTPUT); digitalWrite(ADC_CSb, 1);
  SPI.begin(ADC_SCK, ADC_MISO, ADC_MOSI, ADC_CSb);
  SPI.beginTransaction(SPISettings(100000, MSBFIRST, SPI_MODE0));
}

// read raw ADC data (full scale=4095 (12bit))
uint16_t multipurpose_module::ADCread(uint8_t ch) {
  uint16_t data = 0;
  uint8_t dh, dl;
  digitalWrite(ADC_CSb, 0);
  SPI.transfer(0x06);
  dh = SPI.transfer(ch << 6);
  dl = SPI.transfer(0x00);
  digitalWrite(ADC_CSb, 1);
  data = ((dh & 0x0f) << 8) | dl;
  return(data);
}

// read ADC voltage [V]
float multipurpose_module::ADCreadV(uint8_t ch) {
  return(3.3 * (float)ADCread(ch) / 4095.0);
}