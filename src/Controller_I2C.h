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

// ============================================
// マスター用（機体側がマスター）
// ============================================
template <typename InputData>
class Controller_I2C_Master : public Controller_Base<Config_I2C,InputData>{
private:
public:
  using Controller_Base<Config_I2C,InputData>::Controller_Base;

  bool begin() override{
    // マスター初期化（アドレス指定しない）
    return Wire.begin(this->config.sda, this->config.scl, this->config.frequency);
  }

  bool update() override{
    // スレーブからデータを要求
    Wire.requestFrom(this->config.address, (size_t)sizeof(InputData));
    
    if (Wire.available() >= sizeof(InputData)) {
      // 受信バッファから構造体のメモリ領域へ直接バイナリとして読み込む
      //*
      uint8_t* bytePtr = reinterpret_cast<uint8_t*>(&this->command);
      for (size_t i = 0; i < sizeof(InputData); i++) {
        bytePtr[i] = Wire.read();
      }
      /*Wire.readByte(reinterpret_cast<uint8_t*>(&this->command),sizeof(InputData));*/

      // 残ったゴミデータがあればすべて読み飛ばしてバッファを空にする
      while (Wire.available() > 0) {
        Wire.read();
      }
      return true;
    }
    return false;
  }
};

/////////////////

template <typename InputData, typename OutputData>
class Controller_I2C_Master_Response : public Controller_I2C_Master<InputData>{
private:
  OutputData& response;

public:
  Controller_I2C_Master_Response(Config_I2C& config, InputData& input, OutputData& output):
  Controller_I2C_Master<InputData>(config,input),response(output){}

  bool send(){
    // マスターがスレーブへデータを送信
    Wire.beginTransmission(this->config.address);
    Wire.write(reinterpret_cast<uint8_t*>(&this->response), sizeof(OutputData));
    return Wire.endTransmission() == 0;
  }
};

// ============================================
// スレーブ用（外部マスターからコマンド受信）
// ============================================
template <typename InputData>
class Controller_I2C_Slave : public Controller_Base<Config_I2C,InputData>{
private:
  inline static Controller_I2C_Slave *_instance = nullptr;
  
  static void static_recv_cb(int size){
    if(_instance == nullptr) return;
    
    if (size >= sizeof(InputData)) {
      uint8_t* bytePtr = reinterpret_cast<uint8_t*>(&_instance->command);
      for (int i = 0; i < sizeof(InputData); i++) {
        bytePtr[i] = Wire.read();
      }
      // 残りのデータを捨てる
      while (Wire.available() > 0) {
        Wire.read();
      }
    }
  }

public:
  using Controller_Base<Config_I2C,InputData>::Controller_Base;

  bool begin() override{
    // スレーブ初期化（アドレス指定）
    Wire.begin(this->config.address, this->config.sda, this->config.scl);
    _instance = this;
    Wire.onReceive(static_recv_cb);
    return true;
  }

  bool update() override{
    // スレーブはコールバックで自動更新
    // 必要に応じて追加処理
    return true;
  }
};

//////////////////

template <typename InputData, typename OutputData>
class Controller_I2C_Slave_Response : public Controller_I2C_Slave<InputData>{
private:
  OutputData& response;
  inline static Controller_I2C_Slave_Response *_instance = nullptr;
  
  static void static_recv_cb(int size){
    if(_instance == nullptr) return;
    
    if (size >= sizeof(InputData)) {
      uint8_t* bytePtr = reinterpret_cast<uint8_t*>(&_instance->command);
      for (int i = 0; i < sizeof(InputData); i++) {
        bytePtr[i] = Wire.read();
      }
      while (Wire.available() > 0) {
        Wire.read();
      }
    }
  }
  
  static void static_request_cb(){
    if(_instance == nullptr) return;
    Wire.write(reinterpret_cast<uint8_t*>(&_instance->response), sizeof(OutputData));
  }

public:
  Controller_I2C_Slave_Response(Config_I2C& config, InputData& input, OutputData& output):
    Controller_I2C_Slave<InputData>(config,input),response(output){}

  bool begin() override{
    // スレーブ初期化
    Wire.begin(this->config.address, this->config.sda, this->config.scl);
    _instance = this;
    Wire.onReceive(static_recv_cb);
    Wire.onRequest(static_request_cb);
    return true;
  }

  bool update() override{
    // スレーブはコールバックで自動更新
    return true;
  }
};

