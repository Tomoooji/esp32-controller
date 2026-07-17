#pragma once
#include "Controller_BaseClass.h"
#include <Wire.h>

/*
struct InputData{
  //uint32_t angle;//degree
  //uint32_t dist;
  //uint32_t turn;
} __attribute__((packed));
*/

struct Config_I2C{
  uint8_t address; //初期値は 0x2A など
  int sda = -1;
  int scl = -1;
  uint32_t frequency = 0;
};

template <typename InputData>
class Controller_I2C : public Controller_Base<Config_I2C,InputData>{
private:
public:
  using Controller_Base<Config_I2C,InputData>::Controller_Base;
  //memset(&_currentCmd, 0, sizeof(RobotCommand)); // 構造体をゼロクリア

  bool begin() override{
    return Wire.begin(this->config.address, this->config.sda, this->config.scl, this->config.frequency);
  }

  bool update() override{
    int availableBytes = Wire.available();
    // 構造体のサイズ以上のデータがバッファに届いているかチェック
    if (availableBytes >= sizeof(InputData)) {
      // 受信バッファから構造体のメモリ領域へ直接バイナリとして読み込む
      uint8_t* bytePtr = reinterpret_cast<uint8_t*>(&this->command);
      for (size_t i = 0; i < sizeof(InputData); i++) {
        bytePtr[i] = Wire.read();
      }
      
      // 残ったゴミデータがあればすべて読み飛ばしてバッファを空にする
      while (Wire.available() > 0) {
        Wire.read();
      }
      return true;
    }
    return false;
  }
};

/////////////////

template <typename InputData, typename OutputData>
class Controller_I2C_Response : public Controller_Base<Config_I2C,InputData>{
private:
  OutputData& response;
public:
  Controller_I2C_Response(Config_Serial& config, InputData& input, OutputData& output):
    config(config),command(input),response{output}{}
  //memset(&_currentCmd, 0, sizeof(RobotCommand)); // 構造体をゼロクリア

  bool begin() override{
    return Wire.begin(this->config.sda, this->config.scl, this->config.frequency);
  }

  bool update() override{
    // 構造体のサイズ以上のデータがバッファに届いているかチェック
    if (Wire.available() >= sizeof(InputData)) {
      // 受信バッファから構造体のメモリ領域へ直接バイナリとして読み込む
      Wire.readBytes(reinterpret_cast<uint8_t*>(&this->command), sizeof(InputData));

      // 残ったゴミデータがあればすべて読み飛ばしてバッファを空にする
      while (Wire.available() > 0) {
        Wire.read();
      }
      return true;
    }
    return false;
  }

  bool send(){
    // 送信処理の開始
    Wire.beginTransmission(this->config.address);
    // 構造体のメモリ領域をバイト配列（uint8_t*）にキャストして、丸ごと16バイト送信
    Wire.write(reinterpret_cast<uint8_t*>(&response), sizeof(OutputData));
    return Wire.endTransmisson() == 0;
  }
};



/*
// master_sender.ino
#include <Wire.h>
#include "DataStructure.h"

// 受信側（スレーブ）と同じアドレスを指定
const uint8_t SLAVE_ADDRESS = 0x2A; 

void setup() {
    Serial.begin(115200);
    
    // マスタとしてI2Cを起動（アドレスは指定しない）
    // 標準ピン（SDA:21, SCL:22）、通信速度100kHz
    Wire.begin(21, 22, 100000); 
    
    Serial.println("I2C Master Sender Ready...");
}

void loop() {
    // 1. 送信用テストデータの作成
    static int16_t speedCounter = 0;
    RobotCommand testCmd;
    
    // 構造体をゼロクリアしてから値をセット
    memset(&testCmd, 0, sizeof(RobotCommand)); 
    
    // 毎回値が変わるように、カウンターを元にデータを作成
    testCmd.motorLeft = speedCounter;
    testCmd.motorRight = -speedCounter; // 逆転
    testCmd.ledPattern = (speedCounter % 5);
    testCmd.weaponCmd = 0xAA; // 固定値

    // 2. I2C送信処理
    Serial.print("Sending Data... Left: ");
    Serial.print(testCmd.motorLeft);
    Serial.print(", Right: ");
    Serial.println(testCmd.motorRight);

    // 送信処理の開始
    Wire.beginTransmission(SLAVE_ADDRESS);
    
    // 構造体のメモリ領域をバイト配列（uint8_t*）にキャストして、丸ごと16バイト送信
    Wire.write(reinterpret_cast<uint8_t*>(&testCmd), sizeof(RobotCommand));
    
    // 送信を確定して終了（戻り値 0 = 成功）
    byte error = Wire.endTransmission();
    
    if (error != 0) {
        Serial.print("Transmission failed. Error code: ");
        Serial.println(error);
    }

    // 3. テスト用のカウンターを更新（-500 〜 500の間でループ）
    speedCounter += 50;
    if (speedCounter > 500) {
        speedCounter = -500;
    }

    // 1秒待機
    delay(1000);
}

*/