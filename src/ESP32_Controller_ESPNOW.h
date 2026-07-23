/**
 * @file Controller_ESPNOW.h
 * @brief ESP-NOWで構造体をやりとりするライブラリ
 * 
 * @author Tomoooji
 * @date 2026-07-23
 * @copyright Copyright (c) 2026
 * 
 * @note コールバック関数のinfoはesp_now_recv_info_t* あるいは esp_now_send_info_t* に更新する必要があるかも
 */

#ifdef ESP32
#pragma once

#include "ESP32_Controller_BaseClass.h"
#include <esp_now.h>
#include <WiFi.h>

/*
volatile struct InputData{
  //uint32_t angle;//degree
  //uint32_t dist;
  //uint32_t turn;
} __attribute__((packed));
*/

/**　@brief ESP-NOW(受信only)用設定 */
struct Config_ESPNOW{
  volatile bool receive_new = false; ///< 値の更新フラグ
};

/**
 * @brief ESP-NOWで構造体を受け取るクラス
 * 
 * @tparam InputData 相手から受け取るデータ(構造体)
 * @attention InputDataは__attribute__((__packed__))を付けて宣言し、パディングを無効化すること
 */
template <typename InputData>
class Controller_ESPNOW :public Controller_Base<Config_ESPNOW,InputData>{

private:

  inline static Controller_ESPNOW *_instance = nullptr;

  /**
   * @brief 受信時のコールバック関数
   * @details 受け取ったデータをinputにコピーし、configの新規受信フラグを立てる
   * 
   * @attention inputはパック済みの構造体である必要がある
   * @param info たしか送り手のアドレスとかが入ってる
   * @param data 受け取ったデータ
   * @param len  受け取ったデータのサイズ
   */
  static void static_recv_cb(const uint8_t* info, const uint8_t* data, int len){
    if(_instance == nullptr || _instance->config.receive_new || sizeof(InputData) != len) return;
    memcpy(&_instance->input, data, sizeof(InputData));
    _instance->config.receive_new = true;
  }

public:

  using Controller_Base<Config_ESPNOW,InputData>::Controller_Base;

  /**
   * @brief setup()で呼ばれる初期化関数
   * @details WiFiのモード設定、ESP_NOWの初期化、コールバック関数の登録を行う
   * 
   * @retval ture  初期化成功
   * @retval false 初期化失敗
   */
  bool begin() override{
    // WiFiモード設定 stationの略?
    WiFi.mode(WIFI_STA);
    if(esp_now_init() != ESP_OK) return false;
    // コールバック関数登録 static関数なので複数インスタンス作るとバグる
    _instance = this;
    esp_now_register_recv_cb(static_recv_cb);

    return true;
  }

  /**
   * @brief loop()内で呼ばれる値の更新(のチェック)を行う関数
   * @details 値の更新自体はコールバック関数がやってくれるのでフラグ管理のみ
   * 
   * @retval true  更新あり
   * @retval false 更新なし
   */
  bool update() override{
    if(this->config.receive_new){
      this->config.receive_new = false;
      return true;
    }
    return false;
  }
};
template <typename InputData>
using Controller = Controller_ESPNOW<InputData>;

/////////

/** @brief ESP-NOW(送受信)用設定 */
struct Config_ESPNOW_Response{
  const uint8_t* address_rimocon = nullptr;
  volatile bool receive_new = false;
  volatile bool send_success = false;
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
template <typename InputData, typename OutData>
class Controller_ESPNOW_Response :public Controller_Base<Config_ESPNOW_Response,InputData>{

private:

  OutData& output;

  inline static Controller_ESPNOW_Response *_instance = nullptr;

  /**
   * @brief 受信時のコールバック関数(流用)
   * @see Controller_ESPNOW::static_recv_cb
   */
  static void static_recv_cb(const uint8_t* info, const uint8_t* data, int len){
    if(_instance == nullptr || _instance->config.receive_new || sizeof(InputData) != len) return;
    memcpy(&_instance->input, data, sizeof(InputData));
    _instance->config.receive_new = true;
  }

  /**
   * @brief 送信時のコールバック関数
   * @details データが相手に届いたかどうか確かめる.
   * 
   * @param info idk
   * @param flag idk
   */
  static void static_send_cb(const uint8_t* info ,const esp_now_send_status_t flag){
    if(_instance == nullptr) return;
    _instance->config.send_success = (flag == ESP_NOW_SEND_SUCCESS);
  }

public:

  /**
   * @brief Controller_ESPNOW_Response オブジェクトを作成
   * 
   * @param config_data 設定用構造体の参照
   * @param input_data  受け取るデータ(構造体)の参照
   * @param output_data 送るデータ(構造体)の参照
   */
  Controller_ESPNOW_Response(Config_ESPNOW_Response& config_data, InputData& input_data, OutData& output_data):
  Controller_Base<Config_ESPNOW_Response,InputData>(config_data,input_data),output(output_data){}
  
  /**
   * @brief setup()で呼ばれる初期化関数
   * @details 送信用にペア登録の処理が追加されている。
   * 
   * @see Controller_ESPNOW::begin
   * @retval true  初期化成功
   * @retval false 初期化失敗
   */
  bool begin() override{
    WiFi.mode(WIFI_STA);
    if(esp_now_init() != ESP_OK) return false;
      
    esp_now_peer_info_t peer_info;
    memset(&peer_info,0,sizeof(peer_info));
    memcpy(peer_info.peer_addr,this->config.address_rimocon,6);
    peer_info.channel = 0;
    peer_info.encrypt = false;
    if(esp_now_add_peer(&peer_info) != ESP_OK) return false;
    
    _instance = this;
    esp_now_register_recv_cb(static_recv_cb);
    esp_now_register_send_cb(static_send_cb);
    return true;
  }

  /**
   * @brief loop()内で呼ばれる値の更新(のチェック)を行う関数
   * @details 値の更新自体はコールバック関数がやってくれるのでフラグ管理のみ
   * 
   * @see Controller_ESPNOW::update
   * @retval true  更新あり
   * @retval false 更新なし
   */
  bool update() override{
    if(this->config.receive_new){
      this->config.receive_new = false;
      return true;
    }
    return false;
  }

  /**
   * @brief 構造体を相手に送る関数
   * @attention こいつだけvoidなのでif文に突っ込まないこと。送信できたかどうかはget_config.send_successを参照する。
   */
  void send(){
    esp_now_send(this->config.address_rimocon, reinterpret_cast<uint8_t*>(&this->output), sizeof(OutData));
  }
};
template <typename InputData, typename OutputData>
using Controller_Response = Controller_ESPNOW_Response<InputData,OutputData>;

#endif
