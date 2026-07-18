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
  int baudrate = 115200;
  int Rx = -1;
  int Tx = -1;
};

template <typename InputData>
class Controller_Serial : public Controller_Base<Config_Serial,InputData>{
private:
  HardwareSerial& SER;

public:
  Controller_Serial(HardwareSerial& serial, Config_Serial& config, InputData& input):
    Controller_Base<InputData>(config,input),SER(serial){}

  bool begin() override{
    this->SER.begin(this->config.baudrate, SERIAL_8N1, this->config.Rx, this->config.Tx);
    // 8ビット、パリティなし、ストップビット1（8N1）
    return this->SER;
  }

  bool update() override{
    if(this->SER.available() >= sizeof(InputData)){
      this->SER.readBytes(reinterpret_cast<uint8_t*>(&this->command), sizeof(InputData));
      while (this->SER.available() > 0) {
        this->SER.read();
      }
      return true;
    }
    return false;
  }
};

//////////

template <typename InputData, typename OutputData>
class Controller_Serial_Response : public Controller_Serial<Config_Serial,InputData>{
private:
  OutputData& response;

public:
  Controller_Serial_Response(HardwareSerial& serial, Config_Serial& config, InputData& input, OutputData& output):
    Controller_Serial<InputData>(serial,config,input),response{output}{}

  bool send(){
    return this->SER.write(reinterpret_cast<uint8_t*>(&this->response), sizeof(OutputData)) == sizeof(OutputData);
  }
};

