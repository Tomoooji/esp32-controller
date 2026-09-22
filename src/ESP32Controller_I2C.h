/**
 * @file ESP32Controller_I2C.h
 * @brief I2Cで構造体をやりとりするライブラリ
 * 
 * @author Tomoooji (https://github.com/Tomoooji)
 * @date 2026-09-07
 * @copyright Copyright (c) 2026
 * 
 * @attention Slave側にはC++17以降でないと動かないコードが含まれます。
 * @note 基本的に機体側はMasterとして運用、Slaveは一旦放置！
 */

#pragma once
#ifdef ESP32
#include <Arduino.h>
#include <Wire.h>
#include "ESP32Controller_Base.h"

namespace ESP32ControllerInternal {

/**
 * @brief I2C(Master)で構造体を受け取るクラス
 * 
 * @tparam InputData 相手から受け取るデータ(構造体)
 * @attention InputDataは__attribute__((__packed__))を付けて宣言し、パディングを無効化すること
 * @note 機体側は他のI2C機器との接続がありうるのでMasterとして運用
 */
template <typename InputData>
class ESP32Controller_I2CMaster : public Base<ESP32Controller_I2CMaster::Config, InputData> {
public:
  /**　@brief I2C通信用の設定　*/
  struct Config : public Base<Config, InputData>::ConfigStruct {
    uint8_t address_slave; //初期値は 0x2A など
    int sda = -1;
    int scl = -1;
    uint32_t frequency = 0; // 0ならデフォルトの100kHz
  };
  using Base<Config, InputData>::Base;

  /**
   * @brief setup()で呼ばれる初期化関数
   * @details MasterとしてI2Cを初期化し、結果を返す
   * 
   * @retval true  初期化成功
   * @retval false 初期化失敗f
   */
  bool begin() override {
    // マスター初期化（アドレス指定しない）
    return Wire.begin(this->config_.sda, this->config_.scl, this->config_.frequency);
  }

  /**
   * @brief loop()内で呼ばれる値の更新を行う関数
   * @details 
   * 
   * @note もしかしたらreadByteがエラー吐くかも?
   * @retval true  更新あり
   * @retval false 更新なし
   */
  bool update() override {
    // スレーブからデータを要求
    Wire.requestFrom(this->config_.address_slave, (size_t)sizeof(InputData));
    
    if (Wire.available() >= sizeof(InputData))  {
      // 受信バッファから構造体のメモリ領域へ直接バイナリとして読み込む
      Wire.readBytes(reinterpret_cast<uint8_t*>(&this->input_),sizeof(InputData));
      // ↑動かなかったら↓下のを使ってね
      /*uint8_t* bytePtr = reinterpret_cast<uint8_t*>(&_currentCmd);
      for (size_t i = 0; i < sizeof(Robotinput); i++)  {
          bytePtr[i] = Wire.read();
      }*/
      // 残ったゴミデータがあればすべて読み飛ばしてバッファを空にする
      while(Wire.available() > 0) {
        Wire.read();
      }
      return true;
    }
    return false;
  }
};

/////////////////

/**
 * @brief I2C(Master)で構造体を送受信するクラス
 * @details 
 * 
 * @tparam InputData  相手から受け取るデータ(構造体)
 * @tparam OutputData 相手に送るデータ(構造体)
 * @attention InputData,OutputDataは__attribute__((__packed__))を付けて宣言し、パディングを無効化すること
 * @note 機体側は他のI2C機器との接続がありうるのでMasterとして運用
 */
template <typename InputData, typename OutputData>
class ESP32Controller_Response_I2CMaster : public ResponseBase<ESP32Controller_I2CMaster<InputData>, ESP32Controller_I2CMaster::Config, InputData, OutputData> {
public:
  using ResponseBase<ESP32Controller_I2CMaster<InputData>, ESP32Controller_I2CMaster::Config, InputData, OutputData>::ResponseBase;
  
  /**
   * @brief 構造体を相手に送る関数
   * 
   * @retval true  送信成功
   * @retval false 送信失敗
   */
  bool send() const override {
    // マスターがスレーブへデータを送信
    Wire.beginTransmission(this->config_.address_slave);
    Wire.write(reinterpret_cast<uint8_t*>(&this->output_), sizeof(OutputData));
    return Wire.endTransmission() == 0;
  }
};

// ============================================
// スレーブ用（外部マスターからコマンド受信）-> 基本使わない方針で
// ============================================

/**
 * @brief I2C(Slave)で構造体を受け取るクラス
 * 
 * @tparam InputData 相手から受け取るデータ(構造体)
 * @attention InputDataは__attribute__((__packed__))を付けて宣言し、パディングを無効化すること
 */

template <typename InputData>
class ESP32Controller_I2CSlave : public Base<ESP32Controller_I2CSlave::Config, InputData> {
protected:
  portMUX_TYPE recv_mux = portMUX_INITIALIZER_UNLOCKED;
  InputData input_buffer_; 
  inline static ESP32Controller_I2CSlave *_instance = nullptr; //!< C++17以上でないと使えない
  
  /**
   * @brief 受信時のコールバック関数
   * @details 受け取ったデータをinput_buffer_にコピーし、configの新規受信フラグを立てる
   * 
   * @param size 受け取ったデータのサイズ
   * @see ESP32Controller_I2CSlave_Response::static_recv_cb
   */
  static void static_recv_cb(int size) {
    if (_instance == nullptr) return;
    if (size >= sizeof(InputData)) {
      portENTER_CRITICAL(&_instance->recv_mux);
      Wire.readBytes(reinterpret_cast<uint8_t*>(&_instance->input_buffer_),sizeof(InputData));
      // ↑動かなかったら↓下のを使ってね
      /*uint8_t* bytePtr = reinterpret_cast<uint8_t*>(&_currentCmd);
      for (size_t i = 0; i < sizeof(Robotinput); i++) {
          bytePtr[i] = Wire.read();
      }*/
      while(Wire.available() > 0) {
        Wire.read();
      }
      _instance->config_.receive_new = true;
      portEXIT_CRITICAL(&_instance->recv_mux);
    }
  }

public:
  /** @brief I2C通信用の設定(スレーブ用) */
  struct Config : public Base<Config, InputData>::ConfigStruct {
    uint8_t address;
    int sda = -1;
    int scl = -1;
    uint32_t frequency = 0; // 0ならデフォルトの100kHz
    volatile bool receive_new;
    bool is_connect = false;
  };

  using Base<Config, InputData>::Base;

  /**
   * @brief setup()で呼ばれる初期化関数
   * @details SlaveとしてI2Cを初期化、コールバック関数の登録を行って結果を返す
   * 
   * @retval true  初期化成功
   * @retval false 初期化失敗
   */
  bool begin() override {
    // スレーブ初期化（アドレス指定）
    if (!Wire.begin(this->config_.address, this->config_.sda, this->config_.scl)) return false;
    _instance = this;
    Wire.onReceive(static_recv_cb);
    this->config_.is_connect = true;
    return true;
  }

  /**
   * @brief loop()内で呼ばれる値の更新(のチェック)を行う関数
   * @details コールバック関数が更新してくれたinput_buffer_からinput_にコピーし、フラグを倒す。
   * 
   * @retval true  更新あり
   * @retval false 更新なし
   * @note コピーしてる間はCritical Sectionでコールバック関数を止めている。
   * @see ESP32Controller_I2CSlave_Response::update
   */
  bool update() override {
    // ↓ 多分あってるけど、もしかしたらportENTER_CRITICAL_ISRの方が正解かもしれない
    // ↓ onReceiveに渡すコールバック関数内でxPortIsrContext()を実行してtrueだったらそっちに変えてくださいな
    portENTER_CRITICAL(&this->recv_mux);
    // ここに巨大な処理を入れると大変なのでInputDataは控えめなサイズにする
      if (this->config_.receive_new) {
        memcpy(&this->input_,&this->input_buffer_,sizeof(InputData));
        this->config_.receive_new = false;
        this->config_.is_connect = true;
      } else {
        this->config_.is_connect = false;
      }
      // ↓ こちらも同じく。I2Cの受信コールバックがISRならそれ用に_ISRつけたやつを呼ぶ必要がある。
      portEXIT_CRITICAL(&this->recv_mux);
    return this->config_.is_connect;
  }
};

//////////////////


/**
 * @brief I2C(Slave)で構造体を送受信するクラス
 * 
 * @tparam InputData 相手から受け取るデータ(構造体)
 * @tparam OutputData   相手に送るデータ(構造体)
 * 
 * @note コールバック関数は継承できないので双方向verもBaseからの継承にしている
 * @attention InputData,OutputDataは__attribute__((__packed__))を付けて宣言し、パディングを無効化すること
 * @attention 受信onlyの方でstatic_recv_cbを変更してもこちらとは同期されてない
 */
template <typename InputData, typename OutputData>
class ESP32Controller_Response_I2CSlave : public ResponseBase<ESP32Controller_I2CSlave<InputData>, ESP32Controller_Response_I2CSlave::Config, InputData, OutputData> {
private:
  OutputData output_buffer_; //!< 送信バッファ

  /**
   * @brief リクエスト受信時のコールバック関数
   * @details Masterからのリクエストに応じてデータを送信
   */
  static void static_request_cb() {
    if (_instance == nullptr) return;
    // ↓ 多分あってるけど、もしかしたらportENTER_CRITICAL_ISRの方が正解かもしれない
    portENTER_CRITICAL(&_instance->recv_mux);
    _instance->config_.send_success = Wire.write(reinterpret_cast<uint8_t*>(&_instance->output_buffer_), sizeof(OutputData)) == sizeof(OutputData);
    portEXIT_CRITICAL(&_instance->recv_mux);
  }

public:
  /** @brief I2C通信用の設定(スレーブ、送受信用) */
  struct Config : public ESP32Controller_I2CSlave<InputData>::Config {
    volatile bool send_success;
  };

  using ResponseBase<ESP32Controller_I2CSlave<InputData>, Config, InputData, OutputData>::ResponseBase;

  /**
   * @brief setup()で呼ばれる初期化関数
   * @details SlaveとしてI2Cを初期化、コールバック関数の登録を行って結果を返す
   * 
   * @retval true  初期化成功
   * @retval false 初期化失敗
   */
  bool begin() override {
    // スレーブ初期化
    if (!Wire.begin(this->config_.address, this->config_.sda, this->config_.scl)) return false;
    _instance = this;
    Wire.onReceive(static_recv_cb);
    Wire.onRequest(static_request_cb);
    this->config_.is_connect = true;
    return true;
  }

  /**
   * @brief loop()内で呼ばれる値の更新(のチェック)を行う関数
   * @details コールバック関数が更新してくれたinput_buffer_からinput_にコピーし、フラグを倒す。
   * 
   * @retval true  更新あり
   * @retval false 更新なし
   * @note コピーしてる間はCritical Sectionでコールバック関数を止めている。
   * @see ESP32Controller_I2CSlave_Response::update
   */
  bool update() override {
    // ↓ 多分あってるけど、もしかしたらportENTER_CRITICAL_ISRの方が正解かもしれない
    // ↓ onReceiveに渡すコールバック関数内でxPortIsrContext()を実行してtrueだったらそっちに変えてくださいな
    portENTER_CRITICAL(&this->recv_mux);
    if (this->config_.receive_new) {
      // ここに巨大な処理を入れると大変なのでInputDataは控えめなサイズにする
      memcpy(&this->input_,&this->input_buffer_,sizeof(InputData));
      memcpy(&this->output_buffer_,&this->output_,sizeof(OutputData));// 送信バッファにコピー
      this->config_.receive_new = false;
      this->config_.is_connect = true;
    } else {
      this->config_.is_connect = false;
    }
    // ↓ こちらも同じく。I2Cの受信コールバックがISRならそれ用に_ISRつけたやつを呼ぶ必要がある。
    portEXIT_CRITICAL(&this->recv_mux);
    return this->config_.is_connect;
  }

  bool send() const override {
    return this->config_.is_connect;
  }
};

} // namespace ESP32ControllerInternal

template <typename InputData>
using ESP32Controller = ESP32ControllerInternal::ESP32Controller_I2CMaster<InputData>;
template <typename InputData, typename OutputData>
using ESP32Controller_Response = ESP32ControllerInternal::ESP32Controller_Response_I2CMaster<InputData,OutputData>;

#endif
