#include <Wire.h>
#include "../Button.h"

uint8_t address = 0x2A;

Button button1(18);
Button button2(19);

struct BlinkCommand {
  bool is_on;
  int power;
} __attribute__((__packed__));
volatile BlinkCommand command;

void sendCommand() {
  if (Wire.write(reinterpret_cast<uint8_t*>(&command), sizeof(BlinkCommand)) == sizeof(BlinkCommand)) {
    Serial.println("Command sent via I2C");
  }
  else {
    Serial.println("Failed to send command via I2C");
  }
}

void setup() {
  Serial.begin(115200);

  button1.begin();
  button2.begin();

  if(!Wire.begin(address, 21, 22, 100000)) {
    Serial.println("Failed to initialize I2C");
    return;
  }
  Wire.onRequest(sendCommand);
}

void loop() {
  bool pushed_1 = button1.isPushed();
  bool pushed_2 = button2.isPushed();

  if (pushed_1 && pushed_2) {
    command.is_on = !command.is_on;
  } else if (pushed_1) {
    command.power = min(command.power + 10, 255);
  } else if (pushed_2) {
    command.power = max(command.power - 10, 0);
  }

  delay(10);
}