// Bibliotecas externas
#include <Arduino.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <FS.h>
#include <LittleFS.h>
#include <time.h>

// Cabeçalhos dos arquivos do projeto
#include "config.h"
#include "distance_sensor.h"
#include "log_util.h"
#include "rfid_card.h"
#include "telegram_bot.h"

// Caixa d'água
#define WATER_TANK_GREEN 13
#define WATER_TANK_YELLOW 12
#define WATER_TANK_RED 14
#define TRIG_PIN 5
#define ECHO_PIN 18
#define NUM_MEASUREMENTS 51

float greenLevel = 10;
float yellowLevel = 20;
float redLevel = 30;
float distances[NUM_MEASUREMENTS];

// Fechadura da porta
#define DOOR_LOCK 27
#define RXD2 16
#define TXD2 17
#define BAUD 9600

HardwareSerial RFID(2);
String cardSerial;
time_t lastTimeReading = 0;

// Bot do Telegram
WiFiClientSecure client;
UniversalTelegramBot bot(BOT_TOKEN, client);

unsigned long lastTimeBotRan;
int botRequestDelay = 1000;

// Protótipos das tasks
void waterTankMeasuringTask(void *parameter);
void doorAccessTask(void *parameter);

void setup() {
  Serial.begin(9600);
  
  // Pinos do circuito
  pinMode(WATER_TANK_GREEN, OUTPUT);
  pinMode(WATER_TANK_YELLOW, OUTPUT);
  pinMode(WATER_TANK_RED, OUTPUT);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(DOOR_LOCK, OUTPUT);
  
  // Leitor RFID RDM6300
  RFID.begin(BAUD, SERIAL_8N1, RXD2, TXD2);
  Serial.println("Serial 2 iniciado com taxa de baud 9600");

  // LittleFS
  if (!LittleFS.begin()) {
    Serial.println("Erro ao montar LittleFS");
    return;
  }

  File logFile = LittleFS.open("/log.txt", "w");
  if (logFile) {
    logFile.println("### LOG INICIALIZADO ###");
    logFile.close();
  }

  File indexFile = LittleFS.open("/log_index.txt", "r");
  logIndex = indexFile ? indexFile.readStringUntil('\n').toInt() + 1 : 1;
  indexFile.close();

  if (!LittleFS.exists("/authorized_cards.txt")) {
    File cardsFile = LittleFS.open("/authorized_cards.txt", "w");
    if (cardsFile) {
      cardsFile.println("# Cartões cadastrados");
      cardsFile.close();
    }
  }

  // Rede WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  #ifdef ESP32
    client.setCACert(TELEGRAM_CERTIFICATE_ROOT);
  #endif

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Conectando com o WiFi...");
  }

  Serial.print("IP adquirido da rede WiFi: ");
  Serial.println(WiFi.localIP());

  // Horário
  configTime(-3 * 3600, 0, "pool.ntp.org", "time.nist.gov");
  struct tm timeinfo;
  while (!getLocalTime(&timeinfo)) {
    Serial.println("Aguardando sincronização NTP...");
    delay(1000);
  }

  time(&lastTimeReading);
  Serial.print("Hora inicial: ");
  Serial.println(lastTimeReading);

  // Tasks
  xTaskCreatePinnedToCore(waterTankMeasuringTask, "WaterTank", 4096, NULL, 1, NULL, 0);
  xTaskCreatePinnedToCore(doorAccessTask, "DoorAcess", 4096, NULL, 1, NULL, 1);
}

void loop() {
  // Recebe comandos do bot do Telegram
  if (millis() > lastTimeBotRan + botRequestDelay) {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    while (numNewMessages) {
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
  }
}

// Task de medição da caixa d'água
void waterTankMeasuringTask(void *parameter) {
  while (true) {
    for (int i = 0; i < NUM_MEASUREMENTS; i++) {
      distances[i] = measureDistance();
      delay(5);
    }

    sortArray(distances, NUM_MEASUREMENTS);
    float median = distances[NUM_MEASUREMENTS / 2];

    logToFile("Medição: " + String(median) + " cm");
    Serial.println("Mediana da distância (cm): " + String(median));

    digitalWrite(WATER_TANK_GREEN, LOW);
    digitalWrite(WATER_TANK_YELLOW, LOW);
    digitalWrite(WATER_TANK_RED, LOW);

    if (median < greenLevel) digitalWrite(WATER_TANK_GREEN, HIGH);
    else if (median < yellowLevel) digitalWrite(WATER_TANK_YELLOW, HIGH);
    else if (median < redLevel) digitalWrite(WATER_TANK_RED, HIGH);

    delay(1000);
  }
}

// Task de permissão da fechadura da porta
void doorAccessTask(void *parameter) {
  while (true) {
    while (RFID.available() > 0) {
      char cardChar = RFID.read();
      cardSerial += cardChar;
    }

    if (cardSerial.length() >= 14) {
      String cleanSerial = cleanCardSerial(cardSerial);
      String message;

      if (isCardAuthorized(cleanSerial)) {
        time_t currentTime;
        time(&currentTime);

        if (difftime(currentTime, lastTimeReading) >= 5) {
          lastTimeReading = currentTime;
          digitalWrite(DOOR_LOCK, LOW);
          delay(2000);
          digitalWrite(DOOR_LOCK, HIGH);
          message = "Acesso permitido para cartão: " + cleanSerial;
          Serial.println(message);
          logToFile(message);
        }
      }
      else {
        message = "Acesso negado para cartão: " + cleanSerial;
        Serial.println(message);
        logToFile(message);
      }
      cardSerial = "";
    }

    delay(200);
  }
}
