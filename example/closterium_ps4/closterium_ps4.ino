#define CONTROLLER_TYPE CONTROLLER_PS4
#include "ESP32_Controller.h"

struct WheelCommand{
  int speed_R;
  int speed_L;
  void apply(){
    this->speed_L = PS4.LStickY();
    this->speed_R = PS4.RStickY();
  }
} input;

Config_PS4 config = {.mac="00:00:00:00:00:00"};

Controller<WheelCommand> ps4(config,input);

void setup(){}
void loop(){}