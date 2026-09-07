/**
 * @file ESP32Controller_BluetoothSerial.h
 * @brief BluetoothSerialで構造体をやり取りするライブラリ
 * 
 * @author Tomoooji (https://github.com/Tomoooji)
 * @date 2026-09-07
 * @copyright Copyright (c) 2026
 * 
 * @todo SPPまわり
 */

#pragma once
#ifdef ESP32
#include <Arduino.h>
#include <BluetoothSerial.h>
#include "ESP32Controller_Base.h"

/** @brief BluetoothSerial用設定 */
struct Config_BluetoothSerial {
  const char* device_name = "ESP32_BT"; ///< デバイス名
  bool as_master = false; ///< trueならマスター、falseならスレーブ
};

/**
 * @brief BluetoothSerialで構造体を受信するクラス
 * 
 * @tparam InputData 相手から受け取るデータ(構造体)
 * @attention InputDataは__attribute__((__packed__))を付けて宣言し、パディングを無効化すること
 */
template <typename InputData>
class ESP32Controller_BluetoothSerial : public ESP32Controller_Base<Config_BluetoothSerial,InputData> {

protected:
  BluetoothSerial bluetoothserial_;

public:
  using ESP32Controller_Base<Config_BluetoothSerial,InputData>::ESP32Controller_Base;

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
using ESP32Controller = ESP32Controller_BluetoothSerial<InputData>;


/**
 * @brief BluetoothSerialで構造体を送受信するクラス
 * 
 * @tparam InputData  相手から受け取るデータ(構造体)
 * @tparam OutputData 相手に送るデータ(構造体)
 * @attention InputData,OutputDataは__attribute__((__packed__))を付けて宣言し、パディングを無効化すること
 */
template <typename InputData, typename OutputData>
class ESP32Controller_BluetoothSerial_Response : public ESP32Controller_BluetoothSerial<InputData> {

private:
  OutputData& output_;

public:
  /**
   * @brief ESP32Controller_BluetoothSerial_Response オブジェクトを作成
   * 
   * @param config_data 設定用構造体の参照
   * @param input_data  受け取るデータ(構造体)の参照
   * @param output_data 送るデータ(構造体)の参照
   */
  ESP32Controller_BluetoothSerial_Response(Config_BluetoothSerial& config_data, InputData& input_data, OutputData& output_data):
    ESP32Controller_BluetoothSerial<InputData>(config_data,input_data),output_(output_data) {}

  /**
   * @brief 構造体を相手に送る関数
   * 
   * @retval true  送信成功
   * @retval false 送信失敗
   */
  bool send() const {
    return this->bluetoothserial_.write(reinterpret_cast<uint8_t*>(&this->output_), sizeof(OutputData)) == sizeof(OutputData);
  }

  /**
   * @brief output オブジェクトを設定
   * 
   * @param new_output 新しく設定するoutputオブジェクトの参照
   * @return 設定したoutputオブジェクトへのconst参照
   * @code
   *  // 実体化してから設定
   *   OutputData new_output;
   *   new_output.value = 42;
   *   controller.set_output(new_output);
   * 
   *  // 実体化せずに直接設定
   *   controller.set_output(
   *    // ~C++17
   *     OutputData{42}
   *    // C++20以降
   *     OutputData{.value = 42}
   *   );
   * @endcode 
   */
  const OutputData& set_output(OutputData& new_output) {
    this->output_ = new_output;
    return this->output_;
  }
};

template <typename InputData, typename OutputData>
using ESP32Controller_Response = ESP32Controller_BluetoothSerial_Response<InputData,OutputData>;

#endif
