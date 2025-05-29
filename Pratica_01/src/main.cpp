#include <Arduino.h>

#define VERMELHO 23
#define VERDE 22
#define AZUL 1

void setup() {
  pinMode(VERMELHO, OUTPUT);
  pinMode(AZUL, OUTPUT);
  pinMode(VERDE, OUTPUT);
}

void acende_e_apaga_led(int cor) {
  digitalWrite(cor, HIGH);
  delay(1000);
  digitalWrite(cor, LOW);
}

void loop() {
  acende_e_apaga_led(VERMELHO);
  acende_e_apaga_led(VERDE);
  acende_e_apaga_led(AZUL);
}