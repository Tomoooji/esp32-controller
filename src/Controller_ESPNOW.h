/**
 * @file Controller_ESPNOW.h
 * @brief ESP-NOWで構造体をやりとりするライブラリ
 * 
 * @author Tomoooji
 * @version 0.1
 * @date 2026-07-18
 * @copyright Copyright (c) 2026
 * 
 * @note 
 */
#pragma once
#include "Controller_BaseClass.h"
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
 * @brief 受信のみのクラス
 * 
 * @tparam InputData 相手から受け取るデータ(構造体)
 */
template <typename InputData>
class Controller_ESPNOW :public Controller_Base<Config_ESPNOW,InputData>{

private:

  inline static Controller_ESPNOW *_instance = nullptr;

  /**
   * @brief 受信時のコールバック関数
   * @details 受け取ったデータをcommandにコピーし、configの新規受信フラグを立てる
   * 
   * @attention commandはパック済みの構造体である必要がある
   * @param info たしか送り手のアドレスとかが入ってる
   * @param data 受け取ったデータ
   * @param len 受け取ったデータのサイズ
   */
  static void static_recv_cb(const esp_now_recv_info_t* info, const uint8_t* data, int len){
    if(_instance == nullptr || _instance->config.receive_new || sizeof(InputData) != len) return;
    memcpy(&_instance->command, data, sizeof(InputData));
    _instance->config.receive_new = true;
  }

public:

  using Controller_Base<Config_ESPNOW,InputData>::Controller_Base;

  /**
   * @brief setup()で呼ばれる初期化関数
   * @details WiFiのモード設定、ESP_NOWの初期化、コールバック関数の登録を行う
   * 
   * @retval ture 初期化成功
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
   * @retval true 更新あり
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

/////////
/* memo
  コールバック関数は継承できないので双方向verもBaseからの継承にしている
  受信onlyの方でupdateとかstatic_recv_cbを変更しても双方向verとは共通化されてないため注意
*/

/** @brief */
struct Config_ESPNOW_Response{
  const uint8_t* address_rimocon = nullptr;
  volatile bool receive_new = false;
  volatile bool send_success = false;
};

/**
 * @brief 
 * 
 * @tparam InputData 
 * @tparam OutData 
 */
template <typename InputData, typename OutData>
class Controller_ESPNOW_Response :public Controller_Base<Config_ESPNOW_Response,InputData>{

private:

  OutData& response;

  inline static Controller_ESPNOW_Response *_instance = nullptr;

  /** @brief */
  static void static_recv_cb(const esp_now_recv_info_t* info, const uint8_t* data, int len){
    if(_instance == nullptr || _instance->config.receive_new || sizeof(InputData) != len) return;
    memcpy(&_instance->command, data, sizeof(InputData));
    _instance->config.receive_new = true;
  }

  /**
   * @brief 
   * 
   * @param info 
   * @param flag 
   */
  static void static_send_cb(const esp_now_send_info_t* info ,const esp_now_send_status_t flag){
    if(_instance == nullptr) return;
    _instance->config.send_success = (flag == ESP_NOW_SEND_SUCCESS);
  }

public:

  /**
   * @brief Construct a new Controller_ESPNOW_Response object
   * 
   * @param config 
   * @param input 
   * @param output 
   */
  Controller_ESPNOW_Response(Config_ESPNOW_Response& config, InputData& input, OutData& output):
  Controller_Base<Config_ESPNOW_Response,InputData>(config,input),response(output){}
  
  /**
   * @brief 
   * 
   * @return true 
   * @return false 
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
   * @brief 
   * 
   * @return true 
   * @return false 
   */
  bool update() override{
    if(this->config.receive_new){
      this->config.receive_new = false;
      return true;
    }
    return false;
  }

  /**
   * @brief 
   * @details 
   * @attention 
   */
  void send(){
    esp_now_send(this->config.address_rimocon, reinterpret_cast<uint8_t*>(&this->response), sizeof(OutData));
  }
};


