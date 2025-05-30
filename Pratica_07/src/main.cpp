#include <Arduino.h>
#include <WiFi.h>
#include <LittleFS.h>
#include <ArduinoJson.h>

#define RED_LED_PIN 13
#define GREEN_LED_PIN 12
#define BLUE_LED_PIN 14
#define BUTTON_PIN 27
#define FORMAT_LITTLEFS_IF_FAILED true

int ledCurrentlyOn = 0;
const char* jsonFilePath = "/config.json";
String ledColor = "nenhuma";

void saveJSON();
void connectWiFiFromJSON();
void addWiFiNetworkToJSON(const char*, const char*);
void printSavedConfig();

void saveJSON() {
  DynamicJsonDocument doc(1024);
  File file = LittleFS.open("/config.json", "r");
  if (file) {
    DeserializationError error = deserializeJson(doc, file);
    file.close();
    if (error) {
      Serial.println("Erro ao ler JSON existente, substituindo.");
      doc.clear();
    }
  }
  if (!doc.containsKey("wifi") || !doc["wifi"].is<JsonArray>()) {
    doc["wifi"] = JsonArray();
  }
  JsonObject led = doc.createNestedObject("led");
  led["color"] = ledColor;
  led["red"] = digitalRead(RED_LED_PIN);
  led["green"] = digitalRead(GREEN_LED_PIN);
  led["blue"] = digitalRead(BLUE_LED_PIN);
  file = LittleFS.open("/config.json", "w");
  if (!file) {
    Serial.println("Erro ao abrir arquivo para escrita");
    return;
  }
  serializeJsonPretty(doc, file);
  file.close();
  printSavedConfig();
}

void connectWiFiFromJSON() {
  File file = LittleFS.open(jsonFilePath, "r");
  if (!file) {
    Serial.println("Arquivo JSON não encontrado. Criando um novo.");
    saveJSON();
    return;
  }
  DynamicJsonDocument doc(512);
  DeserializationError error = deserializeJson(doc, file);
  file.close();
  if (error) {
    Serial.println("Erro ao ler JSON");
    return;
  }
  JsonArray wifiList = doc["wifi"];
  for (JsonObject net : wifiList) {
    const char* ssid = net["ssid"];
    const char* password = net["password"];
    Serial.printf("Tentando conectar em: %s\n", ssid);
    WiFi.begin(ssid, password);
    unsigned long startTime = millis();
    while (WiFi.status() != WL_CONNECTED && (millis() - startTime) < 8000) {
      if (WiFi.status() == WL_CONNECTED) break;
      yield();
    }
    if (WiFi.status() == WL_CONNECTED) {
      Serial.printf("Conectado em: %s\n", ssid);
      return;
    }
    WiFi.disconnect(true);
    Serial.printf("Falha ao conectar em: %s\n", ssid);
  }
  Serial.println("Não foi possível conectar a nenhuma rede.");
}

void addWiFiNetworkToJSON(const char* ssid, const char* password) {
  DynamicJsonDocument doc(1024);
  File file = LittleFS.open("/config.json", "r");
  if (file) {
    DeserializationError error = deserializeJson(doc, file);
    file.close();
    if (error) {
      Serial.println("Erro ao ler JSON existente. Criando novo.");
      doc.clear();
    }
  } else {
    Serial.println("Arquivo não encontrado. Criando novo.");
  }
  if (!doc.containsKey("wifi")) {
    doc.createNestedArray("wifi");
  }
  JsonArray wifiList = doc["wifi"];
  for (JsonObject net : wifiList) {
    if (net["ssid"] == ssid) {
      Serial.println("Essa rede já está salva. Ignorando.");
      return;
    }
  }
  JsonObject newNet = wifiList.createNestedObject();
  newNet["ssid"] = ssid;
  newNet["password"] = password;
  file = LittleFS.open("/config.json", "w");
  if (!file) {
    Serial.println("Erro ao abrir arquivo para escrita.");
    return;
  }
  serializeJsonPretty(doc, file);
  file.close();
  Serial.println("Nova rede adicionada ao JSON com sucesso!");
  printSavedConfig();
}

void printSavedConfig() {
  File file = LittleFS.open("/config.json", "r");
  if (!file) {
    Serial.println("Arquivo de configuração não encontrado.");
    return;
  }
  DynamicJsonDocument doc(1024);
  DeserializationError error = deserializeJson(doc, file);
  file.close();
  if (error) {
    Serial.print("Erro ao ler JSON: ");
    Serial.println(error.c_str());
    return;
  }
  Serial.println("=== Configuração Salva no JSON ===");
  if (doc.containsKey("wifi") && doc["wifi"].is<JsonArray>()) {
    JsonArray wifiList = doc["wifi"].as<JsonArray>();
    if (wifiList.size() > 0) {
      Serial.println("Redes WiFi:");
      for (JsonObject net : wifiList) {
        const char* ssid = net["ssid"];
        const char* password = net["password"];
        Serial.printf("  - SSID: %s, Senha: %s\n", ssid ? ssid : "(vazio)", password ? password : "(vazio)");
      }
    } else {
      Serial.println("Nenhuma rede WiFi salva.");
    }
  } else {
    Serial.println("Campo 'wifi' ausente ou inválido.");
  }
  if (doc.containsKey("led") && doc["led"].is<JsonObject>()) {
    JsonObject led = doc["led"];
    Serial.println("Estado do LED:");
    Serial.printf("  - Cor atual: %s\n", led["color"] | "nenhuma");
    Serial.printf("  - Vermelho: %s\n", led["red"] ? "ligado" : "desligado");
    Serial.printf("  - Verde: %s\n", led["green"] ? "ligado" : "desligado");
    Serial.printf("  - Azul: %s\n", led["blue"] ? "ligado" : "desligado");
  } else {
    Serial.println("Estado do LED não definido.");
  }
  Serial.println("===================================");
}

void setup() {
  Serial.begin(9600);
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);
  pinMode(BLUE_LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  if (!LittleFS.begin(FORMAT_LITTLEFS_IF_FAILED)) {
    Serial.println("Falha ao montar LittleFS");
    return;
  }
  addWiFiNetworkToJSON("A25", "wifisga25");
  connectWiFiFromJSON();
  printSavedConfig();
}

void loop() {
  static bool buttonPressed = false;
  int buttonState = digitalRead(BUTTON_PIN);
  if (buttonState == LOW && !buttonPressed) {
    buttonPressed = true;
    if (ledCurrentlyOn == 0) {
      digitalWrite(RED_LED_PIN, HIGH);
      ledColor = "vermelho";
      ledCurrentlyOn = RED_LED_PIN;
    } else if (ledCurrentlyOn == RED_LED_PIN) {
      digitalWrite(RED_LED_PIN, LOW);
      digitalWrite(GREEN_LED_PIN, HIGH);
      ledColor = "verde";
      ledCurrentlyOn = GREEN_LED_PIN;
    } else if (ledCurrentlyOn == GREEN_LED_PIN) {
      digitalWrite(GREEN_LED_PIN, LOW);
      digitalWrite(BLUE_LED_PIN, HIGH);
      ledColor = "azul";
      ledCurrentlyOn = BLUE_LED_PIN;
    } else if (ledCurrentlyOn == BLUE_LED_PIN) {
      digitalWrite(BLUE_LED_PIN, LOW);
      ledColor = "nenhuma";
      ledCurrentlyOn = 0;
    }
    saveJSON();
    delay(250);
  }
  if (buttonState == HIGH) {
    buttonPressed = false;
  }
}
