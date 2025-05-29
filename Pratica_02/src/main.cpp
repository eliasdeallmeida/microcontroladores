#include <Arduino.h>

#define VERMELHO_CARRO 13
#define AMARELO_CARRO 12
#define VERDE_CARRO 14
#define VERMELHO_PEDESTRE 27
#define VERDE_PEDESTRE 26
#define BOTAO 25

void setup() {
  Serial.begin(9600);

  pinMode(VERMELHO_CARRO, OUTPUT);
  pinMode(AMARELO_CARRO, OUTPUT);
  pinMode(VERDE_CARRO, OUTPUT);
  pinMode(VERMELHO_PEDESTRE, OUTPUT);
  pinMode(VERDE_PEDESTRE, OUTPUT);
  pinMode(BOTAO, INPUT_PULLUP);
}

void loop() {
  int leituraBotao = digitalRead(BOTAO);

  digitalWrite(VERDE_CARRO, HIGH);
  digitalWrite(VERMELHO_PEDESTRE, HIGH);

  if (leituraBotao == LOW) {
    digitalWrite(VERDE_CARRO, LOW);
    digitalWrite(VERMELHO_CARRO, HIGH);
    delay(500);
    digitalWrite(VERMELHO_PEDESTRE, LOW);
    digitalWrite(VERDE_PEDESTRE, HIGH);
    delay(4000);
    digitalWrite(VERMELHO_CARRO, LOW);

    for (int i = 0; i < 3; i++) {
      digitalWrite(AMARELO_CARRO, HIGH);
      delay(1000);
      digitalWrite(AMARELO_CARRO, LOW);
      digitalWrite(VERDE_PEDESTRE, LOW);
      delay(1000);
      digitalWrite(VERDE_PEDESTRE, HIGH);
    }
    
    digitalWrite(VERDE_CARRO, HIGH);
    digitalWrite(VERDE_PEDESTRE, LOW);
    digitalWrite(VERMELHO_PEDESTRE, HIGH);
  }
}