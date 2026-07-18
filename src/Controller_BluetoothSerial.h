/**
 * @file Controller_BluetoothSerial.h
 * @brief 
 * 
 * @author Tomoooji (https://github.com/Tomoooji)
 * @version 0.1
 * @date 2026-07-18
 * @copyright Copyright (c) 2026
 * 
 * @note 
 */

#ifdef ESP32
#pragma once

#include "Controller_BaseClass.h"
#include <BluetoothSerial.h>

/*
struct InputData{
  //uint32_t angle;//degree
  //uint32_t dist;
  //uint32_t turn;
} __attribute__((packed));
*/

struct Config_BluetoothSerial{
  const char* device_name = "ESP32_BT";
  uint32_t baud_rate = 115200;
};

// ============================================
// BluetoothSerial（受信のみ）
// ============================================
template <typename InputData>
class Controller_BluetoothSerial : public Controller_Base<Config_BluetoothSerial,InputData>{
public:
  using Controller_Base<Config_BluetoothSerial,InputData>::Controller_Base;

  bool begin() override{
    return BluetoothSerial.begin(this->config.device_name);
  }

  bool update() override{
    if(BluetoothSerial.available() >= sizeof(InputData)){
      BluetoothSerial.readBytes(reinterpret_cast<uint8_t*>(&this->command), sizeof(InputData));
      
      // 残ったゴミデータがあればすべて読み飛ばす
      while (BluetoothSerial.available() > 0) {
        BluetoothSerial.read();
      }
      return true;
    }
    return false;
  }
};
template <typename InputData>
using Controller = Controller_BluetoothSerial<InputData>;

// ============================================
// BluetoothSerial（双方向通信）
// ============================================
template <typename InputData, typename OutputData>
class Controller_BluetoothSerial_Response : public Controller_BluetoothSerial<InputData>{
private:
  OutputData& response;

public:
  Controller_BluetoothSerial_Response(Config_BluetoothSerial& config, InputData& input, OutputData& output):
    Controller_BluetoothSerial<InputData>(config,input),response(output) {}

  bool send(){
    return BluetoothSerial.write(reinterpret_cast<uint8_t*>(&this->response), sizeof(OutputData)) == sizeof(OutputData);
  }
};
template <typename InputData>
using Controller_Response = Controller_BlutoothSerial_Response<InputData>;

#endif
