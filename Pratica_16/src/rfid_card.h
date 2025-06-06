#ifndef RFID_CARD_H
#define RFID_CARD_H

#include <Arduino.h>

String cleanCardSerial(String serial);
bool isCardAuthorized(const String &serial);
void saveCardSerial(const String &serial);
void removeCardSerial(const String &serial);

#endif
