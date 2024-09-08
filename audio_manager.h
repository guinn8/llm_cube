#ifndef AUDIO_MANAGER_H
#define AUDIO_MANAGER_H



#include <WiFiClientSecure.h>

void addWavHeader(uint8_t *data, int dataSize);
void sendAudioToWhisper(String &transcribedText);
void recordAudio(void);

extern uint8_t *microphonedata0;
extern size_t data_offset;

#endif // AUDIO_MANAGER_H
