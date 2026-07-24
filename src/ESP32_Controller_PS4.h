/**
 * @file Controller_PS4.h
 * @brief PS4コントローラー(DualShock4)からBluetoothで入力値を受け取るライブラリ
 * 
 * @author Tomoooji (https://github.com/Tomoooji)
 * @date 2026-07-23
 * @copyright Copyright (c) 2026
 * 
 * @note 結構無理くりラップしてるので他クラスとの互換性が不要ならそのまま使うことを推奨します。
 */

#ifdef ESP32
#pragma once

#include "ESP32_Controller_BaseClass.h"
#include <PS4Controller.h>

/*
struct Input_PS4{
  void apply();
};
*/

/** @brief DualShock4との通信用設定 */
struct Config_PS4{
  const char* mac;
  Config_PS4(const char* mac = nullptr):mac(mac){}
};

/**
 * @brief DualShock4から入力値を受け取るクラス
 * @details PS4Controllerライブラリのラッパー?みたいな状態
 * 
 * @tparam InputData 相手から受け取るデータ(構造体)
 */
template <typename InputData>
class Controller_PS4 :public Controller_Base<Config_PS4,InputData>{
public:
  using Controller_Base<Config_PS4,InputData>::Controller_Base;
  
  /**
   * @brief setup()で呼ばれる初期化関数
   * 
   * @retval true  初期化成功
   * @retval false 初期化失敗
   */
  bool begin() override{
    return PS4.begin(this->config.mac);
  }

  /**
   * @brief 入力値の代入用関数
   * 
   * @note input.apply()はboolの方が良いとは思うがユーザーに書かせるのもなぁ...
   * @retval true  コントローラーと接続中
   * @retval false コントローラーと接続していない
   */
  bool update() override{
    if(PS4.isConnected()){
      this->input.apply();
      return true;
    }
    return false;
  }
};
template <typename InputData>
using Controller = Controller_PS4<InputData>;

#endif
