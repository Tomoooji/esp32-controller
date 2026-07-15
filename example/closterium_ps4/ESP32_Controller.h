#pragma once
#include "Controller_BaseClass.h"

#ifndef CONTROLLER_TYPE

#elif CONTROLLER_TYPE == CONTROLLER_PS4
  #include "Controller_PS4.h"
  using Controller = Controller_PS4;

#elif CONTROLLER_TYPE == CONTROLLER_SERIAL
  #include "Controller_Serial.h"
  using Controller = Controller_Serial;
  //using Controller_Response = Controller_Serial_Response;

#elif CONTROLLER_TYPE == CONTROLLER_I2C
  #include "Controller_I2C.h"
  using Controller = Controller_I2C;
  //using Controller_Response = Controller_I2C_Response;

#elif CONTROLLER_TYPE == CONTROLLER_ESPNOW
  #include "Controller_ESPNOW.h"
  using Controller = Controller_ESPNOW;
  using Controller_Response = Controller_ESPNOW_Response;

/*
#elif CONTROLLER_TYPE == CONTROLLER_REMOTEXY
#include "Controller_RemoteXY.h"
using Controller = Controller_RemoteXY;
*/

#endif

