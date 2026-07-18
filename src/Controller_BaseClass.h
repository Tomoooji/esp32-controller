/**
 * @file Controller_Base.h
 * @brief 各ライブラリの基底クラス
 * 
 * @author Tomoooji
 * @version 0.1
 * @date 2026-07-18
 * @copyright Copyright (c) 2026
 * 
 * @note 
 */
#pragma once
#include <Arduino.h>

/**
 * @brief 
 * 
 * @tparam ConfigData 
 * @tparam InputData 
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
   * @param config 
   * @param input 
   */
  explicit Controller_Base(ConfigData& config, InputData& input):config(config),command(input){}

  /**
   * @brief 
   * 
   * @return true 
   * @return false 
   */
  virtual bool begin() = 0;
  
  /**
   * @brief 
   * 
   * @return true 
   * @return false 
   */
  virtual bool update() = 0;

  /**
   * @brief Get the input object
   * 
   * @return const InputData& 
   */
  const InputData& get_input(){return this->command;}
  
  /**
   * @brief Get the config object
   * 
   * @return ConfigData& 
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
