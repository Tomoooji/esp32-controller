#define CONTROLLER_TYPE CONTROLLER_PS4
#include <ESP32_Controller.h>

struct WheelCommand{
  int speed_L;
  int speed_R;
  void apply(){
    this->speed_L = abs(PS4.LStickY())>20 ? PS4.LStickY() : 0;
    this->speed_R = abs(PS4.RStickY())>20 ? PS4.RStickY() : 0;
  }
} input;
Config_PS4 config = {.mac="00:00:00:00:00:00"};
Controller<WheelCommand> ps4pad(config,input);

enum MOTOR_ID{
  MOTOR_FL,MOTOR_ML,MOTOR_BL,
  MOTOR_FR,MOTOR_MR,MOTOR_BR
};

constexpr uint8_t pins[] = {
  //FL,    ML,    BL,   FR,    MR,   BR
  14,27, 26,25, 33,32, 16,17, 5,18, 19,21
};


void setup(){
  ps4pad.begin();
  for(uint8_t pin : pins){
    ledcAttach(pin,12800,8);
    ledcWrite(pin,0);
  }
}

void loop(){
  if(ps4pad.update()){
    rotate(MOTOR_FL,ps4pad.get_input().speed_L);
    rotate(MOTOR_ML,ps4pad.get_input().speed_L);
    rotate(MOTOR_BL,ps4pad.get_input().speed_L);

    rotate(MOTOR_FR,ps4pad.get_input().speed_R);
    rotate(MOTOR_MR,ps4pad.get_input().speed_R);
    rotate(MOTOR_BR,ps4pad.get_input().speed_R);
  }
}

void rotate(uint8_t ID,int speed){
  ledcWrite(pins[ID*2],speed>0 ? speed : 0);
  ledcWrite(pins[ID*2+1],speed>0 ? 0 : -speed);
}
