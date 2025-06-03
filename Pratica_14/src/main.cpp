#include <Arduino.h>

#define RED_PIN 15
#define POTENTIOMETER 36

void setup() {
  Serial.begin(9600);
  pinMode(RED_PIN, OUTPUT);
  analogSetAttenuation(ADC_11db);
}

void loop() {
  int analogValue = analogRead(POTENTIOMETER);
  float brightness = map(analogValue, 0, 4095, 0, 255);
  analogWrite(RED_PIN, brightness);
  Serial.print("Analog: ");
  Serial.print(analogValue);
  Serial.print(", Brightness: ");
  Serial.println(brightness);
  delay(100);
}
