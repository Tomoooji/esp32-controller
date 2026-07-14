#include "ESP32_Controller.h"

/*

Controller_PS4::begin() override{}
Controller_PS4::update() override{}

Controller_Serial::begin() override{}
Controller_Serial::update() override{}

Controller_I2C::begin() override{}
Controller_I2C::update() override{}


Controller_ESPNOW::begin() override{}
Controller_ESPNOW::update() override{}


Controller_ESPNOW_r::Controller_ESPNOW_r(){}

bool Controller_ESPNOW_r::begin() override{
  WiFi.mode(WIFI_STA);
  if(esp_now_init() != ESP_OK) return false;
    
  esp_now_peer_info_t peer_info;
  memset(&this->peer_info,0,sizeof(this->peer_info));
  memcpy(this->peer_info.peer_addr,this->config.address_rimocon,6);
  this->peer_info.channel = 0;
  this->peer_info.encrypt = false;
  if(esp_now_add_peer(&this->peer_info) != ESP_OK) return false;
  
  _instance = this;
  esp_now_register_send_cb(static_send_cb);
  esp_now_register_recv_cb(static_recv_cb);
  return true;
}

bool Controller_ESPNOW_r::update() override{
    if(this->config.receive_new){
      ...
    }
    this->config.receive_new = false;
}

void Controller_ESPNOW_r::send(){
  esp_now_send(this->config.address_rimocon, (uint8_t*)&this->reply, sizeof(OutData));
}

*/
