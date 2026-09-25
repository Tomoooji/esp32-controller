/**
 * @file ESP32Controller_ESPNOW.h
 * @brief ESP-NOWで構造体をやりとりするライブラリ
 * 
 * @attention C++20以上が必要です。
 * @attention 同じInputDataを指定したクラスでインスタンスを複数作るとコールバック関数が奪われます。
 * @note 入出力用の構造体には__attribute__((__packed__))を付けて宣言し、パディングを無効化することを推奨します。
 * 
 * @author Tomoooji (https://github.com/Tomoooji)
 * @version 2.0.0
 * @date 2026-09-22
 * @copyright Copyright (c) 2026
 */

#pragma once
#ifdef ESP32
#include <atomic>
#include <esp_now.h>
#include <WiFi.h>
#include "ESP32Controller_Base.h"

namespace ESP32ControllerInternal {

/**
 * @brief ESP-NOWで構造体を受け取るクラス
 * 
 * @tparam InputData 相手から受け取るデータ(構造体)
 * @attention InputDataは__attribute__((__packed__))を付けて宣言し、パディングを無効化すること
 */
template <typename InputData>
class ESP32Controller_ESPNOW : public Base<ESP32Controller_ESPNOW::Config, InputData> {
protected:
  portMUX_TYPE recv_mux = portMUX_INITIALIZER_UNLOCKED;
  InputData input_buffer_; 
  inline static ESP32Controller_ESPNOW *_instance = nullptr; //!< C++17以上でないと使えない
  
  /**
   * @brief 受信時のコールバック関数
   * @details 受け取ったデータをinput_buffer_にコピーし、configの新規受信フラグを立てる
   * 
   * @attention inputはパック済みの構造体である必要がある
   * @param addr (旧Ver用)送り手のアドレス
   * @param info (新Ver用)送り手のアドレスなどが入ってる
   * @param data 受け取ったデータ
   * @param len  受け取ったデータのサイズ
   * @see ESP32Controller_ESPNOW::static_recv_cb
   */
#if ESP_IDF_VERSION <= ESP_IDF_VERSION_VAL(5, 0, 0)
  static void static_recv_cb(const uint8_t* addr, const uint8_t* data, int len) {
    if (_instance == nullptr || sizeof(InputData) != len) return; // _instance->config_.receive_new || はいらないはず
    portENTER_CRITICAL(&_instance->recv_mux);
    memcpy(&_instance->input_buffer_, data, sizeof(InputData));
    _instance->config_.receive_new.store(true);
    portEXIT_CRITICAL(&_instance->recv_mux);
  }
#else
  static void static_recv_cb(const esp_now_recv_info_t* info, const uint8_t* data, int len) {
    if (_instance == nullptr || sizeof(InputData) != len) return; // _instance->config_.receive_new || はいらないはず
    portENTER_CRITICAL(&_instance->recv_mux);
    memcpy(&_instance->input_buffer_, data, sizeof(InputData));
    _instance->config_.receive_new.store(true);
    portEXIT_CRITICAL(&_instance->recv_mux);
  }
#endif

public:
  /** @brief ESP-NOW(受信only)用設定 */
  struct Config : public Base<Config, InputData>::ConfigStruct {
    std::atomic<bool> receive_new = false; ///< 値の更新フラグ
  };

  using Base<Config, InputData>::Base;
  
  /**
   * @brief setup()で呼ばれる初期化関数
   * @details WiFiのモード設定、ESP_NOWの初期化、コールバック関数の登録を行う
   * 
   * @retval true  初期化成功
   * @retval false 初期化失敗
   */
  bool begin() override {
    // WiFiモード設定 stationの略?
    WiFi.mode(WIFI_STA);
    if (esp_now_init() != ESP_OK) return false;
    // コールバック関数登録 static関数なので複数インスタンス作るとバグる
    _instance = this;
    esp_now_register_recv_cb(static_recv_cb);
    return true;
  }

  /**
   * @brief loop()内で呼ばれる値の更新(のチェック)を行う関数
   * @details コールバック関数が更新してくれたinput_buffer_からinput_にコピーし、フラグを倒す。
   * 
   * @retval true  更新あり
   * @retval false 更新なし
   * @note コピーしてる間はCritical Sectionでコールバック関数を止めている。
   * @see Controller_ESPNOW_Response::update
   */
  bool update() override {
    if (this->config_.receive_new.load()) {
      portENTER_CRITICAL(&this->recv_mux);
      // ここに巨大な処理を入れると大変だけどそもそもESP-NOWが扱えるデータ量(250バイト)的にmemcpyしてもそんなに重たくない...はず
      memcpy(&this->input_,&this->input_buffer_,sizeof(InputData));
      portEXIT_CRITICAL(&this->recv_mux);
      this->config_.receive_new.store(false);
      return true;
    }
    return false;
  }
};


/**
 * @brief ESP-NOWで構造体を送受信するクラス
 * 
 * @tparam InputData 相手から受け取るデータ(構造体)
 * @tparam OutData   相手に送るデータ(構造体)
 * 
 * @note コールバック関数は継承できないので双方向verもBaseからの継承にしている
 * @attention InputData,OutputDataは__attribute__((__packed__))を付けて宣言し、パディングを無効化すること
 * @attention 受信onlyの方でstatic_recv_cbを変更してもこちらとは同期されてない
 */
template <typename InputData, typename OutputData>
class ESP32Controller_Response_ESPNOW : public ResponseBase<ESP32Controller_ESPNOW<InputData>, ESP32Controller_Response_ESPNOW::Config, InputData, OutputData> {
private:
  /**
   * @brief 送信時のコールバック関数
   * @details データが相手に届いたかどうか確かめる.
   * 
   * @param info (Arduino Coreのバージョン次第ではuint8_t*にする必要あり)
   * @param flag idk
   */
#if ESP_IDF_VERSION <= ESP_IDF_VERSION_VAL(5, 0, 0)
  static void static_send_cb(const uint8_t* info ,const esp_now_send_status_t flag) {
    if (_instance == nullptr) return;
    _instance->config_.send_success.store(flag == ESP_NOW_SEND_SUCCESS);
  }
#else
  static void static_send_cb(const esp_now_send_info_t* info ,const esp_now_send_status_t flag) {
    if (_instance == nullptr) return;
    _instance->config_.send_success.store(flag == ESP_NOW_SEND_SUCCESS);
  }
#endif

  /**
   * @brief 構造体を相手に送る関数
   * @attention こいつだけvoidなのでif文に突っ込まないこと。送信できたかどうかはget_config.send_successを参照する。
   */
  void _send() {
    esp_now_send(this->config_.mac_peer, reinterpret_cast<uint8_t*>(&this->output_), sizeof(OutputData));
  }

public:
  /** 
   * @brief ESP-NOW(送受信)用設定
   */
  struct Config : public ESP32Controller_ESPNOW::Config {
    const uint8_t* mac_peer = nullptr; ///< 送信先のMACアドレス
    //volatile bool receive_new = false;
    std::atomic<bool> send_success = false;
  };

  using ResponseBase<ESP32Controller_ESPNOW<InputData>, Config, InputData, OutputData>::ResponseBase;

  /**
   * @brief setup()で呼ばれる初期化関数
   * @details 送信用にペア登録の処理が追加されている。
   * 
   * @retval true  初期化成功
   * @retval false 初期化失敗
   * @see ESP32Controller_ESPNOW::begin
   */
  bool begin() override {
    WiFi.mode(WIFI_STA);
    if (esp_now_init() != ESP_OK) return false;
      
    esp_now_peer_info_t peer_info;
    memset(&peer_info,0,sizeof(peer_info));
    memcpy(peer_info.peer_addr,this->config_.mac_peer,6);
    peer_info.channel = 0;
    peer_info.encrypt = false;
    if (esp_now_add_peer(&peer_info) != ESP_OK) return false;
    
    _instance = this;
    esp_now_register_recv_cb(static_recv_cb);
    esp_now_register_send_cb(static_send_cb);
    return true;
  }
  
  bool send() const override {
    this->_send();
    return this->config_.send_success.load();
    //return esp_now_send(this->config_.mac_peer, reinterpret_cast<uint8_t*>(&this->output_), sizeof(OutputData)) == ESP_OK;
  }
};  

} // namespace ESP32ControllerInternal
template <typename InputData>
using ESP32Controller = ESP32ControllerInternal::ESP32Controller_ESPNOW<InputData>;
template <typename InputData, typename OutputData>
using ESP32Controller_Response = ESP32ControllerInternal::ESP32Controller_Response_ESPNOW<InputData,OutputData>;

#endif
