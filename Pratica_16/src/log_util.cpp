#include <LittleFS.h>
#include "log_util.h"
#include "config.h"

void logToFile(const String &message) {
  String timestamp = getFormattedTime();
  String logEntry = "#" + String(logIndex) + " [" + timestamp + "] " + message;
  File file = LittleFS.open("/log.txt", "a");
  if (file) {
    file.println(logEntry);
    file.close();
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
  if (!getLocalTime(&timeinfo)) return "Data/Hora inválida";
  char buf[20];
  strftime(buf, sizeof(buf), "%d/%m/%Y %H:%M:%S", &timeinfo);
  return String(buf);
}

String readLastLinesOfLog(int qtd) {
  File file = LittleFS.open("/log.txt", "r");
  if (!file) return "";
  String content = file.readString();
  file.close();

  int count = 0;
  String output = "";
  for (int i = content.length() - 1; i >= 0; i--) {
    if (content[i] == '\n') count++;
    if (count == qtd) {
      output = content.substring(i + 1);
      break;
    }
  }
  if (output.length() == 0) return content;
  return output;
}
