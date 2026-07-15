#define CONTROLLER_TYPE CONTROLLER_ESPNOW
#include <ESP32_Controller.h>

struct BlinkCommand{
  bool is_on;
  int power;
} command;
Config_ESPNOW config;
Controller<BlinkCommand> esprimocon(config,command);

constexpr uint8_t pin_led = 2;

void setup(){
  if(!esprimocon.begin()) return;
  ledcAttach(pin_led, OUTPUT);
}

void loop(){
  if(esprimocon.update()){
    if(!esprimocon.get_input().is_on){
      ledcWrite(pin_led, 0);
    }else{
      ledcWrite(pin_led, esprimocon.get_input().power);
    }
  }
  delay(10);
}