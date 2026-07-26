#include <WiFi.h>
#include <esp_now.h>
#include "../Button.h"

const char[] peer_mac = "00:00:00:00:00:00"; // 送信先のMACアドレスを指定してください

Button button1(18);
Button button2(19);

struct BlinkCommand {
  bool is_on;
  int power;
} __attribute__((__packed__));
volatile BlinkCommand command;

void send_cb(const esp_now_send_info_t *mac_addr, esp_now_send_status_t status) {
  if (status == ESP_NOW_SEND_SUCCESS) {
    Serial.println("Send success");
  }
  else {
    Serial.println("Send fail");
  }
}

void setup() {
  Serial.begin(115200);

  button1.begin();
  button2.begin();

  WiFi.mode(WIFI_STA);
  if(esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  esp_now_peer_info_t peerInfo;
  memset(&peerInfo,0,sizeof(peerInfo));//
  memcpy(peerInfo.peer_addr, peer_mac, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  
  if(esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Error adding peer");
    return;
  }

  esp_now_register_send_cb(send_cb);
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

  esp_now_send(peer_mac, reinterpret_cast<uint8_t*>(&command), sizeof(BlinkCommand));
  Serial.println("Command sent");

  delay(10);
}