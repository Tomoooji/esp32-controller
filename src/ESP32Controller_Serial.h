/**
 * @file ESP32Controller_Serial.h
 * @brief シリアル通信(UART)で構造体をやり取りするライブラリ
 * 
 * @author Tomoooji (https://github.com/Tomoooji)
 * @date 2026-09-07
 * @copyright Copyright (c) 2026
 * 
 * @note 
 */

#pragma once
#ifdef ESP32
#include "ESP32Controller_Base.h"


/**
 * @brief シリアル通信(UART)で構造体を受け取るクラス
 * 
 * @tparam InputData 相手から受け取るデータ(構造体)
 * @attention InputDataは__attribute__((__packed__))を付けて宣言し、パディングを無効化すること
 */
template <typename InputData>
class ESP32Controller_Serial : public ESP32ControllerBase<ESP32Controller_Serial::Config_Serial, InputData> {
protected:
  HardwareSerial& serial_;

public:
  /** @brief シリアル通信(UART)の設定 */
  struct Config_Serial : public ESP32ControllerBase<Config_Serial, InputData>::ConfigStruct {
    int baudrate = 115200;
    int Rx = -1;
    int Tx = -1;
  };
  /**
   * @brief ESP32Controller_Serial オブジェクトを作成
   * 
   * @param serial Serial or Serial2
   * @param config_data 設定用構造体の参照
   * @param input_data 受け取るデータ(構造体)の参照
   */
  ESP32Controller_Serial(HardwareSerial& serial, Config_Serial &&config_data, InputData &&input_data)
  : ESP32ControllerBase<Config_Serial, InputData>(Config_Serial{std::move(config_data)}, std::move(input_data)), serial_(serial) {}

  /**
   * @brief setup()で呼ばれる初期化関数
   * 
   * @retval true  初期化成功
   * @retval false 初期化失敗
   * @note SERIAL_8N1 = 8ビット、パリティなし、ストップビット1（8N1）
   */
  bool begin() override {
    this->serial_.begin(this->config_.baudrate, SERIAL_8N1, this->config_.Rx, this->config_.Tx);
    return this->serial_;
  }

  /**
   * @brief loop()内で呼ばれる値の更新を行う関数
   * @details データ量を指定して読み込み、余った分は捨てる
   * 
   * @retval true  更新あり
   * @retval false 更新なし
   */
  bool update() override {
    if (this->serial_.available() >= sizeof(InputData)) {
      this->serial_.readBytes(reinterpret_cast<uint8_t*>(&this->input_), sizeof(InputData));
      while(this->serial_.available() > 0) {
        this->serial_.read();
      }
      return true;
    }
    return false;
  }
};

template <typename InputData>
using ESP32Controller = ESP32Controller_Serial<InputData>;

//////////

/**
 * @brief シリアル通信(UART)で構造体を送受信するクラス
 * 
 * @tparam InputData  相手から受け取るデータ(構造体)
 * @tparam OutputData 相手に送るデータ(構造体)
 * @attention InputData,OutputDataは__attribute__((__packed__))を付けて宣言し、パディングを無効化すること
 */
template <typename InputData, typename OutputData>
class ESP32Controller_Response_Serial : public ESP32ControllerResponseBase<ESP32Controller_Serial<InputData>, ESP32Controller_Serial<InputData>::Config_Serial, InputData, OutputData> {
public:
  /**
   * @brief ESP32Controller_Serial_Response オブジェクトを作成
   * 
   * @param serial Serial or Serial2
   * @param config_data 設定用構造体の参照
   * @param input_data 受け取るデータ(構造体)の参照
   * @param output_data 送るデータ(構造体)の参照
   */
  ESP32Controller_Response_Serial(HardwareSerial& serial, typename ESP32Controller_Serial<InputData>::Config_Serial &&config_data, InputData &&input_data, OutputData &&output_data)
      : ESP32ControllerResponseBase<ESP32Controller_Serial<InputData>, typename ESP32Controller_Serial<InputData>::Config_Serial, InputData, OutputData>(
        std::move(config_data), std::move(input_data), std::move(output_data)), serial_(serial) {}

  /**
   * @brief loop()内で呼ばれる値の更新を行う関数
   * 
   * @retval true  更新あり
   * @retval false 更新なし
   */
  bool send() override {
    return this->serial_.write(reinterpret_cast<uint8_t*>(&this->output_), sizeof(OutputData)) == sizeof(OutputData);
  }
};

template <typename InputData, typename OutputData>
using ESP32Controller_Response = ESP32Controller_Response_Serial<InputData,OutputData>;

#endif
