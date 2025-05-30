#include <Arduino.h>

#define LED_PIN 13

int THRESHOLD = 40;
RTC_DATA_ATTR int bootCount = 0;

touch_pad_t touchPin;

void setup() {
  pinMode(LED_PIN, OUTPUT);
  Serial.begin(9600);
  ++bootCount;
  Serial.println("Boot number: " + String(bootCount));
  delay(1000);
  touchPin = esp_sleep_get_touchpad_wakeup_status();
  touchSleepWakeUpEnable(T5, THRESHOLD);
}

void loop() {
  Serial.println("Touch detected on GPIO 12");
  digitalWrite(LED_PIN, HIGH);
  delay(5000);
  Serial.println("Preparing to sleep now");
  Serial.flush();
  esp_deep_sleep_start();
}
