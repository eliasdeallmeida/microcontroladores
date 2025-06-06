#include <Arduino.h>
#include <UniversalTelegramBot.h>
#include "telegram_bot.h"
#include "log_util.h"
#include "rfid_card.h"
#include "config.h"

extern UniversalTelegramBot bot;
extern float greenLevel, yellowLevel, redLevel;

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
    String message = "Comando recebido de " + from_name + ": " + text;
    Serial.println(message);
    logToFile(message);

    if (text == "/start") {
      String welcome = "Bem vindo, " + from_name + "!\n";
      welcome += "Use os comandos:\n";
      welcome += "/cadastrar <serial>\n";
      welcome += "/descadastrar <serial>\n";
      welcome += "/definir_nivel_verde <cm>\n";
      welcome += "/definir_nivel_amarelo <cm>\n";
      welcome += "/definir_nivel_vermelho <cm>\n";
      welcome += "/log";
      bot.sendMessage(chat_id, welcome, "");
    }
    else if (text.startsWith("/cadastrar")) {
      String serial = text.substring(11);
      serial.trim();
      if (serial.length() > 0) {
        saveCardSerial(serial);
        message = "Cartão cadastrado: " + serial;
        bot.sendMessage(chat_id, message, "");
        logToFile(message);
      } else {
        bot.sendMessage(chat_id, "Uso: /cadastrar <serial>", "");
      }
    }
    else if (text.startsWith("/descadastrar")) {
      String serial = text.substring(14);
      serial.trim();
      if (serial.length() > 0) {
        removeCardSerial(serial);
        message = "Cartão removido: " + serial;
        bot.sendMessage(chat_id, message, "");
        logToFile(message);
      } else {
        bot.sendMessage(chat_id, "Uso: /descadastrar <serial>", "");
      }
    }
    else if (text.startsWith("/definir_nivel_verde")) {
      float value = text.substring(21).toFloat();
      if (value > 0) {
        greenLevel = value;
        bot.sendMessage(chat_id, "Nível verde definido: " + String(value), "");
      } else {
        bot.sendMessage(chat_id, "Uso: /definir_nivel_verde <cm>", "");
      }
    }
    else if (text.startsWith("/definir_nivel_amarelo")) {
      float value = text.substring(23).toFloat();
      if (value > 0) {
        yellowLevel = value;
        bot.sendMessage(chat_id, "Nível amarelo definido: " + String(value), "");
      } else {
        bot.sendMessage(chat_id, "Uso: /definir_nivel_amarelo <cm>", "");
      }
    }
    else if (text.startsWith("/definir_nivel_vermelho")) {
      float value = text.substring(24).toFloat();
      if (value > 0) {
        redLevel = value;
        bot.sendMessage(chat_id, "Nível vermelho definido: " + String(value), "");
      } else {
        bot.sendMessage(chat_id, "Uso: /definir_nivel_vermelho <cm>", "");
      }
    }
    else if (text == "/log") {
      String content = readLastLinesOfLog(20);
      if (content.isEmpty()) {
        bot.sendMessage(chat_id, "O log está vazio.", "");
      } else {
        bot.sendMessage(chat_id, content, "");
      }
    }
  }
}
