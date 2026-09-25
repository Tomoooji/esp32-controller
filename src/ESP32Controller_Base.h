/**
 * @file ESP32Controller_Base.h
 * @brief ESP32Controllerの基底クラス群
 *
 * @attension 基底クラス群はESP32Controllerの各通信方式のクラスで継承されることを想定しているため、直接使うことは想定していません。
 * @attention C++20で追加されたstd::derived_fromを使っているため,C++20以降でないとコンパイルできない。
 * 
 * @author Tomoooji (https://github.com/Tomoooji)
 * @version 2.0.0
 * @date 2026-09-22
 * @copyright Copyright (c) 2026
 */

#pragma once
#ifdef ESP32
#include <Arduino.h>
#include <concepts>

namespace ESP32ControllerInternal {

/** @brief インターフェース */
class Interface {
public:
  virtual ~Interface() = default;
  virtual bool begin() = 0; ///< setup()で呼ばれる初期化関数
  virtual bool update() = 0; ///< loop()内で呼ばれる値の更新を行う関数
};

/**
 * @brief 受信用基底クラス
 * @details 受信用の基底クラス。ConfigDataとInputDataをテンプレートで受け取り、継承先でそれぞれの型を指定することで、任意の設定データと入力データを扱える。
 * @tparam ConfigData 設定データの型
 * @tparam InputData 入力データの型
 * @note InputDataは__attribute__((__packed__))を付けて宣言し、パディングを無効化することを推奨します。
 */
template <typename ConfigData, typename InputData>
class Base : public Interface {
protected:
  struct ConfigStruct {}; ///< 設定データの型を継承先で定義するための空構造体
  ConfigData config_; ///< 設定データの実体
  InputData input_;   ///< 入力データの実体

public:
  /**
   * @brief Base オブジェクトを作成
   * @details ConfigDataとInputDataの実体を受け取り、config_とinput_にコピーする。
   * @param config_data 設定用構造体の右辺値参照(その場で作った構造体を渡すことを想定)
   * @param input_data 入力用構造体の右辺値参照(その場で作った構造体を渡すことを想定)
   */
  explicit Base(ConfigData &&config_data, InputData /*&&*/input_data)
      : config_(std::move(config_data)), input_(std::move(input_data)) {}

  ConfigData &config() { return this->config_; } ///< 設定データの実体への参照を返す
  InputData &input() { return this->input_; }    ///< 入力データの実体への参照を返す
};

/**
 * @brief 送受信用基底クラス
 * @details 受信用基底クラスをテンプレートでとって継承することで、任意の受信クラスに対して送信用の機能を追加できる。
 * @tparam Controller 受信用基底クラス(正確にはInterfaceを継承しているクラス)
 * @tparam ConfigData 設定データの型
 * @tparam InputData 入力データの型
 * @tparam OutputData 出力データの型
 * @note OutputDataは__attribute__((__packed__))を付けて宣言し、パディングを無効化することを推奨します。
 */
template <std::derived_from<Interface> Controller, typename ConfigData, typename InputData, typename OutputData>
class ResponseBase : public Controller {
protected:
  OutputData output_; ///< 送信データの実体(非同期に更新される場合は別途bufferを用意する必要がある)

public:
  explicit ResponseBase(ConfigData &&config_data, InputData /*&&*/input_data, OutputData /*&&*/output_data)
      : Controller(std::move(config_data), std::move(input_data)), output_(std::move(output_data)) {}
  virtual bool send() const = 0; ///< 送信用関数(非同期に送信する場合はCriticalSectionを使ってbufferにOutputDataの実体をコピーして送信する必要がある)
  OutputData &output() { return this->output_; } ///< 送信データの実体への参照を返す
};

} // namespace ESP32ControllerInternal
#endif
