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
class Controller_I2C : public Controller<Config_I2C,InputData>{
private:
public:
  using Controller<Config_I2C,InputData>::Controller;
  //memset(&_currentCmd, 0, sizeof(RobotCommand)); // 構造体をゼロクリア

  bool begin() override{
    return Wire.begin(this->config.address, this->config.sda, this->config.scl, this->config.frequency);
  }

  bool update() override{
    int availableBytes = Wire.available();
    // 構造体のサイズ以上のデータがバッファに届いているかチェック
    if (availableBytes >= sizeof(InputData)) {
      // 受信バッファから構造体のメモリ領域へ直接バイナリとして読み込む
      uint8_t* bytePtr = reinterpret_cast<uint8_t*>(&this->command);
      for (size_t i = 0; i < sizeof(InputData); i++) {
        bytePtr[i] = Wire.read();
      }
      
      // 残ったゴミデータがあればすべて読み飛ばしてバッファを空にする
      while (Wire.available() > 0) {
        Wire.read();
      }
      return true;
    }
    return false;
  }
};

