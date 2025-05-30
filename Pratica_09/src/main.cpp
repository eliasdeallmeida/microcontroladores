#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

#define LED 13
#define BUTTON 12

const char *ssid = "NOME_DA_REDE";
const char *password = "SENHA_DA_REDE";

const char *mqtt_broker = "broker.mqtt.cool";
const char *topic = "LAESE";
const int mqtt_port = 1883;

bool isLedOn = false;

String message;

WiFiClient espClient;
PubSubClient client(espClient);

void callback(char *topic, byte *payload, unsigned int length) {
  message = "";
  Serial.print("Message arrived in topic: ");
  Serial.println(topic);
  Serial.print("Message:");
  for (int i = 0; i < length; i++) {
    Serial.print((char) payload[i]);
    message += (char) payload[i];
  }
  Serial.println();
  Serial.println("-----------------------");
}

void setup() {
  Serial.begin(9600);

  pinMode(LED, OUTPUT);
  pinMode(BUTTON, INPUT_PULLUP);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi..");
  }

  Serial.println(WiFi.localIP());

  client.setServer(mqtt_broker, mqtt_port);
  client.setCallback(callback);
  while (!client.connected()) {
    String client_id = "esp32-client-";
    client_id += String(WiFi.macAddress());
    Serial.printf("The client %s connects to the public MQTT broker\n", client_id.c_str());
    if (client.connect(client_id.c_str())) {
      Serial.println("Public EMQX MQTT broker connected");
    } else {
      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(2000);
    }
  }

  client.subscribe(topic);
}

void loop() {
  int buttonState = digitalRead(BUTTON);

  if (buttonState == LOW) {
    isLedOn = !isLedOn;
    if (isLedOn) {
      client.publish(topic, "led_amarelo_on");
    }
    else {
      client.publish(topic, "led_amarelo_off");
    }
  }

  if (message == "led_verde_on") {
    digitalWrite(LED, HIGH);
    delay(1000);
  }
  else if (message == "led_verde_off") {
    digitalWrite(LED, LOW);
    delay(1000);
  }
  client.loop();
}
