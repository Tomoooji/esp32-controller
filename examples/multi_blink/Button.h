/**
 * @file Button.h
 * @brief ボタン用クラス
 * 
 * @author Tomoooji (https://github.com/Tomoooji)
 * @version 0.1
 * @date 2026-07-27
 * @copyright Copyright (c) 2026
 */
#pragma once

#include <Arduino.h>

class Button{
private:
  uint8_t pin_;
  bool state_;
  unsigned long last_change_time_;
  unsigned long debounce_time_;

public:
  Button(uint8_t pin, unsigned long debounce_time = 50) : pin_(pin), state_(HIGH), last_change_time_(0), debounce_time_(debounce_time) {}

  void begin() {
    pinMode(pin_, INPUT_PULLUP);
  }

  bool isPressed() {
    bool current_state = digitalRead(pin_);
    if (current_state != state_) {
      unsigned long current_time = millis();
      if (current_time - last_change_time_ > debounce_time_) {
        state_ = current_state;
        last_change_time_ = current_time;
        if (state_ == LOW) {
          return true;
        }
      }
    }
    return false;
  }
};
