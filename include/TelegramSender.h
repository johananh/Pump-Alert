// TelegramBot.h

#ifndef TELEGRAMBOT_H
#define TELEGRAMBOT_H

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

class TelegramSender {
public:
    TelegramSender(const String& token, const String& chatId);
    void send(const String& msg);

private:
    String _token;
    String _chatId;
    WiFiClientSecure _client;
};

#endif
