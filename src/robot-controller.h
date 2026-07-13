#pragma once
#include <Arduino.h>

struct InputData;
struct ConfigData;

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

class Controller_PS4 :public Controller{
public:
  using Controller::Controller;
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

struct InputData{
    //uint32_t angle;//degree
    //uint32_t dist;
    //uint32_t turn;
} __attribute__((packed));

struct ConfigData config{
  int baudrate;
};

class Controller_Serial : public Controller{
private:
  HardwareSerial& SER;

public:
  Controller_Serial(HardwareSerial& serial, ConfigData* config, InputData* input):SER(serial),config(config),command(input){}

  bool begin() override{
    this->SER.begin(this->config.baudrate);
  }
  
  bool begin(const uint8_t Rx, const uint8_t Tx){
    this->SER.begin(this->config.baudrate, SERIAL_8N1, Rx, Tx);
    // 8ビット、パリティなし、ストップビット1（8N1）
  }

  bool update() override{
    if(this->SER.available()){
      this->SER.readBytes((uint8_t*)&this->input, sizeof(InputData));
      //this->SER.write((uint8_t*)&this->input, sizeof(InputData)); //feedback echo
      return true;
    }
    else{
      return false;
    }
  }
};

/*
#include <esp_now.h>
#include <WiFi.h>

struct ConfigData{
  esp_now_peer_info_t _peer_info;
  const uint8_t* receiver_address;
  //inline static ESPnowRimocon *_instance = nullptr;

};


class Controller_ESPNOW :public Controller{
private:
public:
  using Controller::Controller;
  bool begin(){
  }
  bool update(){
  }
};

*/