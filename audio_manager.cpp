#include "audio_manager.h"
#include "network_manager.h"
#include "display_manager.h"
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <M5Core2.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <WiFiMulti.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 1024
#define DATA_SIZE (1024 * BUFFER_SIZE)

uint8_t *microphonedata0 = NULL;
size_t data_offset = 0;

extern char *apiKey;

// Configuration
#define SAMPLE_RATE 44100
#define BITS_PER_SAMPLE 16
const int CHANNELS = 1;

void sendAudioToWhisper(String &transcribedText) {

  WiFiClientSecure client;
  client.setInsecure(); // Use with caution. For production, you should use
                        // client.setCACert or client.setCertificate

  if (!client.connect("api.openai.com", 443)) {
    printToCanvas("Connection failed!\n");
    return;
  }

  // Add WAV header to the audio data
  addWavHeader(microphonedata0, data_offset);
  int totalDataSize = data_offset + 44; // Add header size to data size

  String boundary = "boundary123";
  String contentType = "multipart/form-data; boundary=" + boundary;

  String bodyStart = "--" + boundary + "\r\n";
  bodyStart += "Content-Disposition: form-data; name=\"file\"; "
               "filename=\"audio.wav\"\r\n";
  bodyStart += "Content-Type: audio/wav\r\n\r\n";

  String bodyEnd = "\r\n--" + boundary + "\r\n";
  bodyEnd += "Content-Disposition: form-data; name=\"model\"\r\n\r\nwhisper-1\r\n";
  bodyEnd += "--" + boundary + "--\r\n";

  int contentLength = bodyStart.length() + totalDataSize + bodyEnd.length();

  const char *endpoint = "https://api.openai.com/v1/audio/transcriptions";
  client.print(String("POST ") + endpoint + " HTTP/1.1\r\n");
  client.print("Host: api.openai.com\r\n");
  client.print("Authorization: Bearer " + String(apiKey) + "\r\n");
  client.print("Content-Type: " + contentType + "\r\n");
  client.print("Content-Length: " + String(contentLength) + "\r\n");
  client.print("\r\n");

  client.print(bodyStart);
  
  int count = 0;
  unsigned long previousMillis = millis();
  for (int i = 0; i < totalDataSize; i += BUFFER_SIZE) {
    int chunkSize =
        (totalDataSize - i) < BUFFER_SIZE ? (totalDataSize - i) : BUFFER_SIZE;
    client.write(&microphonedata0[i], chunkSize);

    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= 1000) {
    set_colour(VIM_DARK_CYAN);
      printToCanvas("Sending recording: %ds\n", ++count);
      previousMillis = currentMillis;
    }
  }
  client.print(bodyEnd);

  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      break;
    }
  }
  String response = client.readString();

  // Parse the JSON response to extract the "text" field
  DynamicJsonDocument doc(2048); // Allocate a JSON document of sufficient size
  DeserializationError error = deserializeJson(doc, response);
  if (error) {
    printToCanvas("JSON parse failed: %s\n", error.c_str());
    return;
  }

  const char *text = doc["text"];
  if (text) {
    transcribedText = String(text);

  } else {
    printToCanvas("Text not found in response\n");
  }

  client.stop();
}


void addWavHeader(uint8_t *data, int dataSize) {
  // WAV header size is 44 bytes
  uint8_t header[44];

  // RIFF header
  memcpy(header, "RIFF", 4);
  uint32_t fileSize = 36 + dataSize;
  memcpy(header + 4, &fileSize, 4);
  memcpy(header + 8, "WAVE", 4);

  // fmt subchunk
  memcpy(header + 12, "fmt ", 4);
  uint32_t fmtChunkSize = 16;
  memcpy(header + 16, &fmtChunkSize, 4);
  uint16_t audioFormat = 1;
  memcpy(header + 20, &audioFormat, 2);
  memcpy(header + 22, &CHANNELS, 2);
  uint32_t sampleRate = SAMPLE_RATE;
  memcpy(header + 24, &sampleRate, 4);
  uint32_t byteRate = SAMPLE_RATE * CHANNELS * (BITS_PER_SAMPLE / 8);
  memcpy(header + 28, &byteRate, 4);
  uint16_t blockAlign = CHANNELS * (BITS_PER_SAMPLE / 8);
  memcpy(header + 32, &blockAlign, 2);
  uint16_t bitsPerSample = BITS_PER_SAMPLE;
  memcpy(header + 34, &bitsPerSample, 2);

  // data subchunk
  memcpy(header + 36, "data", 4);
  memcpy(header + 40, &dataSize, 4);

  // Prepend the header to the actual data
  memmove(data + 44, data, dataSize);
  memcpy(data, header, 44);
}

bool readAudioData() {
    size_t byte_read;
    static int last_reported_percentage;
    i2s_read(Speak_I2S_NUMBER, (char *)(microphonedata0 + data_offset + 44),
            BUFFER_SIZE, &byte_read, (100 / portTICK_RATE_MS));
    data_offset += byte_read;

    // Calculate the percentage of the buffer filled and check if it has crossed a 10% threshold
    int percentage_filled = (data_offset * 100) / DATA_SIZE;
    if (percentage_filled / 10 > last_reported_percentage / 10) {
        last_reported_percentage = percentage_filled;
        set_colour(VIM_DARK_GREEN);
        printToCanvas("Recording remaining: %d%%\n", 100- percentage_filled);
    }

    if (data_offset >= DATA_SIZE) {
        printToCanvas("Buffer full!\n");
        return false;
    }
    return true;
}

void recordAudio(void) {
  if (microphonedata0 == NULL) {
    microphonedata0 = (uint8_t *)malloc(DATA_SIZE + 44); // space for the WAV header
    if (microphonedata0 == NULL) {
      printToCanvas("Memory allocation failed!\n");
      while (1) {
        delay(1000);
      }
    }
  }

  data_offset = 0;
  M5.Spk.InitI2SSpeakOrMic(MODE_MIC);
  size_t byte_read;
  unsigned long touch_start_time = 0;
  bool touch_detected = false;
  unsigned long recording_duration = 5000; // 5 seconds for short touch

  while (true) {
    if (M5.Touch.points > 0) {
      if (!touch_detected) {
        touch_start_time = millis();
        touch_detected = true;
      }

      if (millis() - touch_start_time > 300) { // long press behavior
        while (M5.Touch.points > 0) {
          if (!readAudioData()) {
            break;
          }
          M5.update();
        }
        break;
      }
    } else {
      if (touch_detected && millis() - touch_start_time <= 300) { // short touch detected
        unsigned long start_time = millis();
        while (millis() - start_time < recording_duration) {
          if (!readAudioData()) {
            break;
          }
          M5.update();
        }
        break;
      }
      touch_detected = false;
    }
    M5.update();
  }
}
