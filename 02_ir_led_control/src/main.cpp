#include <Arduino.h>
#include <IRremote.h>

#define ir_pin 3
#define led_pin 5

void setup(){
    pinMode(led_pin,OUTPUT);
    digitalWrite(led_pin,LOW);
    IrReceiver.begin(ir_pin);
    Serial.begin(9600);
}

IRDecodedRawDataType last_state = 0x0;
uint8_t led_brightness = UINT8_MAX;
uint8_t default_step = 1;
uint8_t step = default_step;
uint8_t max_step = 20;

bool led_state = false;
bool input_press = false;
bool input_begin = false;

//buttons
const IRDecodedRawDataType play_code = 0xBC43FF00;
const IRDecodedRawDataType increase_code = 0xEA15FF00;
const IRDecodedRawDataType decrease_code = 0xF807FF00;

void loop(){
  if(IrReceiver.decode()){
    IRDecodedRawDataType data = IrReceiver.decodedIRData.decodedRawData;
    input_press = false;
    input_begin = false;
    
    if(data == 0x0 && last_state != play_code){
        data = last_state;
        input_press = true;
    }
    else{
        last_state = data;
        input_begin = true;
    }

    if (data == increase_code || data == decrease_code){
        if(input_begin){
            step = default_step;
        }
        if(input_press){
            step = constrain(step+1,default_step,max_step);
        }
    }
    
    switch(data){
        case play_code: // turn led on/off
        {
            led_state = !led_state;
            led_brightness = UINT8_MAX;
            break;
        }
        case increase_code: // increase brightness
        {
            if (led_state){
                led_brightness = constrain(led_brightness + step,0,UINT8_MAX);
            }
            break;
        }
        case decrease_code: // deacrease brigthness
        {
            if (led_state){
                led_brightness = constrain(led_brightness - step,0,UINT8_MAX);
            }
            break;
        }
    }
    
    IrReceiver.resume();
  }
  
  led_state ? analogWrite(led_pin,led_brightness) : analogWrite(led_pin,0);
}