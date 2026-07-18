/**
 * @file Controller_PS4.h
 * @brief 
 * 
 * @author Tomoooji (https://github.com/Tomoooji)
 * @version 0.1
 * @date 2026-07-18
 * @copyright Copyright (c) 2026
 * 
 * @note 
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

/** @brief  */
struct Config_PS4{
  const char* mac = nullptr;
};

/**
 * @brief 
 * 
 * @tparam InputData 
 */
template <typename InputData>
class Controller_PS4 :public Controller_Base<Config_PS4,InputData>{
public:
  using Controller_Base<Config_PS4,InputData>::Controller_Base;
  
  /**
   * @brief 
   * 
   * @return true 
   * @return false 
   */
  bool begin() override{
    return PS4.begin(this->config.mac);
  }

  /**
   * @brief 
   * 
   * @return true 
   * @return false 
   */
  bool update() override{
    if(PS4.isConnected()){
      this->command.apply();
      return true;
    }
    return false;
  }
};
template <typename InputData>
using Controller = Controller_PS4<InputData>;

#endif