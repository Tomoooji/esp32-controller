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
#include <BluetoothSerial.h>
#include "ESP32Controller_Base.h"

/**
 * @brief BluetoothSerialで構造体を受信するクラス
 * 
 * @tparam InputData 相手から受け取るデータ(構造体)
 * @attention InputDataは__attribute__((__packed__))を付けて宣言し、パディングを無効化すること
 */
template <typename InputData>
class ESP32Controller_BluetoothSerial : public ESP32ControllerBase<ESP32Controller_BluetoothSerial::Config_BluetoothSerial, InputData> {
protected:
  BluetoothSerial bluetoothserial_;

public:
  /** @brief BluetoothSerial用設定 */
  struct Config_BluetoothSerial  : public ESP32ControllerBase<Config_BluetoothSerial, InputData>::ConfigStruct {
    const char* device_name = "ESP32_BT"; ///< デバイス名
    bool as_master = false; ///< trueならマスター、falseならスレーブ
  };

  /** @brief コンストラクタ */
  using ESP32ControllerBase<Config_BluetoothSerial, InputData>::ESP32ControllerBase;

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
    return true; // スレーブは接続待ちなので成功扱い
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
class ESP32Controller_Response_BluetoothSerial : public ESP32ControllerResponseBase<ESP32Controller_BluetoothSerial<InputData>, ESP32Controller_BluetoothSerial::Config_BluetoothSerial, InputData, OutputData> {
public:
  using ESP32ControllerResponseBase<ESP32Controller_BluetoothSerial<InputData>, ESP32Controller_BluetoothSerial::Config_BluetoothSerial, InputData, OutputData>::ESP32ControllerResponseBase;
  /**
   * @brief 構造体を相手に送る関数
   * 
   * @retval true  送信成功
   * @retval false 送信失敗
   */
  bool send() const override {
    return this->bluetoothserial_.write(reinterpret_cast<uint8_t*>(&this->output_), sizeof(OutputData)) == sizeof(OutputData);
  }
};

template <typename InputData, typename OutputData>
using ESP32Controller_Response = ESP32Controller_Response_BluetoothSerial<InputData,OutputData>;

#endif
