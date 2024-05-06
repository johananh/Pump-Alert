
#include "TelegramSender.h"

#include <ArduinoJson.h>
#include <WiFiClientSecure.h>


TelegramSender::TelegramSender(const String& token, const String& chatId)
{
    _token = token;
    _chatId = chatId;
}


void TelegramSender::send(const String& msg) {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi not connected!");
        return;
    }

    WiFiClientSecure secureClient;
    secureClient.setInsecure(); // Use this only if you don't want to verify SSL certificate

    HTTPClient http;
    http.begin(secureClient, "https://api.telegram.org/bot" + _token + "/sendMessage");

    http.addHeader("Content-Type", "application/json");

    DynamicJsonDocument doc(1024);
    doc["chat_id"] = _chatId;
    doc["text"] = msg;

    String requestBody;
    serializeJson(doc, requestBody);

    int httpResponseCode = http.POST(requestBody);

    if (httpResponseCode > 0) {
        String response = http.getString();
        Serial.println(httpResponseCode);
        Serial.println(response);
    } else {
        Serial.print("Error on sending POST: ");
        Serial.println(httpResponseCode);
    }

    http.end();
}