#include <BluetoothSerial.h>
#include "../blink_command.h"
#include "../Button.h"
// ArduinoIDEで実行する場合はblink_command.hとButton.hを同じフォルダにコピーしてください。

BluetoothSerial SerialBT;
const char* slave_name = "ESP32_BT";

Button button1(18);
Button button2(19);

BlinkCommand command;

void setup() {
  Serial.begin(115200);

  button1.begin();
  button2.begin();

  if (!SerialBT.begin("ESP32_Sender")) {
    Serial.println("Failed to initialize Bluetooth");
    return;
  }

  while (!SerialBT.connect(slave_name)) {
    Serial.println("Waiting for connection to slave...");
    delay(1000);
  }
  Serial.println("Connection successful!");
  
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

  if (SerialBT.connected()) {
    SerialBT.write(reinterpret_cast<uint8_t*>(&command), sizeof(BlinkCommand));
    Serial.println("Command sent via Serial");
  }
  else {
    Serial.println("Not connected to slave. Attempting to reconnect...");
    while (!SerialBT.connect(slave_name)) {
      Serial.println("Waiting for connection to slave...");
      delay(1000);
    }
    Serial.println("Connection successful!");
  }  
}