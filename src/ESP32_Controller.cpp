#include "ESP32_Controller.h"

//////////////////////////////

bool Controller_PS4::begin(){
  return PS4.begin(this->config.mac);
}

bool Controller_PS4::update(){
  if(PS4.isConnected()){
    this->command.apply();
    return true;
  }
  return false;
}

//////////////////////////////

template <typename InputData>
Controller_Serial::Controller_Serial(HardwareSerial& serial, ConfigData* config, InputData* input):
    SER(serial),config(config),command(input){}

bool Controller_Serial::begin(){
  this->SER.begin(this->config.baudrate, SERIAL_8N1, Rx, Tx);
  // 8ビット、パリティなし、ストップビット1（8N1）
}

bool Controller_Serial::update(){
  if(this->SER.available()){
    this->SER.readBytes((uint8_t*)&this->input, sizeof(InputData));
    //this->SER.write((uint8_t*)&this->input, sizeof(InputData)); //feedback echo
    return true;
  }
  return false;
}

//////////////////////////////

bool Controller_I2C::begin(){
  Wire.begin(this->config.adress, this->config.sda, this->config.scl, this->config.freqency);
}

bool Controller_I2C::update(){
  int availableBytes = Wire.available();
  // 構造体のサイズ以上のデータがバッファに届いているかチェック
  if (availableBytes >= sizeof(InputData)) {
    // 受信バッファから構造体のメモリ領域へ直接バイナリとして読み込む
    uint8_t* bytePtr = reinterpret_cast<uint8_t*>(&command);
    for (size_t i = 0; i < sizeof(InputData); i++) {
      bytePtr[i] = Wire.read();
    }
    
    // 残ったゴミデータがあればすべて読み飛ばしてバッファを空にする
    while (Wire.available() > 0) {
      Wire.read();
    }
    retirn true;
  }
  retirn false;
}

//////////////////////////////

bool Controller_ESPNOW::begin(){
  WiFi.mode(WIFI_STA);
  if(esp_now_init() != ESP_OK) return false;
  _instance = this;
  esp_now_register_recv_cb(static_recv_cb);
  return true;
}

bool Controller_ESPNOW::update(){
  if(this->config.receive_new){
    this->config.receive_new = false;
    return true;
  }
  return false;
}

///////////////

template <typename InputData, typename OutData>
Controller_ESPNOW_r::Controller_ESPNOW_r(ConfigData* config, InputData* input, OutData* output):
    config(config),command(input),reply(output){}

bool Controller_ESPNOW_r::begin(){
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

bool Controller_ESPNOW_r::update(){
  if(this->config.receive_new){
    this->config.receive_new = false;
    return true;
  }
  return false;
}

void Controller_ESPNOW_r::send(){
  esp_now_send(this->config.address_rimocon, (uint8_t*)&this->reply, sizeof(OutData));
}

