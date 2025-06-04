#include <Arduino.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <FS.h>
#include <LittleFS.h>
#include <time.h>
#include <vector>

#define WATER_TANK_GREEN 13
#define WATER_TANK_YELLOW 12
#define WATER_TANK_RED 14

#define TRIG_PIN 5
#define ECHO_PIN 18

#define SOUND_SPEED 0.034
#define NUM_MEASUREMENTS 51

#define BOT_TOKEN "BOT_TOKEN"
#define CHAT_ID "CHAT_ID"

#define DOOR_LOCK 27
#define RXD2 16
#define TXD2 17
#define GPS_BAUD 9600

HardwareSerial gpsSerial(2);

String cartao;

float greenLevel = 10;
float yellowLevel = 20;
float redLevel = 30;

long duration;
float distanceCm;
float distances[NUM_MEASUREMENTS];

const char* ssid = "SSID";
const char* password = "PASSWORD";

int botRequestDelay = 1000;
unsigned long lastTimeBotRan;

int logIndex;

WiFiClientSecure client;
UniversalTelegramBot bot(BOT_TOKEN, client);

float measureDistance();
void sortArray(float array[], int size);
void handleNewMessages(int numMessages);
void logToFile(const String &message);
String getFormattedTime();
String limparCartao(String bruto);
bool cartaoAutorizado(const String &codigo);
void salvarCartao(const String &codigo);
void removerCartao(const String &codigo);

void setup() {
  Serial.begin(9600);

  pinMode(WATER_TANK_GREEN, OUTPUT);
  pinMode(WATER_TANK_YELLOW, OUTPUT);
  pinMode(WATER_TANK_RED, OUTPUT);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  pinMode(DOOR_LOCK, OUTPUT);
  gpsSerial.begin(GPS_BAUD, SERIAL_8N1, RXD2, TXD2);
  Serial.println("Serial 2 started at 9600 baud rate");

  if (!LittleFS.begin()) {
    Serial.println("Erro ao montar LittleFS");
    return;
  }

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

  configTime(-3 * 3600, 0, "pool.ntp.org", "time.nist.gov");

  struct tm timeinfo;
  while (!getLocalTime(&timeinfo)) {
    Serial.println("Aguardando sincronização NTP...");
    delay(1000);
  }

  File logFile = LittleFS.open("/log.txt", "w");
  if (logFile) {
    logFile.println("### LOG INICIALIZADO ###");
    logFile.close();
  } else {
    Serial.println("Falha ao criar log.txt");
  }

  File indexFile = LittleFS.open("/log_index.txt", "w");
  if (indexFile) {
    logIndex = indexFile.readStringUntil('\n').toInt() + 1;
    indexFile.close();
  }

    if (!LittleFS.exists("/cartoes.txt")) {
    File cartoesFile = LittleFS.open("/cartoes.txt", "w");
    if (cartoesFile) {
      cartoesFile.println("# Cartões cadastrados");
      cartoesFile.close();
      Serial.println("Arquivo cartoes.txt criado.");
    } else {
      Serial.println("Falha ao criar cartoes.txt");
    }
  }
}

void loop() {
  for (int i = 0; i < NUM_MEASUREMENTS; i++) {
    distances[i] = measureDistance();
    delay(5);
  }

  sortArray(distances, NUM_MEASUREMENTS);
  float median = distances[NUM_MEASUREMENTS / 2];

  String logMessage = "Medição: " + String(median) + " cm";
  logToFile(logMessage);

  Serial.print("Mediana da distância (cm): ");
  Serial.println(median);

  digitalWrite(WATER_TANK_GREEN, LOW);
  digitalWrite(WATER_TANK_YELLOW, LOW);
  digitalWrite(WATER_TANK_RED, LOW);

  if (median < greenLevel) {
    digitalWrite(WATER_TANK_GREEN, HIGH);
  } else if (median < yellowLevel) {
    digitalWrite(WATER_TANK_YELLOW, HIGH);
  } else if (median < redLevel) {
    digitalWrite(WATER_TANK_RED, HIGH);
  }

  while (gpsSerial.available() > 0) {
    char gpsData = gpsSerial.read();
    cartao += gpsData;
  }
  if (cartao.length() >= 14) {
    String codigoLimpo = limparCartao(cartao);
    if (cartaoAutorizado(codigoLimpo)) {
      digitalWrite(DOOR_LOCK, LOW);
      delay(2000);
      digitalWrite(DOOR_LOCK, HIGH);
      Serial.println("Acesso permitido para cartão: " + codigoLimpo);
      logToFile("Acesso permitido para cartão: " + codigoLimpo);
    } else {
      Serial.println("Acesso negado para cartão: " + codigoLimpo);
      logToFile("Acesso negado para cartão: " + codigoLimpo);
    }
  }
  cartao = "";

  if (millis() > lastTimeBotRan + botRequestDelay) {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while (numNewMessages) {
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
  }
}

float measureDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  duration = pulseIn(ECHO_PIN, HIGH);
  return duration * SOUND_SPEED / 2;
}

void sortArray(float array[], int size) {
  for (int i = 0; i < size - 1; i++) {
    for (int j = 0; j < size - i - 1; j++) {
      if (array[j] > array[j + 1]) {
        float temp = array[j];
        array[j] = array[j + 1];
        array[j + 1] = temp;
      }
    }
  }
}

void handleNewMessages(int numNewMessages) {
  for (int i = 0; i < numNewMessages; i++) {
    String chat_id = String(bot.messages[i].chat_id);
    String from_name = bot.messages[i].from_name;

    if (chat_id != CHAT_ID) {
      bot.sendMessage(chat_id, "Usuário não autorizado", "");
      logToFile("Tentativa de acesso não autorizado por: " + from_name);
      continue;
    }

    String text = bot.messages[i].text;
    String logMessage = "Comando recebido de " + from_name + ": " + text;
    Serial.println(logMessage);
    logToFile(logMessage);

    if (text == "/start") {
      String welcome = "Bem vindo, " + from_name + ".\n";
      welcome += "Use os seguintes comandos para controlar sua casa inteligente!\n\n";
      welcome += "/cadastrar <codigo_cartao>\n";
      welcome += "/descadastrar <codigo_cartao>\n";
      welcome += "/definir_nivel_verde <distancia_cm>\n";
      welcome += "/definir_nivel_amarelo <distancia_cm>\n";
      welcome += "/definir_nivel_vermelho <distancia_cm>\n";
      welcome += "/log_caixa_dagua";
      bot.sendMessage(chat_id, welcome, "");
    }
    else if (text.startsWith("/definir_nivel_verde")) {
      float value = text.substring(21).toFloat();
      if (value > 0) {
        greenLevel = value;
        bot.sendMessage(chat_id, "Nível verde definido para " + String(value) + " cm", "");
      } else {
        bot.sendMessage(chat_id, "Valor inválido. Use: /definir_nivel_verde <valor_em_cm>", "");
      }
    }
    else if (text.startsWith("/definir_nivel_amarelo")) {
      float value = text.substring(23).toFloat();
      if (value > 0) {
        yellowLevel = value;
        bot.sendMessage(chat_id, "Nível amarelo definido para " + String(value) + " cm", "");
      } else {
        bot.sendMessage(chat_id, "Valor inválido. Use: /definir_nivel_amarelo <valor_em_cm>", "");
      }
    }
    else if (text.startsWith("/definir_nivel_vermelho")) {
      float value = text.substring(24).toFloat();
      if (value > 0) {
        redLevel = value;
        bot.sendMessage(chat_id, "Nível vermelho definido para " + String(value) + " cm", "");
      } else {
        bot.sendMessage(chat_id, "Valor inválido. Use: /definir_nivel_vermelho <valor_em_cm>", "");
      }
    }
    else if (text == "/log_caixa_dagua") {
      File file = LittleFS.open("/log.txt", "r");
      if (!file) {
        bot.sendMessage(chat_id, "Erro ao abrir log.txt", "");
        return;
      }
      String logContent = "";
      while (file.available()) {
        logContent += (char)file.read();
      }
      file.close();

      int lastLines = 20;
      int count = 0;
      String lastLog = "";
      for (int i = logContent.length() - 1; i >= 0; i--) {
        if (logContent[i] == '\n') count++;
        if (count == lastLines) {
          lastLog = logContent.substring(i + 1);
          break;
        }
      }

      if (lastLog.length() == 0) lastLog = logContent;

      if (lastLog.length() == 0) {
        bot.sendMessage(chat_id, "O log está vazio.", "");
      } else {
        bot.sendMessage(chat_id, lastLog, "");
      }
    }
    else if (text.startsWith("/cadastrar")) {
    String codigo = text.substring(11);
    codigo.trim();
    if (codigo.length() > 0) {
      salvarCartao(codigo);
      bot.sendMessage(chat_id, "Cartão cadastrado com sucesso: " + codigo, "");
      logToFile("Cartão cadastrado: " + codigo);
    } else {
      bot.sendMessage(chat_id, "Uso: /cadastrar <codigo_cartao>", "");
    }
  }
  else if (text.startsWith("/descadastrar")) {
    String codigo = text.substring(14);
    codigo.trim();
    if (codigo.length() > 0) {
      removerCartao(codigo);
      bot.sendMessage(chat_id, "Cartão removido: " + codigo, "");
      logToFile("Cartão removido: " + codigo);
    } else {
      bot.sendMessage(chat_id, "Uso: /descadastrar <codigo_cartao>", "");
    }
  }
  }
}

void logToFile(const String &message) {
  String timestamp = getFormattedTime();
  String logEntry = "#" + String(logIndex) + " [" + timestamp + "] " + message;
  File file = LittleFS.open("/log.txt", "a");
  if (file) {
    file.println(logEntry);
    file.close();
  }
  else {
    Serial.println("Falha ao abrir log.txt para escrita");
  }
  logIndex++;
  File idxFile = LittleFS.open("/log_index.txt", "w");
  if (idxFile) {
    idxFile.println(String(logIndex));
    idxFile.close();
  }
}

String getFormattedTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return "Data/Hora inválida";
  }
  char buf[20];
  strftime(buf, sizeof(buf), "%d/%m/%Y %H:%M:%S", &timeinfo);
  return String(buf);
}

String limparCartao(String bruto) {
  if (bruto.length() > 14) {
    bruto = bruto.substring(0, 14);
  }
  if (bruto.startsWith("\x02") && bruto.endsWith("\x03")) {
    return bruto.substring(1, bruto.length() - 3);
  }
  return bruto;
}

bool cartaoAutorizado(const String &codigo) {
  File file = LittleFS.open("/cartoes.txt", "r");
  if (!file) return false;
  while (file.available()) {
    String linha = file.readStringUntil('\n');
    linha.trim();
    if (linha == codigo) {
      file.close();
      return true;
    }
  }
  file.close();
  return false;
}

void salvarCartao(const String &codigo) {
  if (cartaoAutorizado(codigo)) return;
  File file = LittleFS.open("/cartoes.txt", "a");
  if (file) {
    file.println(codigo);
    file.close();
  }
}

void removerCartao(const String &codigo) {
  File file = LittleFS.open("/cartoes.txt", "r");
  if (!file) return;
  std::vector<String> linhas;
  while (file.available()) {
    String linha = file.readStringUntil('\n');
    linha.trim();
    if (linha != codigo) linhas.push_back(linha);
  }
  file.close();

  file = LittleFS.open("/cartoes.txt", "w");
  for (const auto &linha : linhas) {
    file.println(linha);
  }
  file.close();
}
