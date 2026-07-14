#pragma once
#include <Arduino.h>

template <typename ConfigData, typename InputData>
class Controller{
private:
  ConfigData *config;
  InputData *command;
public:
  explicit Controller(ConfigData* config, InputData* input):config(config),command(input){}
  virtual bool begin() = 0;
  virtual bool update() = 0;
  const InputData& get_input(){return this->command;}
  ConfigData& get_config(){return this->config;}
};

/////////////////////////////////////////

#include <PS4Controller.h>

struct Config_PS4{
  const char* mac = nullptr;
};

template <typename InputData>
class Controller_PS4 :public Controller<Config_PS4,InputData>{
public:
  using Controller<Config_PS4,InputData>::Controller;
  bool begin() override{
    // macアドレスはconfigdataの方に書く
    return PS4.begin(this->config.mac);
  }
  bool update() override{
    if(PS4.isConnected()){

      return true;
    }
    return false;
  }
};

///////////////////////////////////////////

/*
struct InputData{
  //uint32_t angle;//degree
  //uint32_t dist;
  //uint32_t turn;
} __attribute__((packed));
*/

struct Config_Serial{
  int baudrate;
  int Rx = -1;
  int Tx = -1;
};

template <typename InputData>
class Controller_Serial : public Controller<Config_Serial,InputData>{
private:
  HardwareSerial& SER;

public:
  Controller_Serial(HardwareSerial& serial, ConfigData* config, InputData* input):
    SER(serial),config(config),command(input){}

  bool begin() override{
    this->SER.begin(this->config.baudrate, SERIAL_8N1, Rx, Tx);
    // 8ビット、パリティなし、ストップビット1（8N1）
  }

  bool update() override{
    if(this->SER.available()){
      this->SER.readBytes((uint8_t*)&this->input, sizeof(InputData));
      //this->SER.write((uint8_t*)&this->input, sizeof(InputData)); //feedback echo
      return true;
    }
    return false;
  }
};

///////////////////////////

#include <Wire.h>

/*
struct InputData{
  //uint32_t angle;//degree
  //uint32_t dist;
  //uint32_t turn;
} __attribute__((packed));
*/

struct Config_I2C{
  uint8_t adress; //初期値は 0x2A など
  int sda = -1;
  int scl = -1;
  uint32_t frequency = 0;
};

template <typename InputData>
class Controller_I2C : public Controller<Config_I2C,InputData>{
private:
public:
  using Controller<Config_I2C,InputData>::Controller;
  //memset(&_currentCmd, 0, sizeof(RobotCommand)); // 構造体をゼロクリア

  bool begin() override{
    Wire.begin(this->config.adress, this->config.sda, this->config.scl, this->config.freqency);
  }

  bool update() override{
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
};

///////////////////////////

struct Config_RemoteXY{};

class Controller_RemoteXY : public Controller<Config_RemoteXY,...>{
public:
  bool begin() override{
    RemoteXY_Init();
  }
  bool updata() override{
    RemoteXYEngine.handler();
  }
};

///////////////////////////

#include <esp_now.h>
#include <WiFi.h>

/*
struct InputData{
  //uint32_t angle;//degree
  //uint32_t dist;
  //uint32_t turn;
} __attribute__((packed));
*/

struct Config_ESPNOW{
  volatile bool recieve_new;
};

template <typename InputData>
class Controller_ESPNOW :public Controller<Config_ESPNOW,InputData>{
private:
  inline static Controller_ESPNOW *_instance = nullptr;
  static void static_recv_cb(const esp_now_recv_info_t* info, const uint8_t* data, int len){
    if(_instance->config.receive_new || sizeof(InputData) != len) return;
    memcpy(&_instance->command, data, sizeof(InputData));
    _instance->config.receive_new = true;
  }

public:
  using Controller<Config_ESPNOW,InputData>::Controller;
  bool begin() override{
    WiFi.mode(WIFI_STA);
    if(esp_now_init() != ESP_OK) return false;
    _instance = this;
    esp_now_register_recv_cb(static_recv_cb);
    return true;
  }
  bool update() override{
    if(this->config.receive_new){
      ...
    }
    this->config.receive_new = false;
  }
};

/////////

struct Config_ESPNOW_r{
  const uint8_t* address_rimocon = nullptr;
  volatile bool recieve_new;
  volatile bool send_sucess;
};


template <typename InputData, typename OutData>
class Controller_ESPNOW_r :public Controller<Config_ESPNOW_r,InputData>{
private:
  OutData *reply;
  inline static Controller_ESPNOW *_instance = nullptr;
  static void static_recv_cb(const esp_now_recv_info_t* info, const uint8_t* data, int len){
    if(_instance->config.receive_new || sizeof(InputData) != len) return;
    memcpy(&_instance->command, data, sizeof(InputData));
    _instance->config.receive_new = true;
  }
  
  static void static_send_cb(const esp_now_send_info_t* info ,const esp_now_send_status_t flag){
    _instance->config.send_success = (flag == ESP_NOW_SEND_SUCCESS);
  }

public:
  Controller_ESPNOW_r::Controller_ESPNOW_r(ConfigData* config, InputData* input, OutData* output):
    config(config),command(input),reply(output){}
  
  bool begin() override{
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
  
  bool update() override{
    if(this->config.receive_new){
      ...
    }
    this->config.receive_new = false;
  }
  
  void send(){
    esp_now_send(this->config.address_rimocon, (uint8_t*)&this->reply, sizeof(OutData));
  }

};


