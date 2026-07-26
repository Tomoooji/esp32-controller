#define ESPNOW

#ifdef ESPNOW
  #include <ESP32_Controller_ESPNOW.h>
  Config_ESPNOW config;
#elif SERIAL
  #include <ESP32_Controller_Serial.h>
  Config_Serial config;
#elif I2C
  #include <ESP32_Controller_I2C.h>
  Config_I2C_Master config;
#elif BLUETOOTH
  #include <ESP32_Controller_BluetoothSerial.h>
  Config_BluetoothSerial config;
#endif

struct BlinkCommand {
    bool is_on;
    int power;
} __attribute__((__packed__)) command;

Controller<BlinkCommand> ctrler(config,command);
constexpr uint8_t pin_led = 2;

void setup() {
  if (!ctrler.begin()) return;
  ledcAttach(pin_led, 12800, 8);
  //ledcSetup(0, 12800, 8);
  //ledcAttachPin(pin_led, 0);
}

void loop() {
  if (ctrler.update()) {
    if (!ctrler.get_input().is_on) {
      ledcWrite(pin_led, 0);
      //ledcWrite(0, 0);
    }else {
      ledcWrite(pin_led, ctrler.get_input().power);
      //ledcWrite(0, ctrler.get_input().power);
    }
  }
  delay(10);
}