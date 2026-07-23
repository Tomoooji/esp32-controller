/**
 * @file Controller_I2C.h
 * @brief I2Cで構造体をやりとりするライブラリ
 * 
 * @author Tomoooji
 * @date 2026-07-23
 * @copyright Copyright (c) 2026
 * 
 * @note 基本的に機体側はMasterとして運用、Slaveは一旦放置！
 */

#ifdef ESP32
#pragma once

#include "ESP32_Controller_BaseClass.h"
#include <Wire.h>

/*
struct InputData{
  //uint32_t angle;//degree
  //uint32_t dist;
  //uint32_t turn;
} __attribute__((packed));
*/

/**　@brief I2C通信用の設定　*/
struct Config_I2C_Master{
  uint8_t address_slave; //初期値は 0x2A など
  int sda = -1;
  int scl = -1;
  uint32_t frequency = 0;
};

/**
 * @brief I2C(Master)で構造体を受け取るクラス
 * 
 * @tparam InputData 相手から受け取るデータ(構造体)
 * @attention InputDataは__attribute__((__packed__))を付けて宣言し、パディングを無効化すること
 * @note 機体側は他のI2C機器との接続がありうるのでMasterとして運用
 */
template <typename InputData>
class Controller_I2C_Master : public Controller_Base<Config_I2C_Master,InputData>{
private:
public:
  using Controller_Base<Config_I2C_Master,InputData>::Controller_Base;

  /**
   * @brief setup()で呼ばれる初期化関数
   * @details MasterとしてI2Cを初期化し、結果を返す
   * 
   * @retval ture  初期化成功
   * @retval false 初期化失敗
   */
  bool begin() override{
    // マスター初期化（アドレス指定しない）
    return Wire.begin(this->config.sda, this->config.scl, this->config.frequency);
  }

  /**
   * @brief loop()内で呼ばれる値の更新を行う関数
   * @details 
   * 
   * @note もしかしたらreadByteがエラー吐くかも?
   * @retval true  更新あり
   * @retval false 更新なし
   */
  bool update() override{
    // スレーブからデータを要求
    Wire.requestFrom(this->config.address_slave, (size_t)sizeof(InputData));
    
    if (Wire.available() >= sizeof(InputData)) {
      // 受信バッファから構造体のメモリ領域へ直接バイナリとして読み込む
      Wire.readBytes(reinterpret_cast<uint8_t*>(&this->input),sizeof(InputData));
      // ↑動かなかったら↓下のを使ってね
      /*uint8_t* bytePtr = reinterpret_cast<uint8_t*>(&_currentCmd);
      for (size_t i = 0; i < sizeof(Robotinput); i++) {
          bytePtr[i] = Wire.read();
      }*/
      // 残ったゴミデータがあればすべて読み飛ばしてバッファを空にする
      while(Wire.available() > 0){
        Wire.read();
      }
      return true;
    }
    return false;
  }
};
template <typename InputData>
using Controller = Controller_I2C_Master<InputData>;

/////////////////

/**
 * @brief I2C(Master)で構造体を送受信するクラス
 * @details 
 * 
 * @tparam InputData  相手から受け取るデータ(構造体)
 * @tparam OutputData 相手に送るデータ(構造体)
 * @attention InputData,OutputDataは__attribute__((__packed__))を付けて宣言し、パディングを無効化すること
 * @note 機体側は他のI2C機器との接続がありうるのでMasterとして運用
 */
template <typename InputData, typename OutputData>
class Controller_I2C_Master_Response : public Controller_I2C_Master<InputData>{
private:
  OutputData& output;

public:
  /**
   * @brief Controller_I2C_Master_Response オブジェクトを作成
   * 
   * @param config_data 設定用構造体の参照
   * @param input_data  受け取るデータ(構造体)の参照
   * @param output_data 送るデータ(構造体)の参照
   */
  Controller_I2C_Master_Response(Config_I2C_Master& config_data, InputData& input_data, OutputData& output_data):
  Controller_I2C_Master<InputData>(config_data,input_data),output(output_data){}

  /**
   * @brief 構造体を相手に送る関数
   * 
   * @retval true  送信成功
   * @retval false 送信失敗
   */
  bool send(){
    // マスターがスレーブへデータを送信
    Wire.beginTransmission(this->config.address);
    Wire.write(reinterpret_cast<uint8_t*>(&this->output), sizeof(OutputData));
    return Wire.endTransmission() == 0;
  }
};
template <typename InputData,OutputData>
using Controller_Response = Controller_I2C_Master_Response<InputData,OutputData>;


// ============================================
// スレーブ用（外部マスターからコマンド受信）-> 基本使わない方針で
// ============================================

/*
volatile struct InputData{
  //uint32_t angle;//degree
  //uint32_t dist;
  //uint32_t turn;
} __attribute__((packed));
*/

/** @brief I2C通信用の設定(スレーブ用) */
struct Config_I2C_Slave{
  uint8_t address;
  int sda = -1;
  int scl = -1;
  uint32_t frequency = 0;
  volatile bool receive_new = false;
};

/**
 * @brief I2C(Slave)で構造体を受け取るクラス
 * 
 * @tparam InputData 相手から受け取るデータ(構造体)
 * @attention InputDataは__attribute__((__packed__))を付けて宣言し、パディングを無効化すること
 */
template <typename InputData>
class Controller_I2C_Slave : public Controller_Base<Config_I2C_Slave,InputData>{
private:
  inline static Controller_I2C_Slave *_instance = nullptr;
  
  /**
  /**
   * @brief 受信時のコールバック関数
   * @details 受け取ったデータをinputにコピーし、configの新規受信フラグを立てる
   * 
   * @param size 受け取ったデータのサイズ?
   */
  static void static_recv_cb(int size){
    if(_instance == nullptr) return;
    if(size >= sizeof(InputData)){
      Wire.readBytes(reinterpret_cast<uint8_t*>(&this->input),sizeof(InputData));
      // ↑動かなかったら↓下のを使ってね
      /*uint8_t* bytePtr = reinterpret_cast<uint8_t*>(&_currentCmd);
      for (size_t i = 0; i < sizeof(Robotinput); i++) {
          bytePtr[i] = Wire.read();
      }*/
      while(Wire.available() > 0){
        Wire.read();
      }
      _instance->config.receive_new = true;
    }
  }

public:
  using Controller_Base<Config_I2C_Slave,InputData>::Controller_Base;

  /**
   * @brief setup()で呼ばれる初期化関数
   * @details SlaveとしてI2Cを初期化、コールバック関数の登録を行って結果を返す
   * 
   * @retval ture  初期化成功
   * @retval false 初期化失敗
   */
  bool begin() override{
    // スレーブ初期化（アドレス指定）
    Wire.begin(this->config.address, this->config.sda, this->config.scl);
    _instance = this;
    Wire.onReceive(static_recv_cb);
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

//////////////////

/** @brief I2C通信用の設定(スレーブ、送受信用) */
struct Config_I2C_Slave_Response{
  uint8_t address; //初期値は 0x2A など
  int sda = -1;
  int scl = -1;
  uint32_t frequency = 0;
  volatile bool receive_new = false;
  volatile bool send_success = false;
};

/**
 * @brief I2C(Slave)で構造体を送受信するクラス
 * 
 * @tparam InputData 相手から受け取るデータ(構造体)
 * @tparam OutputData   相手に送るデータ(構造体)
 * 
 * @note コールバック関数は継承できないので双方向verもBaseからの継承にしている
 * @attention InputData,OutputDataは__attribute__((__packed__))を付けて宣言し、パディングを無効化すること
 * @attention 受信onlyの方でupdateとかstatic_recv_cbを変更してもこちらとは同期されてない
 */
template <typename InputData, typename OutputData>
class Controller_I2C_Slave_Response : public Controller_Base<Config_I2C_Slave_Response,InputData,Config_I2C_Slave_Response>{
private:
  OutputData& output;
  inline static Controller_I2C_Slave_Response *_instance = nullptr;

  /**
   * @brief 受信時のコールバック関数(流用)
   * @see Controller_I2C_Slave::static_recv_cb
   */
  static void static_recv_cb(int size){
    if(_instance == nullptr) return;
    if(size >= sizeof(InputData)){
      Wire.readBytes(reinterpret_cast<uint8_t*>(&this->input),sizeof(InputData));
      // ↑動かなかったら↓下のを使ってね
      /*uint8_t* bytePtr = reinterpret_cast<uint8_t*>(&_currentCmd);
      for (size_t i = 0; i < sizeof(Robotinput); i++) {
          bytePtr[i] = Wire.read();
      }*/
      while(Wire.available() > 0){
        Wire.read();
      }
      _instance->config.receive_new = true;
    }
  }

  /**
   * @brief リクエスト受信時のコールバック関数
   * @details Masterからのリクエストに応じてデータを送信
   */
  static void static_request_cb(){
    if(_instance == nullptr) return;
    _instance->config.send_success = Wire.write(reinterpret_cast<uint8_t*>(&_instance->output), sizeof(OutputData)) == sizeof(OutputData);
  }

public:

  /**
   * @brief Controller_I2C_Slave_Response オブジェクトを作成
   * 
   * @param config_data 設定用構造体の参照
   * @param input_data  受け取るデータ(構造体)の参照
   * @param output_data 送るデータ(構造体)の参照
   */
  Controller_I2C_Slave_Response(Config_I2C_Slave_Response& config_data, InputData& input_data, OutputData& output_data):
    Controller_Base<Config_I2C_Slave_Response,InputData>(config_data,input_data),output(output_data){}

  /**
   * @brief setup()で呼ばれる初期化関数
   * @details SlaveとしてI2Cを初期化、コールバック関数の登録を行って結果を返す
   * 
   * @retval ture  初期化成功
   * @retval false 初期化失敗
   */
  bool begin() override{
    // スレーブ初期化
    Wire.begin(this->config.address, this->config.sda, this->config.scl);
    _instance = this;
    Wire.onReceive(static_recv_cb);
    Wire.onRequest(static_request_cb);
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

#endif
