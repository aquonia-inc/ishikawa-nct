/*
 * multipurpose_module.h
 */
#ifndef __MULTIPURPOSE_MODULE__
#define __MULTIPURPOSE_MODULE__
#include <Arduino.h>

#define ADC_CSb  5
#define ADC_SCK  18
#define ADC_MISO 19
#define ADC_MOSI 23
#define LED 17

class multipurpose_module
{
    public:
        multipurpose_module();
        uint16_t ADCread(uint8_t ch);
        float ADCreadV(uint8_t ch);
        void init();
};

#endif
