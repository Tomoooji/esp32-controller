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

/** @brief インターフェース */
class ESP32ControllerInterface {
public:
  virtual ~ESP32ControllerInterface() = default;
  virtual bool begin() = 0;
  virtual bool update() = 0;
};

/** 
 * @brief 基底クラス
 * @tparam ConfigData 設定データの型
 * @tparam InputData 入力データの型
 */
template <typename ConfigData, typename InputData>
class ESP32ControllerBase : public ESP32ControllerInterface {
protected:
  struct ConfigStruct{};
  ConfigData config_;
  InputData input_;

  public:
  explicit ESP32ControllerBase(ConfigData &&config_data, InputData &&input_data)
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
//class ESP32ControllerDummy : public ESP32ControllerBase<ESP32ControllerDummy::ConfigDummy, InputData> {
//protected:
//public:
//  struct ConfigDummy : public ESP32ControllerBase<ConfigDummy, InputData>::ConfigStruct {};
//  using ESP32ControllerBase<ConfigDummy, InputData>::ESP32ControllerBase;
//  //ESP32ControllerDummy(ConfigDummy &&config_data, InputData &&input_data)
//  //: ESP32ControllerBase<ConfigDummy, InputData>(ConfigDummy{std::move(config_data)}, std::move(input_data)) {}
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
template <std::derived_from<ESP32ControllerInterface> Controller, typename ConfigData, typename InputData, typename OutputData>
class ESP32ControllerResponseBase : public Controller {
protected:
  OutputData output_;

public:
  explicit ESP32ControllerResponseBase(ConfigData &&config_data, InputData &&input_data, OutputData &&output_data)
      : Controller(std::move(config_data), std::move(input_data)), output_(std::move(output_data)) {}
  virtual bool send() = 0;
  virtual OutputData &output() { return this->output_; }
};

///**
// * @brief ESP32ControllerDummyを用いたダミーコントローラー
// * 
// * @tparam InputData 
// * @tparam OutputData 
// */
//template <typename InputData, typename OutputData>
//class ESP32ControllerResponseDummy : public ESP32ControllerResponseBase<ESP32ControllerDummy<InputData>, ESP32ControllerResponseDummy::ConfigResponseDummy, InputData, OutputData> {
//public:
//  struct ConfigResponseDummy : public ESP32ControllerDummy<InputData>::ConfigDummy {};
//  using ESP32ControllerResponseBase<ESP32ControllerDummy<InputData>, ConfigResponseDummy, InputData, OutputData>::ESP32ControllerResponseBase;
//  //ESP32ControllerResponseDummy(ConfigResponseDummy &&config_data, InputData &&input_data, OutputData &&output_data)
//  //    : ESP32ControllerResponseBase<ESP32ControllerDummy<InputData>, ConfigResponseDummy, InputData, OutputData>(
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

#endif
