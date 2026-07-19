/**
 * @file Controller_Base.h
 * @brief 各ライブラリの抽象基底クラスのヘッダ
 * 
 * @author Tomoooji
 * @version 0.1
 * @date 2026-07-18
 * @copyright Copyright (c) 2026
 * 
 * @note 
 */
#ifdef ESP32
#pragma once

#include <Arduino.h>

/**
 * @brief 抽象基底クラス
 * 
 * @tparam ConfigData 設定値とかフラグの格納用
 * @tparam InputData 入力されるデータの格納用
 */
template <typename ConfigData, typename InputData>
class Controller_Base{

protected:
  ConfigData& config;
  InputData& command;

public:

  /**
   * @brief Construct a new Controller_Base object
   * 
   * @param config 参照でもらう
   * @param input 同じく
   */
  explicit Controller_Base(ConfigData& config, InputData& input):config(config),command(input){}

  /**
   * @brief 初期化用関数
   * 
   * @return true 初期化成功
   * @return false 初期化失敗
   * 
   * @note setup内で呼ぶ。1行if文とかでreturnすると良い
   */
  virtual bool begin() = 0;
  
  /**
   * @brief 入力更新関数
   * 
   * @return true 更新有り
   * @return false 更新なし
   * 
   * @note loop内で呼ぶ。if文に突っ込んでロボットの動作を全部その中に入れると暴走対策になる
   */
  virtual bool update() = 0;

  /**
   * @brief Get the input object
   * 
   * @return const InputData& 入力データの構造体への参照
   * 
   * @note Controller.get_input().XXで値を参照できる。代入とかは無理
   */
  const InputData& get_input(){return this->command;}
  
  /**
   * @brief Get the config object
   * 
   * @return ConfigData& 設定データの構造体への参照
   * 
   * @note Controller.get_config().XXで値の参照,更新ができる。
   */
  ConfigData& get_config(){return this->config;}
};


///////////////////////////
/*
struct Config_RemoteXY{};

class Controller_RemoteXY : public Controller_Base<Config_RemoteXY,...>{
public:
  bool begin() override{
    RemoteXY_Init();
  }
  bool updata() override{
    RemoteXYEngine.handler();
  }
};
*/
///////////////////////////

#endif