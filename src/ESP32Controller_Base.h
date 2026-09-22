#pragma once
#ifdef ESP32
#include <Arduino.h>

class ESP32ControllerInterface {
public:
  virtual ~ESP32ControllerInterface() = default;
  virtual bool begin() = 0;
  virtual bool update() = 0;
};

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

template <std::derived_from<ESP32ControllerInterface> Controller, typename ConfigData, typename InputData, typename OutputData>
class ESP32ControllerResponseBase : public Controller {
protected:
  OutputData output_;

public:
  explicit ESP32ControllerResponseBase(ConfigData &&config_data, InputData &&input_data, OutputData &&output_data)
      : Controller(std::move(config_data), std::move(input_data)), output_(std::move(output_data)) {}
  virtual void _send() = 0;
  OutputData &output() { return this->output_; }
};

template <typename InputData>
class ESP32ControllerDummy : public ESP32ControllerBase<ESP32ControllerDummy::ConfigDummy, InputData> {
private:
public:
  struct ConfigDummy : public ESP32ControllerBase<ConfigDummy, InputData>::ConfigStruct {};
  ESP32ControllerDummy(ConfigDummy &&config_data, InputData &&input_data)
      : ESP32ControllerBase<ConfigDummy, InputData>(ConfigDummy{std::move(config_data)}, std::move(input_data)) {}
  bool begin() override {
    return true;
  }
  bool update() override {
    return false;
  }
};

template <typename InputData, typename OutputData>
class ESP32ControllerResponseDummy : public ESP32ControllerResponseBase<ESP32ControllerDummy<InputData>, ESP32ControllerResponseDummy::ConfigResponseDummy, InputData, OutputData> {
public:
  struct ConfigResponseDummy : public ESP32ControllerDummy<InputData>::ConfigDummy {};
  ESP32ControllerResponseDummy(ConfigResponseDummy &&config_data, InputData &&input_data, OutputData &&output_data)
      : ESP32ControllerResponseBase<ESP32ControllerDummy<InputData>, ConfigResponseDummy, InputData, OutputData>(
        std::move(config_data), std::move(input_data), std::move(output_data)) {}
  bool begin() override {
    return true;
  }
  bool update() override {
    return false;
  }
  void _send() override {
    return;
  }
};
#endif
