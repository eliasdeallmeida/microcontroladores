#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <Arduino.h>

const char* ssid = "SSID";
const char* password = "PASSWORD";

<<<<<<< HEAD
#define BOTtoken "BOT_TOKEN"
=======
#define BOT_TOKEN "BOT_TOKEN"
>>>>>>> d8008ff (Fix: informações de WiFi e bot do Telegram)
#define CHAT_ID "CHAT_ID"

WiFiClientSecure client;
UniversalTelegramBot bot(BOT_TOKEN, client);

int botRequestDelay = 1000;
unsigned long lastTimeBotRan;

#define LED_VERMELHO 13
#define LED_AZUL 12
#define LED_VERDE 14

void handleNewMessages(int numNewMessages) {
  Serial.println("handleNewMessages");
  Serial.println(String(numNewMessages));

  for (int i=0; i<numNewMessages; i++) {
    String chat_id = String(bot.messages[i].chat_id);
    if (chat_id != CHAT_ID){
      bot.sendMessage(chat_id, "Usuário não autorizado", "");
      continue;
    }
    
    String text = bot.messages[i].text;
    Serial.println(text);

    String from_name = bot.messages[i].from_name;

    if (text == "/start") {
      String welcome = "Bem vindo, " + from_name + ".\n";
      welcome += "Use os seguintes comandos para controlar o circuito.\n\n";
      welcome += "/led_vermelho_on \n";
      welcome += "/led_vermelho_off\n";
      welcome += "/led_azul_on \n";
      welcome += "/led_azul_off\n";
      welcome += "/led_verde_on \n";
      welcome += "/led_verde_off\n";
      bot.sendMessage(chat_id, welcome, "");
    }

    if (text == "/led_vermelho_on") {
      bot.sendMessage(chat_id, "LED vermelho foi ligado", "");
      digitalWrite(LED_AZUL, LOW);
      digitalWrite(LED_VERDE, LOW);
      digitalWrite(LED_VERMELHO, HIGH);
    }
    
    if (text == "/led_vermelho_off") {
      bot.sendMessage(chat_id, "LED vermelho foi desligado", "");
      digitalWrite(LED_VERMELHO, LOW);
    }

    if (text == "/led_azul_on") {
      bot.sendMessage(chat_id, "LED azul foi ligado", "");
      digitalWrite(LED_VERMELHO, LOW);
      digitalWrite(LED_VERDE, LOW);
      digitalWrite(LED_AZUL, HIGH);
    }
    
    if (text == "/led_azul_off") {
      bot.sendMessage(chat_id, "LED azul foi desligado", "");
      digitalWrite(LED_AZUL, LOW);
    }

    if (text == "/led_verde_on") {
      bot.sendMessage(chat_id, "LED verde foi ligado", "");
      digitalWrite(LED_AZUL, LOW);
      digitalWrite(LED_VERMELHO, LOW);
      digitalWrite(LED_VERDE, HIGH);
    }
    
    if (text == "/led_verde_off") {
      bot.sendMessage(chat_id, "LED verde foi desligado", "");
      digitalWrite(LED_VERDE, LOW);
    }
  }
}

void setup() {
  pinMode(LED_VERMELHO, OUTPUT);
  pinMode(LED_AZUL, OUTPUT);
  pinMode(LED_VERDE, OUTPUT);

  Serial.begin(9600);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  #ifdef ESP32
    client.setCACert(TELEGRAM_CERTIFICATE_ROOT);
  #endif

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Conectando com o WiFi...");
  }

  Serial.println(WiFi.localIP());
}

void loop() {
  if (millis() > lastTimeBotRan + botRequestDelay)  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while(numNewMessages) {
      Serial.println("Resposta adquirida");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
  }
}
