/**
 * @file Controller_BluetoothSerial.h
 * @brief BluetoothSerialで構造体をやり取りするライブラリ
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
#include <BluetoothSerial.h>

/*
struct InputData{
  //uint32_t angle;//degree
  //uint32_t dist;
  //uint32_t turn;
} __attribute__((packed));
*/

/** @brief BluetoothSerial用設定 */
struct Config_BluetoothSerial{
  const char* device_name = "ESP32_BT";
  uint32_t baud_rate = 115200;
};

/**
 * @brief BluetoothSerialで構造体を受信するクラス
 * 
 * @tparam InputData 相手から受け取るデータ(構造体)
 * @attention InputDataは__attribute__((__packed__))を付けて宣言し、パディングを無効化すること
 */
template <typename InputData>
class Controller_BluetoothSerial : public Controller_Base<Config_BluetoothSerial,InputData>{
public:

  using Controller_Base<Config_BluetoothSerial,InputData>::Controller_Base;

  /**
   * @brief setup()で呼ばれる初期化関数
   * @details 
   * 
   * @retval ture  初期化成功
   * @retval false 初期化失敗
   */
  bool begin() override{
    return BluetoothSerial.begin(this->config.device_name);
  }

  /**
   * @brief loop()内で呼ばれる値の更新を行う関数
   * @details データ量を指定して読み込み、余った分は捨てる
   * 
   * @retval true  更新あり
   * @retval false 更新なし
   */
  bool update() override{
    if(BluetoothSerial.available() >= sizeof(InputData)){
      BluetoothSerial.readBytes(reinterpret_cast<uint8_t*>(&this->command), sizeof(InputData));
      
      // 残ったゴミデータがあればすべて読み飛ばす
      while(BluetoothSerial.available() > 0){
        BluetoothSerial.read();
      }
      return true;
    }
    return false;
  }
};

template <typename InputData>
using Controller = Controller_BluetoothSerial<InputData>;


/**
 * @brief BluetoothSerialで構造体を送受信するクラス
 * 
 * @tparam InputData  相手から受け取るデータ(構造体)
 * @tparam OutputData 相手に送るデータ(構造体)
 * @attention InputData,OutputDataは__attribute__((__packed__))を付けて宣言し、パディングを無効化すること
 */
template <typename InputData, typename OutputData>
class Controller_BluetoothSerial_Response : public Controller_BluetoothSerial<InputData>{

private:
  OutputData& response;

public:

  /**
   * @brief Controller_BluetoothSerial_Response オブジェクトを作成
   * 
   * @param config 設定用構造体の参照
   * @param input  受け取るデータ(構造体)の参照
   * @param output 送るデータ(構造体)の参照
   */
  Controller_BluetoothSerial_Response(Config_BluetoothSerial& config, InputData& input, OutputData& output):
    Controller_BluetoothSerial<InputData>(config,input),response(output){}

  /**
   * @brief 構造体を相手に送る関数
   * 
   * @retval true  送信成功
   * @retval false 送信失敗
   */
  bool send(){
    return BluetoothSerial.write(reinterpret_cast<uint8_t*>(&this->response), sizeof(OutputData)) == sizeof(OutputData);
  }
};

template <typename InputData>
using Controller_Response = Controller_BlutoothSerial_Response<InputData>;

#endif
