#include <Arduino.h>
#include <SPI.h>
#include <driver/pcnt.h>
#include <Wire.h>

#include "core_module.h"

#include <rgb_lcd.h>
rgb_lcd lcd;

#define ADC_CSb  5
#define ADC_SCK  18
#define ADC_MISO 19
#define ADC_MOSI 23
#define TDS_RSEL0 26
#define TDS_RSEL1 27
#define MH_FLOW 4
#define GROVE_A_1 33 // SCL
#define GROVE_A_2 32 // SDA
#define GROVE_C_1 13 // RXD2
#define GROVE_C_2 14 // TXD2
#define GROVE_D_1 35 // analog in
#define GROVE_D_2 34 // analog in
#define LED 17

core_module core;

// Timer interrupt
// https://55life555.blog.fc2.com/blog-entry-3194.html
volatile int timeCounter1;
hw_timer_t *timer1 = NULL; 
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

// PWM output (for TDS)
// https://www.mgo-tec.com/blog-entry-ledc-pwm-arduino-esp32.html/2#title06
#define LEDC_CHANNEL_0 0
#define LEDC_TIMER_BIT 8
#define LEDC_BASE_FREQ 2400.0
#define TDS_CLK_PIN 12 // p14

uint16_t flow_count_per_sec = 0;
int i=3;

void IRAM_ATTR onTimer1(){
  portENTER_CRITICAL_ISR(&timerMux);
  flow_count_per_sec = core.read_flow_count();
  pcnt_counter_pause(PCNT_UNIT_0);
  pcnt_counter_clear(PCNT_UNIT_0);
  pcnt_counter_resume(PCNT_UNIT_0);
  portEXIT_CRITICAL_ISR(&timerMux);
}

void setup(void) {
  Wire.begin();

  lcd.begin(16, 2);
  
  pinMode(ADC_CSb, OUTPUT); digitalWrite(ADC_CSb, 1);
  pinMode(TDS_RSEL0, OUTPUT); digitalWrite(TDS_RSEL0, 0);
  pinMode(TDS_RSEL1, OUTPUT); digitalWrite(TDS_RSEL1, 0);
  SPI.begin(ADC_SCK, ADC_MISO, ADC_MOSI, ADC_CSb);
  SPI.beginTransaction(SPISettings(100000, MSBFIRST, SPI_MODE0));
  Serial.begin(115200);
  // counter for flow
  pcnt_config_t pcnt_config;
  pcnt_config.pulse_gpio_num = MH_FLOW;
  pcnt_config.ctrl_gpio_num = PCNT_PIN_NOT_USED;
  pcnt_config.lctrl_mode = PCNT_MODE_KEEP;
  pcnt_config.hctrl_mode = PCNT_MODE_KEEP;
  pcnt_config.channel = PCNT_CHANNEL_0;
  pcnt_config.unit = PCNT_UNIT_0;
  pcnt_config.pos_mode = PCNT_COUNT_INC;
  pcnt_config.neg_mode = PCNT_COUNT_DIS;
  pcnt_config.counter_h_lim = 1000;
  pcnt_config.counter_l_lim = -1000;
  
  pcnt_unit_config(&pcnt_config);
  pcnt_counter_pause(PCNT_UNIT_0);
  pcnt_counter_clear(PCNT_UNIT_0);
  pcnt_counter_resume(PCNT_UNIT_0);

  // timer interrupt for flow
  timer1 = timerBegin(0, 80, true);
  timerAttachInterrupt(timer1, &onTimer1, true);
  timerAlarmWrite(timer1, 1000000, true);
  timerAlarmEnable(timer1);

  // clock for TDS drive
  ledcSetup(LEDC_CHANNEL_0, LEDC_BASE_FREQ, LEDC_TIMER_BIT);
  ledcAttachPin(TDS_CLK_PIN, LEDC_CHANNEL_0);
  ledcWrite(LEDC_CHANNEL_0, 0x80);

  pinMode(LED, OUTPUT);
}

void loop(void) {
  //TDS Setting(3<2<1<0)
  float data=core.ADCread(2);
  while(data>=2000){
    i=i-1;
    core.SetTDSrange(i);
    delay(500);
    data=core.ADCread(2);
    if(i<=0){i=0; break;}
  }
  while(data<=12){
    i=i+1;
    core.SetTDSrange(i);
    delay(500);
    data=core.ADCread(2);
    if(i>=3){i=3; break;}
  }
 
  //Measure
  float temp=core.read_temp();
  float pressure=core.read_pressure();
  float flow=0.02847*flow_count_per_sec-0.3059;
  //float flow=0.0304*flow_count_per_sec-0.3584;
  float tds=core.read_TDS(i);

  // Grove LCD
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("TDS: ");
  lcd.print(TDS);
  lcd.print("mg/L");
  lcd.setCursor(0,1);
  lcd.print("Temp:");
  lcd.print(temp);
  lcd.print("C");

  delay(1000);
}
