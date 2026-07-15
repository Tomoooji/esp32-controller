#pragma once
#include "Controller_BaseClass.h"

#ifndef CONTROLLER_TYPE

#elif CONTROLLER_TYPE == CONTROLLER_PS4
  #include "Controller_PS4.h"
  template <typename InputData>
  using Controller = Controller_PS4<InputData>;

#elif CONTROLLER_TYPE == CONTROLLER_SERIAL
  #include "Controller_Serial.h"
  template <typename InputData>
  using Controller = Controller_Serial<InputData>;
  //using Controller_Response = Controller_Serial_Response<InputData>;

#elif CONTROLLER_TYPE == CONTROLLER_I2C
  #include "Controller_I2C.h"
  template <typename InputData>
  using Controller = Controller_I2C<InputData>;
  //using Controller_Response = Controller_I2C_Response<InputData>;

#elif CONTROLLER_TYPE == CONTROLLER_ESPNOW
  #include "Controller_ESPNOW.h"
  template <typename InputData>
  using Controller = Controller_ESPNOW<InputData>;
  using Controller_Response = Controller_ESPNOW_Response<InputData>;

/*
#elif CONTROLLER_TYPE == CONTROLLER_REMOTEXY
  #include "Controller_RemoteXY.h"
  template <typename InputData>
  using Controller = Controller_RemoteXY<InputData>;
*/

#endif

