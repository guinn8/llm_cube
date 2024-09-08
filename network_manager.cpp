#include <M5Core2.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <WiFiClientSecure.h>
#include <stdlib.h>
#include <string.h>
#include <ArduinoJson.h>  // Include the ArduinoJson library
#include <HTTPClient.h>
#include "audio_manager.h"
#include "network_manager.h"
#include "display_manager.h"

const char* ssid = MY_SSID;
const char* password = MY_PASSWORD;
const char* apiKey = MY_APIKEY;

HTTPClient http;
WiFiMulti wifiMulti;


void sendHttpRequest(const char *prompt) {
    http.begin("https://api.openai.com/v1/chat/completions");
    http.addHeader("Content-Type", "application/json");
    http.addHeader("Authorization", String("Bearer ") + apiKey);

    String payload = String("{\"model\": \"gpt-4o-mini\", \"messages\": [") +
                    "{\"role\": \"system\", \"content\": \"You are a very concise helpful assistant. Break your output into chunks with newlines every couple of sentences. Always respond in ascii compatable output. ONLY MARKUP TEXT WITH EXACTLY: #headings, ##headings, ###headings. 3 LEVELS OF HEADINGS! NO OTHER FORMATTING IS SUPPORTED OR ALLOWED!\"}," +
                    "{\"role\": \"user\", \"content\": \"" + prompt + "\"}], \"max_tokens\": 1000, \"stream\": true}";

    int httpCode = http.POST(payload);

    if (httpCode > 0) {
        if (httpCode == HTTP_CODE_OK) {
            processHttpResponse();
        } else {
            printToCanvas("HTTP POST failed, error: %s\n", http.getString().c_str());
        }
    } else {
        printToCanvas("HTTP POST failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end();
}

void processHttpResponse() {
    WiFiClient* stream = http.getStreamPtr();
    while (http.connected()) {
        if (stream->available()) {
            String line = stream->readStringUntil('\n');
            line.trim();
            if (line == "data: [DONE]") break;

            if (line.startsWith("data:")) {
                line.remove(0, 5);

                StaticJsonDocument<5024> doc;
                DeserializationError error = deserializeJson(doc, line);

                if (!error) {
                    const char* content = doc["choices"][0]["delta"]["content"];
                    if (content) printToCanvas(content);

                } else {
                    printToCanvas("Deserialization error\n");
                }
            }
        }
    }
    printToCanvas("\n"); // needed to flush the last line
    printToCanvas("\n"); // needed to flush the last line
}

void connectToWiFi() {

    wifiMulti.addAP(ssid, password);
    while (wifiMulti.run() != WL_CONNECTED) {
        printToCanvas(".");
        delay(1000);
    }
}
