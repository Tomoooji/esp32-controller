/**
 * @file Controller_Serial.h
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

#include "ESP32_Controller_BaseClass.h"

/*
struct InputData{
  //uint32_t angle;//degree
  //uint32_t dist;
  //uint32_t turn;
} __attribute__((packed));
*/

/** @brief  */
struct Config_Serial{
  int baudrate = 115200;
  int Rx = -1;
  int Tx = -1;
};

/**
 * @brief 
 * 
 * @tparam InputData 相手から受け取るデータ(構造体)
 */
template <typename InputData>
class Controller_Serial : public Controller_Base<Config_Serial,InputData>{

private:
  HardwareSerial& SER;

public:

  /**
   * @brief Controller_Serial オブジェクトを作成
   * 
   * @param serial Serial or Serial2
   * @param config 
   * @param input 
   */
  Controller_Serial(HardwareSerial& serial, Config_Serial& config, InputData& input):
    Controller_Base<Config_Serial,InputData>(config,input),SER(serial){}

  /**
   * @brief setup()で呼ばれる初期化関数
   * @details 
   * 
   * @retval ture 初期化成功
   * @retval false 初期化失敗
   * @note SERIAL_8N1 = 8ビット、パリティなし、ストップビット1（8N1）
   */
  bool begin() override{
    this->SER.begin(this->config.baudrate, SERIAL_8N1, this->config.Rx, this->config.Tx);
    return this->SER;
  }

  /**
   * @brief loop()内で呼ばれる値の更新を行う関数
   * @details 
   * 
   * @retval true 更新あり
   * @retval false 更新なし
   */
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
template <typename InputData>
using Controller = Controller_Serial<InputData>;

//////////

/**
 * @brief 
 * 
 * @tparam InputData 相手から受け取るデータ(構造体)
 * @tparam OutputData 
 */
template <typename InputData, typename OutputData>
class Controller_Serial_Response : public Controller_Serial<InputData>{

private:

  OutputData& response;

public:

  /**
   * @brief Controller_Serial_Response オブジェクトを作成
   * 
   * @param serial 
   * @param config 
   * @param input 
   * @param output 
   */
  Controller_Serial_Response(HardwareSerial& serial, Config_Serial& config, InputData& input, OutputData& output):
    Controller_Serial<InputData>(serial,config,input),response{output}{}

  /**
   * @brief loop()内で呼ばれる値の更新を行う関数
   * @details 
   * 
   * @retval true 更新あり
   * @retval false 更新なし
   */
  bool send(){
    return this->SER.write(reinterpret_cast<uint8_t*>(&this->response), sizeof(OutputData)) == sizeof(OutputData);
  }
};

template <typename InputData>
using Controller_Response = Controller_Serial_Response<InputData>;

#endif