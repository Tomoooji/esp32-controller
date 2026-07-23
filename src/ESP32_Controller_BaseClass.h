/**
 * @file Controller_Base.h
 * @brief 各ライブラリの抽象基底クラスのヘッダ
 * 
 * @author Tomoooji
 * @date 2026-07-23
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
 * @tparam InputData  入力されるデータの格納用
 * @attention このクラスは継承しないと実体化できない抽象基底クラスです。必ず継承先でbool begin()とbool update()を実装してください。
 */
template <typename ConfigData, typename InputData>
class Controller_Base{

protected:
  ConfigData& config;
  InputData& input;

public:

  /**
   * @brief Controller_Base オブジェクトを作成
   * 
   * @param config_data 設定用構造体の参照
   * @param input_data  受け取るデータ(構造体)の参照
   */
  explicit Controller_Base(ConfigData& config_data, InputData& input_data):config(config_data),input(input_data){}

  /**
   * @brief 初期化用の純粋仮想関数
   * 
   * @retval true  初期化成功
   * @retval false 初期化失敗
   * @note setup内で呼ぶ。1行if文とかでreturnすると良い
   */
  virtual bool begin() = 0;
  
  /**
   * @brief 入力更新用の純粋仮想関数
   * 
   * @retval true  更新有り
   * @retval false 更新なし
   * @note loop内で呼ぶ。if文に突っ込んでロボットの動作を全部その中に入れると暴走対策になる
   */
  virtual bool update() = 0;

  /**
   * @brief inputオブジェクトのゲッター関数
   * 
   * @return const InputData& 入力データの構造体への参照
   * @note Controller.get_input().XXで値を参照できる。代入は不可
   */
  const InputData& get_input(){return this->input;}
  
  /**
   * @brief configオブジェクトのゲッター関数
   * 
   * @return ConfigData& 設定データの構造体への参照
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
