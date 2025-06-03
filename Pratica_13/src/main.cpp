#include <Arduino.h>

#define PIN_RED 13
#define PIN_GREEN 12
#define PIN_BLUE 14

void setup() {
  Serial.begin(9600);

  pinMode(PIN_RED, OUTPUT);
  pinMode(PIN_GREEN, OUTPUT);
  pinMode(PIN_BLUE, OUTPUT);
}

void loop() {
  analogWrite(PIN_RED, 255);
  analogWrite(PIN_GREEN, 255);
  analogWrite(PIN_BLUE, 255);
  delay(1000);

  for(int dutyCycle = 255; dutyCycle >= 0; dutyCycle--){
    Serial.println(dutyCycle);
    analogWrite(PIN_GREEN, dutyCycle);   
    delay(15);
  }
  delay(1000);

  for(int dutyCycle = 255; dutyCycle >= 0; dutyCycle--){
    Serial.println(dutyCycle);
    analogWrite(PIN_RED, dutyCycle);   
    delay(15);
  }
  delay(1000);

  for(int dutyCycle = 255; dutyCycle >= 0; dutyCycle--){
    Serial.println(dutyCycle);
    analogWrite(PIN_BLUE, dutyCycle);   
    delay(15);
  }
  delay(1000);
}
