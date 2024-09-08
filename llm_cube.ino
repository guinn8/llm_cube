#include <M5Core2.h>
#include <stdlib.h>
#include <string.h>
#include <HTTPClient.h>
#include <vector>
#include <M5GFX.h>

#include "audio_manager.h"
#include "network_manager.h"
#include "display_manager.h"

const int screenWidth = 320;
const int screenHeight = 240;


void setup() {
    M5.begin(true, true, true, true, kMBusModeOutput, true);
    M5.Axp.SetSpkEnable(true);
    displayInit();
    delay(100);
    set_colour(VIM_DARK_YELLOW);
    printToCanvas("Connecting...\n");
    connectToWiFi();
    set_colour(VIM_DARK_YELLOW);
    printToCanvas("Hold to record!\n");
}

bool transcribed = false;
void loop() {
    M5.update();
    if (M5.Touch.points > 0) {
        TouchPoint_t pos = M5.Touch.point[0];
        if(!transcribed) {
            String transcription;

            M5.Axp.SetVibration(true);
            delay(100);
            M5.Axp.SetVibration(false);

            set_colour(VIM_DARK_GREEN);
            printToCanvas("Recording...\n");

            recordAudio();
            set_colour(VIM_DARK_CYAN);
            printToCanvas("Transcribing...\n");


            M5.Axp.SetVibration(true);
            delay(100);
            M5.Axp.SetVibration(false);

            sendAudioToWhisper(transcription);

            if (!transcription.isEmpty()) {
                clearScreen();
                printToCanvas("%s\n", transcription.c_str());
                printToCanvas("<------------------------>\n");

                sendHttpRequest(transcription.c_str());
            } else {
                printToCanvas("No transcription received.\n");
            }
            
            transcribed = true;
        }

        if (pos.y < screenHeight / 2) { // Right side, upper half for scrolling up
            M5.Axp.SetVibration(true);
            scroll(UP);
            M5.Axp.SetVibration(false);
        } else if (pos.y >= screenHeight / 2) { // Right side, lower half for scrolling down
            M5.Axp.SetVibration(true);
            scroll(DOWN);
            M5.Axp.SetVibration(false);
        }
    }
}
