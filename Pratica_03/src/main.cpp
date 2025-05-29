#include <Arduino.h>

#define VERMELHO_CARRO 13
#define AMARELO_CARRO 12
#define VERDE_CARRO 14
#define VERMELHO_PEDESTRE 27
#define VERDE_PEDESTRE 26
#define BOTAO 25

bool solicitarTravessia = false;

void botaoTask(void *param);
void pedestreAtravessa();

void setup() {
  pinMode(VERMELHO_CARRO, OUTPUT);
  pinMode(AMARELO_CARRO, OUTPUT);
  pinMode(VERDE_CARRO, OUTPUT);
  pinMode(VERMELHO_PEDESTRE, OUTPUT);
  pinMode(VERDE_PEDESTRE, OUTPUT);
  pinMode(BOTAO, INPUT_PULLUP);
  xTaskCreate(botaoTask, "Botao Task", 1024, NULL, 1, NULL);
  Serial.begin(9600);
}

void botaoTask(void *param) {
  while (true) {
  vTaskDelay(5);
    if (digitalRead(BOTAO) == LOW) {
      Serial.println("Botão pressionado");
      solicitarTravessia = true;
    }
  }
}

void piscarLED(int cor) {
  for (int i = 0; i < 3; i++) {
    digitalWrite(cor, HIGH);
    delay(1000);
    digitalWrite(cor, LOW);
    delay(500);
  }
}

void pedestreAtravessa() {
  Serial.println("Pedestre atravessando");
  solicitarTravessia = false;
  digitalWrite(VERDE_CARRO, LOW);
  piscarLED(AMARELO_CARRO);
  digitalWrite(VERMELHO_CARRO, HIGH);
  digitalWrite(VERMELHO_PEDESTRE, LOW);
  digitalWrite(VERDE_PEDESTRE, HIGH);
  delay(6000);
  digitalWrite(VERDE_PEDESTRE, LOW);
  digitalWrite(VERMELHO_CARRO, LOW);
  Serial.println("Pedestre atravessou");
}

void loop() {
  solicitarTravessia = false;
  digitalWrite(VERDE_CARRO, HIGH);
  digitalWrite(VERMELHO_PEDESTRE, HIGH);
  for(int i = 1; i <= 400; i++){
    delay(10);
    if (solicitarTravessia == true) {
      break;
    }
  }
  if(solicitarTravessia == true){
    delay(2000);
    pedestreAtravessa();
  }
  else {
    digitalWrite(VERDE_CARRO, LOW);
    piscarLED(AMARELO_CARRO);
    digitalWrite(VERMELHO_CARRO, HIGH);
    digitalWrite(VERMELHO_PEDESTRE, LOW);
    digitalWrite(VERDE_PEDESTRE, HIGH);
    delay(3000);
    digitalWrite(VERMELHO_CARRO, LOW);
    digitalWrite(VERDE_PEDESTRE, LOW);
  }
}
