#include <Arduino.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include <esp_now.h>

// a0:b7:65:26:9c:c8

#define LED 13
#define BUTTON 12

uint8_t broadcastAddress1[] = {0xf0, 0x24, 0xf9, 0x44, 0x10, 0x40};

typedef struct data {
  bool isLedOn;
} data;

data dataSent;
data dataReceived;

esp_now_peer_info_t peerInfo;

bool isLedOn = true;

void onDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  char macStr[18];
  Serial.print("Packet to: ");
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  Serial.print(macStr);
  Serial.print(" send status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void updateLedState(data dataReceived) {
  if (dataReceived.isLedOn) {
    digitalWrite(LED, HIGH);
  }
  else {
    digitalWrite(LED, LOW);
  }
}

void onDataReceived(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&dataReceived, incomingData, sizeof(dataReceived));
  Serial.print("Bytes received: ");
  Serial.println(len);
  Serial.print("Is Led On? ");
  Serial.println(dataReceived.isLedOn);
  Serial.println();
  updateLedState(dataReceived);
}

void readButton() {
  int buttonState = digitalRead(BUTTON);
  if (buttonState == LOW) {
    isLedOn = !isLedOn;
    dataSent.isLedOn = isLedOn;
  }
}

void checkDataSentSuccess(esp_err_t result) {
  if (result == ESP_OK) {
    Serial.println("Sent with success");
  }
  else {
    Serial.println("Error sending the data");
  }
}

void readMacAddress(){
  uint8_t baseMac[6];
  esp_err_t ret = esp_wifi_get_mac(WIFI_IF_STA, baseMac);
  if (ret == ESP_OK) {
    Serial.printf("%02x:%02x:%02x:%02x:%02x:%02x\n",
                  baseMac[0], baseMac[1], baseMac[2],
                  baseMac[3], baseMac[4], baseMac[5]);
  } else {
    Serial.println("Failed to read MAC address");
  }
}

void setup() {
  Serial.begin(9600);

  pinMode(LED, OUTPUT);
  pinMode(BUTTON, INPUT_PULLUP);

  WiFi.mode(WIFI_STA);

  Serial.print("ESP32 MAC Address: ");
  readMacAddress();

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  esp_now_register_send_cb(onDataSent);
  
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  memcpy(peerInfo.peer_addr, broadcastAddress1, 6);
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }

  esp_now_register_recv_cb(esp_now_recv_cb_t(onDataReceived));
}

void loop() {
  readButton();
  esp_err_t result = esp_now_send(0, (uint8_t *) &dataSent, sizeof(data));
  checkDataSentSuccess(result);
  delay(100);
}
