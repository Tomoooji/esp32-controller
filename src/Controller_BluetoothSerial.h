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
class Controller_BluetoothSerial : public Controller_Base<Config_BluetoothSerial, InputData>{
private:
  BluetoothSerial& BT;

public:
  Controller_BluetoothSerial(BluetoothSerial& bt, Config_BluetoothSerial& config, InputData& input):
    BT(bt), config(config), command(input) {}

  bool begin() override{
    return this->BT.begin(this->config.device_name);
  }

  bool update() override{
    if(this->BT.available() >= sizeof(InputData)){
      uint8_t* bytePtr = reinterpret_cast<uint8_t*>(&this->command);
      this->BT.readBytes(bytePtr, sizeof(InputData));
      
      // 残ったゴミデータがあればすべて読み飛ばす
      while (this->BT.available() > 0) {
        this->BT.read();
      }
      return true;
    }
    return false;
  }
};

// ============================================
// BluetoothSerial（双方向通信）
// ============================================
template <typename InputData, typename OutputData>
class Controller_BluetoothSerial_Response : public Controller_Base<Config_BluetoothSerial, InputData>{
private:
  BluetoothSerial& BT;
  OutputData& response;

public:
  Controller_BluetoothSerial_Response(BluetoothSerial& bt, Config_BluetoothSerial& config, InputData& input, OutputData& output):
    BT(bt), config(config), command(input), response(output) {}

  bool begin() override{
    return this->BT.begin(this->config.device_name);
  }

  bool update() override{
    if(this->BT.available() >= sizeof(InputData)){
      uint8_t* bytePtr = reinterpret_cast<uint8_t*>(&this->command);
      this->BT.readBytes(bytePtr, sizeof(InputData));
      
      // 残ったゴミデータがあればすべて読み飛ばす
      while (this->BT.available() > 0) {
        this->BT.read();
      }
      return true;
    }
    return false;
  }

  bool send(){
    size_t written = this->BT.write(reinterpret_cast<uint8_t*>(&this->response), sizeof(OutputData));
    return written == sizeof(OutputData);
  }
};

