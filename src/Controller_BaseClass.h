#pragma once
#include <Arduino.h>

template <typename ConfigData, typename InputData>
class Controller_Base{
protected:
  ConfigData& config;
  InputData& command;
public:
  explicit Controller_Base(ConfigData& config, InputData& input):config(config),command(input){}
  virtual bool begin() = 0;
  virtual bool update() = 0;
  const InputData& get_input(){return this->command;}
  ConfigData& get_config(){return this->config;}
};


///////////////////////////
/*
struct Config_RemoteXY{};

class Controller_RemoteXY : public Controller_Base<Config_RemoteXY,...>{
public:
  bool begin() override{
    RemoteXY_Init();
  }
  bool updata() override{
    RemoteXYEngine.handler();
  }
};
*/
///////////////////////////
