
int last_value1 = 0;
int last_value2 = 0;
int cur_value1 = 0;
int cur_value2 = 0;

void setup() {
  pinMode(36, INPUT); //set pin mode to input.设置引脚模式为输入模式
  pinMode(26, INPUT);
  Serial.begin(115200);
}

void loop() {
  cur_value1 = digitalRead(36); // read the value of BUTTON.  读取22号引脚的值
  cur_value2 = digitalRead(26);
  Serial.println.print("Btn.1  Btn.2");
  Serial.println.print("Value: ");
  Serial.println.print("State: ");
  if(cur_value1 != last_value1){
    if(cur_value1==0){
      Serial.println.print("0"); // display the status
      Serial.println.print("pre");
    }
    else{
      Serial.println.print("1"); // display the status
      Serial.println.print("rel");
    }
    last_value1 = cur_value1;
  }
  if(cur_value2 != last_value2){
    if(cur_value2==0){
      Serial.println.print("0"); // display the status
      Serial.println.print("pre");
    }
    else{
      Serial.println.print("1"); // display the status
      Serial.println.print("rel");
    }
    last_value2 = cur_value2;
  }
}