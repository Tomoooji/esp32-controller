/**
 * @file ESP32Controller_Base.h
 * @brief 
 * 
 * @author Tomoooji (https://github.com/Tomoooji)
 * @version 0.1
 * @date 2026-09-22
 * @copyright Copyright (c) 2026
 */
#pragma once
#ifdef ESP32
#include <concepts>
#include <Arduino.h>

namespace ESP32ControllerInternal {

  
  /** @brief インターフェース */
  class Interface {
public:
virtual ~Interface() = default;
  virtual bool begin() = 0;
  virtual bool update() = 0;
};

/** 
 * @brief 基底クラス
 * @tparam ConfigData 設定データの型
 * @tparam InputData 入力データの型
 */
template <typename ConfigData, typename InputData>
class Base : public Interface {
protected:
  struct ConfigStruct{};
  ConfigData config_;
  InputData input_;

  public:
  explicit Base(ConfigData &&config_data, InputData &&input_data)
  : config_(std::move(config_data)), input_(std::move(input_data)) {}
  InputData &input() { return this->input_; }
  ConfigData &config() { return this->config_; }
};

///**
// * @brief ダミーコントローラー
// * 
// * @tparam InputData 
// */
//template <typename InputData>
//class ESP32ControllerDummy : public Base<ESP32ControllerDummy::ConfigDummy, InputData> {
  //protected:
  //public:
//  struct ConfigDummy : public Base<ConfigDummy, InputData>::ConfigStruct {};
//  using Base<ConfigDummy, InputData>::Base;
//  //ESP32ControllerDummy(ConfigDummy &&config_data, InputData &&input_data)
//  //: Base<ConfigDummy, InputData>(ConfigDummy{std::move(config_data)}, std::move(input_data)) {}
//  bool begin() override {
  //    return true;
  //  }
  //  bool update() override {
//    return false;
//  }
//};

/**
 * @brief 
 * 
 * @tparam Controller 
 * @tparam ConfigData 
 * @tparam InputData 
 * @tparam OutputData 
 */
template <std::derived_from<Interface> Controller, typename ConfigData, typename InputData, typename OutputData>
class ResponseBase : public Controller {
protected:
OutputData output_;

public:
  explicit ResponseBase(ConfigData &&config_data, InputData &&input_data, OutputData &&output_data)
      : Controller(std::move(config_data), std::move(input_data)), output_(std::move(output_data)) {}
  virtual bool send() const = 0;
  OutputData &output() { return this->output_; }
};

///**
// * @brief ESP32ControllerDummyを用いたダミーコントローラー
// * 
// * @tparam InputData 
// * @tparam OutputData 
// */
//template <typename InputData, typename OutputData>
//class ESP32ControllerResponseDummy : public ResponseBase<ESP32ControllerDummy<InputData>, ESP32ControllerResponseDummy::ConfigResponseDummy, InputData, OutputData> {
  //public:
//  struct ConfigResponseDummy : public ESP32ControllerDummy<InputData>::ConfigDummy {};
//  using ResponseBase<ESP32ControllerDummy<InputData>, ConfigResponseDummy, InputData, OutputData>::ResponseBase;
//  //ESP32ControllerResponseDummy(ConfigResponseDummy &&config_data, InputData &&input_data, OutputData &&output_data)
//  //    : ResponseBase<ESP32ControllerDummy<InputData>, ConfigResponseDummy, InputData, OutputData>(
//  //      std::move(config_data), std::move(input_data), std::move(output_data)) {}
//  bool begin() override {
//    return true;
//  }
//  bool update() override {
//    return false;
//  }
//  bool send() override {
  //    return;
  //  }
  //};
  
} // namespace ESP32ControllerInternal
  #endif
  