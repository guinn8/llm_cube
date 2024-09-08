#include <M5Core2.h>
#include <stdlib.h>
#include <string.h>
#include <HTTPClient.h>
#include <vector>
#include <M5GFX.h>

#include "audio_manager.h"
#include "display_manager.h"
#include "network_manager.h"

const int screenWidth = 320;
const int screenHeight = 240;
int fontHeight;
int fontWidth;
int maxLinesOnScreen;
std::vector<String> textBuffer;
int startLine = 0;
M5GFX display;
M5Canvas canvas(&display);

static int saved_colour;
void set_colour(int colour){
    saved_colour = colour;
}

void setTextColorForHeading(const String& line) {
    if (line.startsWith("###")) {
        canvas.setTextColor(VIM_DARK_CYAN);
    } else if (line.startsWith("##")) {
        canvas.setTextColor(VIM_DARK_GREEN);
    } else if (line.startsWith("#")) {
        canvas.setTextColor(VIM_DARK_YELLOW);
    } else {
        canvas.setTextColor(saved_colour);
        saved_colour = VIM_LIGHT_GRAY;
    }
}

void drawLines(int startY, int lineIndex, int linesToDraw) {
    for (int i = 0; i < linesToDraw; i++) {
        if (lineIndex + i < textBuffer.size()) {
            canvas.setCursor(0, startY + i * fontHeight);
            setTextColorForHeading(textBuffer[lineIndex + i]);
            canvas.print(textBuffer[lineIndex + i].c_str());
        }
    }
}
void clearScreen() {
    canvas.fillScreen(TFT_BLACK); // Assuming TFT_BLACK is the background color
    canvas.pushSprite(0, 0); // Push the cleared canvas to the display
    textBuffer.clear();
    startLine = 0;
}

void updatePartialDisplay(const String& newText, ScrollDirection direction) {
    if (!newText.isEmpty()) {
        textBuffer.push_back(newText); // Add the new line of text
    }

    if (direction == DOWN) {
        startLine = (startLine + maxLinesOnScreen >= textBuffer.size()) ? textBuffer.size() - maxLinesOnScreen : startLine + 1;
        canvas.scroll(0, -fontHeight);
        drawLines(screenHeight - fontHeight, startLine + maxLinesOnScreen - 1, 1);
    } else if (direction == UP && startLine > 0) {
        startLine--;
        canvas.scroll(0, fontHeight);
        drawLines(0, startLine, 1);
    }

    canvas.pushSprite(0, 0);
}

void displayInit() {
    display.begin();
    canvas.setColorDepth(8); // mono color
    canvas.createSprite(display.width(), display.height());
    canvas.setTextSize((float)canvas.width() / 160);
    fontHeight = canvas.fontHeight(canvas.getFont());
    fontWidth = canvas.fontWidth(canvas.getFont());

    maxLinesOnScreen = screenHeight / fontHeight;
}

void scroll(ScrollDirection direction) {
    int totalLines = textBuffer.size();
    int maxStartLine = totalLines - maxLinesOnScreen;

    if ((direction == DOWN && startLine < maxStartLine) || (direction == UP && startLine > 0)) {
        updatePartialDisplay("", direction);
    }
}

void printToCanvas(const char* format, ...) {
    static String lineBuffer;
    char buffer[256];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    lineBuffer += buffer;
    while (lineBuffer.length() > 0) {
        int newlineIndex = lineBuffer.indexOf('\n');
        int lineWidth = canvas.textWidth(lineBuffer.c_str());

        if (newlineIndex >= 0 || lineWidth > screenWidth) {
            String line;
            line = lineBuffer.substring(0, screenWidth / canvas.textWidth(" "));
            lineBuffer = lineBuffer.substring(screenWidth / canvas.textWidth(" "));
            if (line.startsWith("#")) {
                updatePartialDisplay("\n", DOWN); // Scroll down before printing
            }
            updatePartialDisplay(line, DOWN);
        } else {
            break;
        }
    }
}
