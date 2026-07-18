/**
 * @file Controller_I2C.h
 * @brief I2Cで構造体をやりとりするライブラリ
 * 
 * @author Tomoooji
 * @version 0.1
 * @date 2026-07-18
 * @copyright Copyright (c) 2026
 * 
 * @note 基本的に機体側はMasterとして運用、Slaveは一旦放置！
 */

#ifdef ESP32
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

/**　@brief I2C通信用の設定　*/
struct Config_I2C_Master{
  uint8_t address_slave; //初期値は 0x2A など
  int sda = -1;
  int scl = -1;
  uint32_t frequency = 0;
};

/**
 * @brief 
 * @details 
 * 
 * @tparam InputData 
 */
template <typename InputData>
class Controller_I2C_Master : public Controller_Base<Config_I2C_Master,InputData>{
private:
public:
  using Controller_Base<Config_I2C_Master,InputData>::Controller_Base;

  /**
   * @brief 
   * @details 
   * 
   * @retval true 
   * @retval false 
   */
  bool begin() override{
    // マスター初期化（アドレス指定しない）
    return Wire.begin(this->config.sda, this->config.scl, this->config.frequency);
  }

  /**
   * @brief 
   * @details 
   * 
   * @retval true 
   * @retval false 
   */
  bool update() override{
    // スレーブからデータを要求
    Wire.requestFrom(this->config.address_slave, (size_t)sizeof(InputData));
    
    if (Wire.available() >= sizeof(InputData)) {
      // 受信バッファから構造体のメモリ領域へ直接バイナリとして読み込む
      Wire.readBytes(reinterpret_cast<uint8_t*>(&this->command),sizeof(InputData));

      // 残ったゴミデータがあればすべて読み飛ばしてバッファを空にする
      while (Wire.available() > 0) {
        Wire.read();
      }
      return true;
    }
    return false;
  }
};
template <typename InputData>
using Controller = Controller_I2C_Master<InputData>;

/////////////////

/**
 * @brief 
 * @details 
 * 
 * @tparam InputData 
 * @tparam OutputData 
 */
template <typename InputData, typename OutputData>
class Controller_I2C_Master_Response : public Controller_I2C_Master<InputData>{
private:
  OutputData& response;

public:
  /**
   * @brief Construct a new Controller_I2C_Master_Response object
   * @details 
   * 
   * @param config 
   * @param input 
   * @param output 
   */
  Controller_I2C_Master_Response(Config_I2C_Master& config, InputData& input, OutputData& output):
  Controller_I2C_Master<InputData>(config,input),response(output){}

  /**
   * @brief 
   * @details 
   * 
   * @retval true 
   * @retval false 
   */
  bool send(){
    // マスターがスレーブへデータを送信
    Wire.beginTransmission(this->config.address);
    Wire.write(reinterpret_cast<uint8_t*>(&this->response), sizeof(OutputData));
    return Wire.endTransmission() == 0;
  }
};
template <typename InputData>
using Controller_Response = Controller_I2C_Master_Response<InputData>;






// ============================================
// スレーブ用（外部マスターからコマンド受信）-> 基本使わない方針で
// ============================================

/*
volatile struct InputData{
  //uint32_t angle;//degree
  //uint32_t dist;
  //uint32_t turn;
} __attribute__((packed));
*/


struct Config_I2C_Slave{
  uint8_t address; //初期値は 0x2A など
  int sda = -1;
  int scl = -1;
  uint32_t frequency = 0;
  volatile bool receive_new = false;
};


template <typename InputData>
class Controller_I2C_Slave : public Controller_Base<Config_I2C_Slave,InputData>{
private:
  inline static Controller_I2C_Slave *_instance = nullptr;
  
  static void static_recv_cb(int size){
    if(_instance == nullptr) return;
    if (size >= sizeof(InputData)) {
      Wire.readBytes(reinterpret_cast<uint8_t*>(&this->command),sizeof(InputData));
      while (Wire.available() > 0) {
        Wire.read();
      }
      _instance->config.receive_new = true;
    }
  }

public:
  using Controller_Base<Config_I2C_Slave,InputData>::Controller_Base;

  bool begin() override{
    // スレーブ初期化（アドレス指定）
    Wire.begin(this->config.address, this->config.sda, this->config.scl);
    _instance = this;
    Wire.onReceive(static_recv_cb);
    return true;
  }

  bool update() override{
    if(this->config.receive_new){
      this->config.receive_new = false;
      return true;
    }
    return false;
  }
};

//////////////////
struct Config_I2C_Slave_Response{
  uint8_t address; //初期値は 0x2A など
  int sda = -1;
  int scl = -1;
  uint32_t frequency = 0;
  volatile bool receive_new = false;
  volatile bool send_success = false;
};

template <typename InputData, typename OutputData>
class Controller_I2C_Slave_Response : public Controller_Base<Config_I2C_Slave_Response,InputData,Config_I2C_Slave_Response>{
private:
  OutputData& response;
  inline static Controller_I2C_Slave_Response *_instance = nullptr;

  static void static_recv_cb(int size){
    if(_instance == nullptr) return;
    if (size >= sizeof(InputData)) {
      Wire.readBytes(reinterpret_cast<uint8_t*>(&this->command),sizeof(InputData));
      while (Wire.available() > 0) {
        Wire.read();
      }
      _instance->config.receive_new = true;
    }
  }

  static void static_request_cb(){
    if(_instance == nullptr) return;
    _instance->config.send_success = Wire.write(reinterpret_cast<uint8_t*>(&_instance->response), sizeof(OutputData)) == sizeof(OutputData);
  }

public:
  Controller_I2C_Slave_Response(Config_I2C_Slave_Response& config, InputData& input, OutputData& output):
    Controller_Base<Config_I2C_Slave_Response,InputData>(config,input),response(output){}

  bool begin() override{
    // スレーブ初期化
    Wire.begin(this->config.address, this->config.sda, this->config.scl);
    _instance = this;
    Wire.onReceive(static_recv_cb);
    Wire.onRequest(static_request_cb);
    return true;
  }

  bool update() override{
    if(this->config.receive_new){
      this->config.receive_new = false;
      return true;
    }
    return false;
  }

};

#endif