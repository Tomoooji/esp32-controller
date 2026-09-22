#pragma once
#include <Arduino.h>
#include <BluePad32.h>
#include "ESP32Controller_Base.h"

struct Config_Bluepad32 {};

template <typename InputData>
class ESP32Controller_Bluepad32 : public ESP32Controller_Base<Config_Bluepad32, InputData> {
private:
public:
};