enum ScrollDirection { UP, DOWN };
void scroll(ScrollDirection direction);
void printToCanvas(const char* format, ...);
void displayText();
void displayInit();
void clearScreen();
void set_colour(int colour);

#define VIM_BLACK        0x0000  /*   0,   0,   0 */
#define VIM_DARK_BLUE    0x000A  /*   0,   0, 160 */
#define VIM_DARK_GREEN   0x0320  /*   0, 128,   0 */
#define VIM_DARK_CYAN    0x033F  /*   0, 128, 128 */
#define VIM_DARK_RED     0x7800  /* 128,   0,   0 */
#define VIM_DARK_MAGENTA 0x780A  /* 128,   0, 160 */
#define VIM_DARK_YELLOW  0x7BA0  /* 128, 128,   0 */
#define VIM_GRAY         0x8410  /* 132, 132, 132 */
#define VIM_LIGHT_GRAY   0xC618  /* 198, 198, 198 */
#define VIM_WHITE        0xFFFF  /* 255, 255, 255 */