/**
 * @file Controller_ESPNOW.h
 * @brief ESP-NOWで構造体をやりとりするライブラリ
 * 
 * @author Tomoooji (https://github.com/Tomoooji)
 * @date 2026-07-26
 * @copyright Copyright (c) 2026
 * 
 * @attention C++17以降でないと動かないコードが含まれます。
 * @attention 同じInputDataを指定したクラスでインスタンスを複数作るとコールバック関数が奪われます。
 */

#pragma once

#ifdef ESP32

#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>

#include "ESP32_Controller_Base.h"

/*
volatile struct InputData {
  //uint32_t angle;//degree
  //uint32_t dist;
  //uint32_t turn;
} __attribute__((packed));
*/

/** @brief ESP-NOW(受信only)用設定 */
struct Config_ESPNOW {
  volatile bool receive_new; ///< 値の更新フラグ
  Config_ESPNOW():receive_new(false) {}
};

/**
 * @brief ESP-NOWで構造体を受け取るクラス
 * 
 * @tparam InputData 相手から受け取るデータ(構造体)
 * @attention InputDataは__attribute__((__packed__))を付けて宣言し、パディングを無効化すること
 */
template <typename InputData>
class Controller_ESPNOW :public Controller_Base<Config_ESPNOW,InputData> {

private:
  portMUX_TYPE recv_mux = portMUX_INITIALIZER_UNLOCKED;
  InputData input_buffer_; 
  inline static Controller_ESPNOW *_instance = nullptr; //!< C++17以上でないと使えない

  /**
   * @brief 受信時のコールバック関数
   * @details 受け取ったデータをinput_buffer_にコピーし、configの新規受信フラグを立てる
   * 
   * @attention inputはパック済みの構造体である必要がある
   * @param info 送り手のアドレスなどが入ってる(Arduino Coreのバージョン次第ではuint8_t*にする必要あり)
   * @param data 受け取ったデータ
   * @param len  受け取ったデータのサイズ
   * @see Controller_ESPNOW::static_recv_cb
   */
  #if ESP_IDF_VERSION <= ESP_IDF_VERSION_VAL(5, 0, 0)
  static void static_recv_cb(const uint8_t* info, const uint8_t* data, int len) {
    if (_instance == nullptr || sizeof(InputData) != len) return; // _instance->config_.receive_new || はいらないはず
    portENTER_CRITICAL(&_instance->recv_mux);
    memcpy(&_instance->input_buffer_, data, sizeof(InputData));
    _instance->config_.receive_new = true;
    portEXIT_CRITICAL(&_instance->recv_mux);
  }
  #else
  static void static_recv_cb(const esp_now_recv_info_t* info, const uint8_t* data, int len) {
    if (_instance == nullptr || sizeof(InputData) != len) return; // _instance->config_.receive_new || はいらないはず
    portENTER_CRITICAL(&_instance->recv_mux);
    memcpy(&_instance->input_buffer_, data, sizeof(InputData));
    _instance->config_.receive_new = true;
    portEXIT_CRITICAL(&_instance->recv_mux);
  }
  #endif

public:

  using Controller_Base<Config_ESPNOW,InputData>::Controller_Base;

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
    if (this->config_.receive_new) {
      
      portENTER_CRITICAL(&this->recv_mux);
      // ここに巨大な処理を入れると大変だけどそもそもESP-NOWが扱えるデータ量(250バイト)的にmemcpyしてもそんなに重たくない...はず
      memcpy(&this->input_,&this->input_buffer_,sizeof(InputData));
      this->config_.receive_new = false;
      portEXIT_CRITICAL(&this->recv_mux);
      
      return true;
    }
    return false;
  }
};
template <typename InputData>
using Controller = Controller_ESPNOW<InputData>;

/////////

/** 
 * @brief ESP-NOW(送受信)用設定
 * @code 
 *   // ~C++17
 *   Config_ESPNOW_Response config{ {0x00, 0x1A, 0x2B, 0x3C, 0x4D, 0x5E} };
 *   // C++20以降は指示付き初期化子が使える
 *   Config_ESPNOW_Response config{
 *      .mac_peer = {0x00, 0x1A, 0x2B, 0x3C, 0x4D, 0x5E};
 *   }
 * @endcode
 */
struct Config_ESPNOW_Response {
  const uint8_t* mac_peer;
  volatile bool receive_new;
  volatile bool send_success;
  Config_ESPNOW_Response(const uint8_t* mac_peer):mac_peer(mac_peer),receive_new(false),send_success(false) {}
};

/**
 * @brief ESP-NOWで構造体を送受信するクラス
 * 
 * @tparam InputData 相手から受け取るデータ(構造体)
 * @tparam OutData   相手に送るデータ(構造体)
 * 
 * @note コールバック関数は継承できないので双方向verもBaseからの継承にしている
 * @attention InputData,OutputDataは__attribute__((__packed__))を付けて宣言し、パディングを無効化すること
 * @attention 受信onlyの方でupdateとかstatic_recv_cbを変更してもこちらとは同期されてない
 */
template <typename InputData, typename OutputData>
class Controller_ESPNOW_Response :public Controller_Base<Config_ESPNOW_Response,InputData> {

private:
  OutputData& output_;
  portMUX_TYPE recv_mux = portMUX_INITIALIZER_UNLOCKED;
  InputData input_buffer_;
  inline static Controller_ESPNOW_Response *_instance = nullptr; //!< C++17以上でないと使えない

  /**
   * @brief 受信時のコールバック関数(流用)
   * @details 受け取ったデータをinput_buffer_にコピーし、configの新規受信フラグを立てる
   * 
   * @attention inputはパック済みの構造体である必要がある
   * @param info 送り手のアドレスなどが入ってる(Arduino Coreのバージョン次第ではuint8_t*にする必要あり)
   * @param data 受け取ったデータ
   * @param len  受け取ったデータのサイズ
   * @see Controller_ESPNOW::static_recv_cb
   */
  #if ESP_IDF_VERSION <= ESP_IDF_VERSION_VAL(5, 0, 0)
  static void static_recv_cb(const uint8_t* info, const uint8_t* data, int len) {
    if (_instance == nullptr || sizeof(InputData) != len) return; // _instance->config_.receive_new || はいらないはず
    portENTER_CRITICAL(&_instance->recv_mux);
    memcpy(&_instance->input_buffer_, data, sizeof(InputData));
    _instance->config_.receive_new = true;
    portEXIT_CRITICAL(&_instance->recv_mux);
  }
  #else
  static void static_recv_cb(const esp_now_recv_info_t* info, const uint8_t* data, int len) {
    if (_instance == nullptr || sizeof(InputData) != len) return; // _instance->config_.receive_new || はいらないはず
    portENTER_CRITICAL(&_instance->recv_mux);
    memcpy(&_instance->input_buffer_, data, sizeof(InputData));
    _instance->config_.receive_new = true;
    portEXIT_CRITICAL(&_instance->recv_mux);
  }
  #endif

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
    _instance->config_.send_success = (flag == ESP_NOW_SEND_SUCCESS);
  }
  #else
  static void static_send_cb(const esp_now_send_info_t* info ,const esp_now_send_status_t flag) {
    if (_instance == nullptr) return;
    _instance->config_.send_success = (flag == ESP_NOW_SEND_SUCCESS);
  }
  #endif

public:

  /**
   * @brief Controller_ESPNOW_Response オブジェクトを作成
   * 
   * @param config_data 設定用構造体の参照
   * @param input_data  受け取るデータ(構造体)の参照
   * @param output_data 送るデータ(構造体)の参照
   */
  Controller_ESPNOW_Response(Config_ESPNOW_Response& config_data, InputData& input_data, OutputData& output_data):
  Controller_Base<Config_ESPNOW_Response,InputData>(config_data,input_data),output_(output_data) {}
  
  /**
   * @brief setup()で呼ばれる初期化関数
   * @details 送信用にペア登録の処理が追加されている。
   * 
   * @retval true  初期化成功
   * @retval false 初期化失敗
   * @see Controller_ESPNOW::begin
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

  /**
   * @brief loop()内で呼ばれる値の更新(のチェック)を行う関数(流用)
   * @details コールバック関数が更新してくれたinput_buffer_からinput_にコピーし、フラグを倒す。
   * 
   * @retval true  更新あり
   * @retval false 更新なし
   * @note コピーしてる間はCritical Sectionでコールバック関数を止めている。
   * @see Controller_ESPNOW::update
   */
  bool update() override {
    if (this->config_.receive_new) {
      
      portENTER_CRITICAL(&this->recv_mux);
      // ここに巨大な処理を入れると大変だけどそもそもESP-NOWが扱えるデータ量(250バイト)的にmemcpyしてもそんなに重たくない...はず
      memcpy(&this->input_,&this->input_buffer_,sizeof(InputData));
      this->config_.receive_new = false;
      portEXIT_CRITICAL(&this->recv_mux);
      
      return true;
    }
    return false;
  }

  /**
   * @brief 構造体を相手に送る関数
   * @attention こいつだけvoidなのでif文に突っ込まないこと。送信できたかどうかはget_config.send_successを参照する。
   */
  void send() {
    esp_now_send(this->config_.mac_peer, reinterpret_cast<uint8_t*>(&this->output_), sizeof(OutputData));
  }

  /**
   * @brief output オブジェクトを設定
   * 
   * @param new_output 新しく設定するoutputオブジェクトの参照
   * @retval OutputData& 設定したoutputオブジェクトへの参照
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
  OutputData& set_output(OutputData& new_output) {
    this->output_ = new_output;
    return this->output_;
  }

};
template <typename InputData, typename OutputData>
using Controller_Response = Controller_ESPNOW_Response<InputData,OutputData>;

#endif
