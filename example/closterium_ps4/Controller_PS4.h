#pragma once
#include "Controller_BaseClass.h"
#include <PS4Controller.h>

/*
struct Input_PS4{
  void apply();
};
*/


struct Config_PS4{
  const char* mac = nullptr;
};

template <typename InputData>
class Controller_PS4 :public Controller_Base<Config_PS4,InputData>{
public:
  using Controller_Base<Config_PS4,InputData>::Controller_Base;
  bool begin() override{
    return PS4.begin(this->config->mac);
  }
  bool update() override{
    if(PS4.isConnected()){
      this->command->apply();
      return true;
    }
    return false;
  }
};

