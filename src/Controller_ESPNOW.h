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

struct Config_ESPNOW{
  volatile bool receive_new = false;
};

template <typename InputData>
class Controller_ESPNOW :public Controller_Base<Config_ESPNOW,InputData>{
private:
  inline static Controller_ESPNOW *_instance = nullptr;

  static void static_recv_cb(const esp_now_recv_info_t* info, const uint8_t* data, int len){
    if(_instance == nullptr || _instance->config.receive_new || sizeof(InputData) != len) return;
    memcpy(&_instance->command, data, sizeof(InputData));
    _instance->config.receive_new = true;
  }

public:
  using Controller_Base<Config_ESPNOW,InputData>::Controller_Base;

  bool begin() override{
    WiFi.mode(WIFI_STA);
    if(esp_now_init() != ESP_OK) return false;

    _instance = this;
    esp_now_register_recv_cb(static_recv_cb);
    return true;
  }

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

struct Config_ESPNOW_Response{
  const uint8_t* address_rimocon = nullptr;
  volatile bool receive_new = false;
  volatile bool send_success = false;
};

template <typename InputData, typename OutData>
class Controller_ESPNOW_Response :public Controller_Base<Config_ESPNOW_Response,InputData>{
private:
  OutData& response;
  inline static Controller_ESPNOW_Response *_instance = nullptr;

  static void static_recv_cb(const esp_now_recv_info_t* info, const uint8_t* data, int len){
    if(_instance == nullptr || _instance->config.receive_new || sizeof(InputData) != len) return;
    memcpy(&_instance->command, data, sizeof(InputData));
    _instance->config.receive_new = true;
  }

  static void static_send_cb(const esp_now_send_info_t* info ,const esp_now_send_status_t flag){
    if(_instance == nullptr) return;
    _instance->config.send_success = (flag == ESP_NOW_SEND_SUCCESS);
  }

public:
  Controller_ESPNOW_Response(Config_ESPNOW_Response& config, InputData& input, OutData& output):
  Controller_Base<Config_ESPNOW_Response,InputData>(config,input),response(output){}
  
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

  bool update() override{
    if(this->config.receive_new){
      this->config.receive_new = false;
      return true;
    }
    return false;
  }

  void send(){
    esp_now_send(this->config.address_rimocon, reinterpret_cast<uint8_t*>(&this->response), sizeof(OutData));
  }
};


