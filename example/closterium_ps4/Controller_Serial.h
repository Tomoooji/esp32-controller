#pragma once
#include "Controller_BaseClass.h"

/*
struct InputData{
  //uint32_t angle;//degree
  //uint32_t dist;
  //uint32_t turn;
} __attribute__((packed));
*/

struct Config_Serial{
  int baudrate;
  int Rx = -1;
  int Tx = -1;
};

template <typename InputData>
class Controller_Serial : public Controller<Config_Serial,InputData>{
private:
  HardwareSerial& SER;

public:
  Controller_Serial(HardwareSerial& serial, ConfigData& config, InputData& input):
    SER(serial),config(config),command(input){}

  bool begin() override{
    this->SER.begin(this->config.baudrate, SERIAL_8N1, this->config.Rx, this->config.Tx);
    // 8ビット、パリティなし、ストップビット1（8N1）
    return this->SER;
  }

  bool update() override{
    if(this->SER.available()){
      this->SER.readBytes((uint8_t*)&this->command, sizeof(InputData));
      //this->SER.write((uint8_t*)&this->command, sizeof(InputData)); //feedback echo
      return true;
    }
    return false;
  }
};
