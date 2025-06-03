#include <Arduino.h>

#define SOUND_SPEED 0.034
#define NUM_MEASUREMENTS 51

const int trigPin = 5;
const int echoPin = 18;
const int buzzerPin = 15;

long duration;
float distanceCm;
float distances[NUM_MEASUREMENTS];

void setup() {
  Serial.begin(9600);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(buzzerPin, OUTPUT);
}

float measureDistance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  return duration * SOUND_SPEED / 2;
}

void sortArray(float arr[], int size) {
  for (int i = 0; i < size - 1; i++) {
    for (int j = 0; j < size - i - 1; j++) {
      if (arr[j] > arr[j + 1]) {
        float temp = arr[j];
        arr[j] = arr[j + 1];
        arr[j + 1] = temp;
      }
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
  
  Serial.print("Mediana da distância (cm): ");
  Serial.println(median);
  
  if (median < 20) {
    digitalWrite(buzzerPin, HIGH);
  }
  else if (median < 50) {
    digitalWrite(buzzerPin, HIGH);
    delay(200);
    digitalWrite(buzzerPin, LOW);
    delay(50);
  }
  else if (median < 100) {
    digitalWrite(buzzerPin, HIGH);
    delay(500);
    digitalWrite(buzzerPin, LOW);
    delay(100);
  }
}
