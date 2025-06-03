#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <WiFiManager.h>

int pino_reset = 4;

void setup() {
  Serial.begin(9600);

  delay(10);
  pinMode(pino_reset, INPUT);
  WiFiManager wifiManager;
  wifiManager.setConfigPortalTimeout(240);

  if (!wifiManager.autoConnect("PRATICA_12", "pratica12")) {
    Serial.println(F("Falha na conexao. Resetar e tentar novamente..."));
    delay(3000);
    ESP.restart();
    delay(5000);
  }

  Serial.println(F("Conectado na rede Wifi."));
  Serial.print(F("Endereco IP: "));
  Serial.println(WiFi.localIP());
}

void loop() {
  int valor = digitalRead(pino_reset);
  if (valor == 1) {
    WiFiManager wifiManager;
    wifiManager.resetSettings();
    Serial.println("Configuracoes zeradas!");
    ESP.restart();
  }
}
