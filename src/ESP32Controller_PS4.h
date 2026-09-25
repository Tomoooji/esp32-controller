/**
 * @file ESP32Controller_PS4.h
 * @brief PS4コントローラー(DualShock4)からBluetoothで入力値を受け取るライブラリ
 * 
 * @attention C++20以上が必要です。
 * @attention 入力用構造体には関数apply()を定義する必要があります。
 * @note 結構無理くりラップしてるので他クラスとの互換性が不要ならそのまま使うことを推奨します。
 * 
 * @author Tomoooji (https://github.com/Tomoooji)
 * @version 2.0.0
 * @date 2026-09-22
 * @copyright Copyright (c) 2026
 */

#pragma once
#ifdef ESP32
#include <PS4Controller.h>
#include "ESP32Controller_Base.h"

namespace ESP32ControllerInternal{

  /**
 * @brief DualShock4から入力値を受け取るクラス
 * @details PS4Controllerライブラリのラッパー?みたいな状態
 * 
 * @tparam InputData 相手から受け取るデータ(構造体)
 */
template <typename InputData> requires requires(InputData input) { {input.apply()}; }
class ESP32Controller_PS4 : public Base<ESP32Controller_PS4::Config, InputData> {
protected:
public:
  /** 
   * @brief DualShock4との通信用設定
   * @details MACアドレスを指定して初期化する。
   * @note MACアドレスなしで初期化するとESP32のMACアドレスが使われる。
   */
  struct Config : public Base<Config, InputData>::ConfigStruct {
    const char* mac = nullptr;
  };
  
  using Base<Config, InputData>::Base;
  
  /**
   * @brief setup()で呼ばれる初期化関数
   * 
   * @retval true  初期化成功
   * @retval false 初期化失敗
   */
  bool begin() override {
    return PS4.begin(this->config_.mac);
  }
  
  /**
   * @brief 入力値の代入用関数
   * 
   * @note input.apply()はboolの方が良いとは思うがユーザーに書かせるのもなぁ...
   * @retval true  コントローラーと接続中
   * @retval false コントローラーと接続していない
   */
  bool update() override {
    if (PS4.isConnected()) {
      this->input_.apply();
      return true;
    }
    return false;
  }
};


} // namespace ESP32ControllerInternal

template <typename InputData>
using ESP32Controller = ESP32ControllerInternal::ESP32Controller_PS4<InputData>;

#endif
