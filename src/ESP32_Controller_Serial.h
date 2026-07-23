/**
 * @file Controller_Serial.h
 * @brief シリアル通信(UART)で構造体をやり取りするライブラリ
 * 
 * @author Tomoooji (https://github.com/Tomoooji)
 * @date 2026-07-23
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

/** @brief シリアル通信(UART)の設定 */
struct Config_Serial{
  int baudrate = 115200;
  int Rx = -1;
  int Tx = -1;
};

/**
 * @brief シリアル通信(UART)で構造体を受け取るクラス
 * 
 * @tparam InputData 相手から受け取るデータ(構造体)
 * @attention InputDataは__attribute__((__packed__))を付けて宣言し、パディングを無効化すること
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
   * @param config_data 設定用構造体の参照
   * @param input_data 受け取るデータ(構造体)の参照
   */
  Controller_Serial(HardwareSerial& serial, Config_Serial& config_data, InputData& input_data):
    Controller_Base<Config_Serial,InputData>(config_data,input_data),SER(serial){}

  /**
   * @brief setup()で呼ばれる初期化関数
   * 
   * @retval ture  初期化成功
   * @retval false 初期化失敗
   * @note SERIAL_8N1 = 8ビット、パリティなし、ストップビット1（8N1）
   */
  bool begin() override{
    this->SER.begin(this->config.baudrate, SERIAL_8N1, this->config.Rx, this->config.Tx);
    return this->SER;
  }

  /**
   * @brief loop()内で呼ばれる値の更新を行う関数
   * @details データ量を指定して読み込み、余った分は捨てる
   * 
   * @retval true  更新あり
   * @retval false 更新なし
   */
  bool update() override{
    if(this->SER.available() >= sizeof(InputData)){
      this->SER.readBytes(reinterpret_cast<uint8_t*>(&this->input), sizeof(InputData));
      while(this->SER.available() > 0){
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
 * @brief シリアル通信(UART)で構造体を送受信するクラス
 * 
 * @tparam InputData  相手から受け取るデータ(構造体)
 * @tparam OutputData 相手に送るデータ(構造体)
 * @attention InputData,OutputDataは__attribute__((__packed__))を付けて宣言し、パディングを無効化すること
 */
template <typename InputData, typename OutputData>
class Controller_Serial_Response : public Controller_Serial<InputData>{

private:

  OutputData& output;

public:

  /**
   * @brief Controller_Serial_Response オブジェクトを作成
   * 
   * @param serial Serial or Serial2
   * @param config_data 設定用構造体の参照
   * @param input_data 受け取るデータ(構造体)の参照
   * @param output_data 送るデータ(構造体)の参照
   */
  Controller_Serial_Response(HardwareSerial& serial, Config_Serial& config_data, InputData& input_data, OutputData& output_data):
    Controller_Serial<InputData>(serial,config_data,input_data),output{output_data}{}

  /**
   * @brief loop()内で呼ばれる値の更新を行う関数
   * 
   * @retval true  更新あり
   * @retval false 更新なし
   */
  bool send(){
    return this->SER.write(reinterpret_cast<uint8_t*>(&this->output), sizeof(OutputData)) == sizeof(OutputData);
  }
};

template <typename InputData, typename OutData>
using Controller_Response = Controller_Serial_Response<InputData,OutputData>;

#endif
