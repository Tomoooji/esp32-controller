/**
 * @file ESP32Controller_PS4.h
 * @brief PS4コントローラー(DualShock4)からBluetoothで入力値を受け取るライブラリ
 * 
 * @author Tomoooji (https://github.com/Tomoooji)
 * @date 2026-08-27
 * @copyright Copyright (c) 2026
 * 
 * @attention 入力用構造体にはvoid apply()関数を定義する必要があります。
 * @note 結構無理くりラップしてるので他クラスとの互換性が不要ならそのまま使うことを推奨します。
 */

#pragma once
#ifdef ESP32
#include <PS4Controller.h>
#include "ESP32Controller_Base.h"

/**
 * @brief DualShock4から入力値を受け取るクラス
 * @details PS4Controllerライブラリのラッパー?みたいな状態
 * 
 * @tparam InputData 相手から受け取るデータ(構造体)
 */
template <typename InputData>
class ESP32Controller_PS4 : public ESP32ControllerBase<ESP32Controller_PS4::Config_PS4, InputData> {
protected:
public:
  /** 
   * @brief DualShock4との通信用設定
   * 
   * @code 
   *  // ~C++17
   *   Config_PS4 config{"00:1A:2B:3C:4D:5E"};
   *  // C++20からは指示付き初期化子が使える
   *   Config_PS4 config{.mac = "00:1A:2B:3C:4D:5E"}
   * @endcode
   * @note MACアドレスなしで初期化するとESP32のMACアドレスが使われる。
   */
  struct Config_PS4 : public ESP32ControllerBase<Config_PS4, InputData>::ConfigStruct {
    const char* mac = nullptr;
  };

  using ESP32ControllerBase<Config_PS4, InputData>::ESP32ControllerBase;

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

template <typename InputData>
using ESP32Controller = ESP32Controller_PS4<InputData>;

#endif
