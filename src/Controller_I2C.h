#pragma once
#include "Controller_BaseClass.h"
#include <Wire.h>

/*
struct InputData{
  //uint32_t angle;//degree
  //uint32_t dist;
  //uint32_t turn;
} __attribute__((packed));
*/

struct Config_I2C{
  uint8_t address; //初期値は 0x2A など
  int sda = -1;
  int scl = -1;
  uint32_t frequency = 0;
};

template <typename InputData>
class Controller_I2C : public Controller_Base<Config_I2C,InputData>{
private:
public:
  using Controller_Base<Config_I2C,InputData>::Controller_Base;
  //memset(&_currentCmd, 0, sizeof(RobotCommand)); // 構造体をゼロクリア

  bool begin() override{
    return Wire.begin(this->config.sda, this->config.scl, this->config.frequency);
  }

};

/////////////////

template <typename InputData, typename OutputData>
class Controller_I2C_Response : public Controller_I2C<InputData>{
private:
  OutputData& response;
public:
  Controller_I2C_Response(Config_Serial& config, InputData& input, OutputData& output):
    config(config),command(input),response{output}{}

  bool send(){
    // 送信処理の開始
    Wire.beginTransmission(this->config.address);
    // 構造体のメモリ領域をバイト配列（uint8_t*）にキャストして、丸ごと16バイト送信
    Wire.write(reinterpret_cast<uint8_t*>(&response), sizeof(OutputData));
    return Wire.endTransmisson() == 0;
  }

};



/*

bool update() override{
  if (Wire.available() >= sizeof(InputData)) {
    Wire.readBytes(reinterpret_cast<uint8_t*>(&this->command), sizeof(InputData));
    while (Wire.available() > 0) {
      Wire.read();
    }
    return true;
  }
  return false;
}
*/
