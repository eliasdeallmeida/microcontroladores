#include <LittleFS.h>
#include <vector>
#include "rfid_card.h"

String cleanCardSerial(String serial) {
  if (serial.length() > 14) {
    serial = serial.substring(0, 14);
  }
  if (serial.startsWith("\x02") && serial.endsWith("\x03")) {
    return serial.substring(1, serial.length() - 3);
  }
  return serial;
}

bool isCardAuthorized(const String &serial) {
  File file = LittleFS.open("/authorized_cards.txt", "r");
  if (!file) {
    return false;
  }
  while (file.available()) {
    String line = file.readStringUntil('\n');
    line.trim();
    if (line == serial) {
      file.close();
      return true;
    }
  }
  file.close();
  return false;
}

void saveCardSerial(const String &serial) {
  if (isCardAuthorized(serial)) {
    return;
  }
  File file = LittleFS.open("/authorized_cards.txt", "a");
  if (file) {
    file.println(serial);
    file.close();
  }
}

void removeCardSerial(const String &serial) {
  File file = LittleFS.open("/authorized_cards.txt", "r");
  if (!file) {
    return;
  }
  std::vector<String> lines;
  while (file.available()) {
    String line = file.readStringUntil('\n');
    line.trim();
    if (line != serial) {
      lines.push_back(line);
    }
  }
  file.close();
  file = LittleFS.open("/authorized_cards.txt", "w");
  for (const auto &line : lines) {
    file.println(line);
  }
  file.close();
}
