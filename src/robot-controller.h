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

///////////////////////////

#include <Wire.h>

struct InputData{
    //uint32_t angle;//degree
    //uint32_t dist;
    //uint32_t turn;
} __attribute__((packed));

struct ConfigData config{
  uint8_t adress;
  int sda,scl;
};

class Controller_I2C : public Controller{
private:
  //uint8_t _address;
  //bool _isNewDataAvailable;
public:
  // コンストラクタ（ESP32のI2Cスレーブアドレスを指定、初期値は 0x2A など）
  //memset(&_currentCmd, 0, sizeof(RobotCommand)); // 構造体をゼロクリア
  using Controller::Controller;

  // 初期化処理（setup内で呼ぶ）
  bool begin() override{
    Wire.begin(this->config.adress, this->config.sda, this->config.scl, 100000); // 100kHzで起動
  }

  // ポーリング用関数：データが来ていたら構造体に吸い上げる（loop内で毎ターン呼ぶ）
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
      //_isNewDataAvailable = true; // 新着フラグを立てる
    }
    retirn false;
  }
  /*
    // 新しいデータが来ているか確認し、フラグを戻す
    bool hasNewData() {
        if (_isNewDataAvailable) {
            _isNewDataAvailable = false;
            return true;
        }
        return false;
    }

    // 格納された構造体データを取得する
    const RobotCommand& getCommand() const {
        return _currentCmd;
    }
  */
};

///////////////////////////

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