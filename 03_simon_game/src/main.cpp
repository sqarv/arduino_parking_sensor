#include <Arduino.h>

// VARIABLES
#define start_pin 7
#define supply_pin 8
#define buzzer_pin 2
//                R,Y,G,B
int led_pins[] = {6,3,4,5};
int notes_freq[] = {300,450,600,750};
bool buttons_debounce[] = {0,0,0,0};
bool start_button_debounce = false;
int pins_count = sizeof(led_pins) / sizeof(int);

// CONFIGURATION
int sound_time = 250;
int led_on_time = 500;
int between_led_delay = 200;
bool in_game = false;
int start_sequence_length = 1;

//DATA
int sequence_length = start_sequence_length;
int highest_score = 0;
int *led_sequence;

// HELPER FUNCTIONS
void play_end_sound(bool win){
  tone(buzzer_pin,win ? 1250 : 1000,sound_time);
  delay(sound_time);
  tone(buzzer_pin,win ? 2000 : 500,sound_time * 2);
  delay(1000);
}

void setup(){
  pinMode(start_pin,INPUT_PULLUP);
  pinMode(buzzer_pin,OUTPUT);
  pinMode(supply_pin,OUTPUT);
  pinMode(LED_BUILTIN,OUTPUT);
  
  digitalWrite(supply_pin,LOW);
  digitalWrite(buzzer_pin,LOW);
  digitalWrite(LED_BUILTIN,LOW);
}

void loop(){
  bool start_button_pressed = digitalRead(start_pin) == 0;
  
  if (start_button_pressed){
    if (!start_button_debounce){
      start_button_debounce = true;
      
      in_game = !in_game;
      
      sequence_length = start_sequence_length;
      highest_score = 0;
      digitalWrite(supply_pin,LOW);
      digitalWrite(LED_BUILTIN,in_game);
    }
  }
  else{
    start_button_debounce = false;
  }
  
  if (in_game){                                             // game logic
                                                    //generate random led sequence
    if(led_sequence == nullptr){
      srand(analogRead(A0));
      led_sequence = new int[sequence_length];
    }
    
    int idx = rand() % pins_count;
    led_sequence[sequence_length - 1] = idx;
    
    for (int i = 0 ; i < sequence_length ; i++){
      int idx = led_sequence[i];
      int selected_pin = led_pins[idx];
      int sound_freq = notes_freq[idx];
      led_sequence[i] = idx;
      
      pinMode(selected_pin,OUTPUT);
      digitalWrite(selected_pin,HIGH);
      tone(buzzer_pin,sound_freq,led_on_time);
      delay(led_on_time);
      digitalWrite(selected_pin,LOW);
      delay(between_led_delay);
    }
    
                                                    //player input
                                                    //set player input logic
    for (int i = 0; i < pins_count;i++){
      pinMode(led_pins[i],INPUT);
    }
    digitalWrite(supply_pin,HIGH);
    
                                                    // detecting input and checking if its correct
    bool sequence_completed = true;
    int position_in_sequence = 0;
    
    while (sequence_completed && position_in_sequence < sequence_length){
      for (int idx = 0; idx < pins_count;idx++){
        int led_pin = led_pins[idx];
        bool button_pressed = digitalRead(led_pin);
        
        if (button_pressed){
          if(!buttons_debounce[idx]){               // button pressed logic
            buttons_debounce[idx] = 1;
            
            int button_to_press = led_sequence[position_in_sequence];
            if(button_to_press == idx){             // pressed the correct button
              position_in_sequence++;
            }
            else{                                    // pressed button is wrong 
              sequence_completed = false;
            }
            tone(buzzer_pin,notes_freq[idx],led_on_time);
            
            break;
          }
        }
        else{
          if(buttons_debounce[idx]){
            buttons_debounce[idx] = 0;
          }
        }
      }
    }
    
    delay(1000);
    
    if (sequence_completed){                          // player remembered the sequence - add score and increase length of next sequence
      play_end_sound(true);
      
      //set new highest score
      if (sequence_length > highest_score){
        highest_score = sequence_length;
      }
      
      sequence_length ++;
      led_sequence = (int*)realloc(led_sequence,sequence_length);
    }
    else{                                             // player forgot the sequence - start from the beggining
      play_end_sound(false);
      
      delete[] led_sequence;
      led_sequence = nullptr;
      sequence_length = start_sequence_length;
    }
    
    digitalWrite(supply_pin,LOW);
  }
  
}