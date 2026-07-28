/**
 * @file Controller_BluetoothSerial.h
 * @brief BluetoothSerialで構造体をやり取りするライブラリ
 * 
 * @author Tomoooji (https://github.com/Tomoooji)
 * @date 2026-07-27
 * @copyright Copyright (c) 2026
 * 
 * @todo SPPまわり
 */

#pragma once

#ifdef ESP32

#include <Arduino.h>
#include <BluetoothSerial.h>

#include "ESP32_Controller_Base.h"

/*
struct InputData {
  //uint32_t angle;//degree
  //uint32_t dist;
  //uint32_t turn;
} __attribute__((packed));
*/

/** @brief BluetoothSerial用設定 */
struct Config_BluetoothSerial {
  const char* device_name;
  bool as_master; ///< trueならマスター、falseならスレーブ
  Config_BluetoothSerial(const char* device_name = "ESP32_BT", bool as_master = false):device_name(device_name),as_master(as_master) {}
};

/**
 * @brief BluetoothSerialで構造体を受信するクラス
 * 
 * @tparam InputData 相手から受け取るデータ(構造体)
 * @attention InputDataは__attribute__((__packed__))を付けて宣言し、パディングを無効化すること
 */
template <typename InputData>
class Controller_BluetoothSerial : public Controller_Base<Config_BluetoothSerial,InputData> {

protected:
  BluetoothSerial bluetoothserial_;

public:
  using Controller_Base<Config_BluetoothSerial,InputData>::Controller_Base;

  /**
   * @brief setup()で呼ばれる初期化関数
   * @details マスターの場合はデバイス名で接続を試みる
   * 
   * @retval true  初期化成功
   * @retval false 初期化失敗
   */
  bool begin() override {
    if (!this->bluetoothserial_.begin(this->config_.device_name, this->config_.as_master)) return false;
    if (this->config_.as_master) {
      this->bluetoothserial_.connect(this->config_.device_name);
      return this->bluetoothserial_.connected();
    }
    return true;
  }

  /**
   * @brief loop()内で呼ばれる値の更新を行う関数
   * @details データ量を指定して読み込み、余った分は捨てる
   * 
   * @retval true  更新あり
   * @retval false 更新なし
   */
  bool update() override {
    if (this->bluetoothserial_.available() >= sizeof(InputData)) {
      this->bluetoothserial_.readBytes(reinterpret_cast<uint8_t*>(&this->input_), sizeof(InputData));
      while(this->bluetoothserial_.available() > 0) {
        this->bluetoothserial_.read();
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
class Controller_BluetoothSerial_Response : public Controller_BluetoothSerial<InputData> {

private:
  OutputData& output_;

public:

  /**
   * @brief Controller_BluetoothSerial_Response オブジェクトを作成
   * 
   * @param config_data 設定用構造体の参照
   * @param input_data  受け取るデータ(構造体)の参照
   * @param output_data 送るデータ(構造体)の参照
   */
  Controller_BluetoothSerial_Response(Config_BluetoothSerial& config_data, InputData& input_data, OutputData& output_data):
    Controller_BluetoothSerial<InputData>(config_data,input_data),output_(output_data) {}

  /**
   * @brief 構造体を相手に送る関数
   * 
   * @retval true  送信成功
   * @retval false 送信失敗
   */
  bool send() {
    return this->bluetoothserial_.write(reinterpret_cast<uint8_t*>(&this->output_), sizeof(OutputData)) == sizeof(OutputData);
  }
};

template <typename InputData, typename OutputData>
using Controller_Response = Controller_BluetoothSerial_Response<InputData,OutputData>;

#endif
