#include <Arduino.h>

#define RED_LED_PIN 13
#define GREEN_LED_PIN 12
#define RXD2 16
#define TXD2 17
#define GPS_BAUD 9600

HardwareSerial gpsSerial(2);

void setup() {
  Serial.begin(9600);
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);
  gpsSerial.begin(GPS_BAUD, SERIAL_8N1, RXD2, TXD2);
  Serial.println("Serial 2 started at 9600 baud rate");
}

void loop() {
  digitalWrite(RED_LED_PIN, HIGH);
  while (gpsSerial.available() > 0) {
    char gpsData = gpsSerial.read();
    Serial.print(gpsData);
    digitalWrite(RED_LED_PIN, LOW);
    digitalWrite(GREEN_LED_PIN, HIGH);
  }
  delay(1000);
  digitalWrite(GREEN_LED_PIN, LOW);
}
