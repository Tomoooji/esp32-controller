#pragma once
#include <Arduino.h>

struct BlinkCommand {
    bool is_on;
    int power;
} __attribute__((__packed__));
