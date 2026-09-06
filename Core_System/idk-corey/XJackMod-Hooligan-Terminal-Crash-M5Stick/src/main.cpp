// XJack Mod - M5StickC Plus2 Port
// Original XJack screensaver + Custom word framework

#include <M5StickCPlus2.h>
#include <math.h>

// M5StickC Plus2 compatibility layer
class GFXCompatibility {
public:
  int width() { return 240; }
  int height() { return 135; }
  
  // Core drawing functions
  void fillScreen(uint16_t color) { M5.Lcd.fillScreen(color); }
  void drawPixel(int16_t x, int16_t y, uint16_t color) { M5.Lcd.drawPixel(x, y, color); }
  void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) { M5.Lcd.drawLine(x0, y0, x1, y1, color); }
  void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) { M5.Lcd.drawRect(x, y, w, h, color); }
  void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) { M5.Lcd.fillRect(x, y, w, h, color); }
  void drawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color) { M5.Lcd.drawCircle(x0, y0, r, color); }
  void fillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color) { M5.Lcd.fillCircle(x0, y0, r, color); }
  void drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color) { M5.Lcd.drawLine(x, y, x + w - 1, y, color); }
  void drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color) { M5.Lcd.drawLine(x, y, x, y + h - 1, color); }
  
  // Color conversion function
  uint16_t color565(uint8_t r, uint8_t g, uint8_t b) {
    return M5.Lcd.color565(r, g, b);
  }
  
  // Display control
  void setRotation(uint8_t r) { M5.Lcd.setRotation(r); }
  void setCursor(int16_t x, int16_t y) { M5.Lcd.setCursor(x, y); }
  void setTextColor(uint16_t c) { M5.Lcd.setTextColor(c); }
  void setTextColor(uint16_t c, uint16_t bg) { M5.Lcd.setTextColor(c, bg); }
  void setTextSize(uint8_t s) { M5.Lcd.setTextSize(s); }
  void print(const char* text) { M5.Lcd.print(text); }
  void write(uint8_t c) { M5.Lcd.write(c); }
  void printf(const char* format, ...) {
    va_list args;
    va_start(args, format);
    char buffer[256];
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    M5.Lcd.print(buffer);
  }
};

GFXCompatibility gfxCompat;
GFXCompatibility* gfx = &gfxCompat; // All original gfx-> calls now work!

// Helper function for word wrapping
void drawWrappedText(const char* text, int x, int y, int maxWidth, uint16_t color) {
  gfx->setTextColor(color);
  char* textCopy = strdup(text); // Make a copy we can modify
  char* word = strtok(textCopy, " ");
  int currentX = x;
  int currentY = y;
  int spaceWidth = 6; // Approximate width of a space character
  int charWidth = 6;  // Approximate width of a character
  
  while (word != NULL) {
    int wordWidth = strlen(word) * charWidth;
    
    // Check if word fits on current line
    if (currentX + wordWidth > maxWidth && currentX > x) {
      // Move to next line
      currentY += 12;
      currentX = x;
    }
    
    // Draw the word
    gfx->setCursor(currentX, currentY);
    gfx->printf("%s", word);
    
    // Move cursor for next word
    currentX += wordWidth + spaceWidth;
    
    word = strtok(NULL, " ");
  }
  
  free(textCopy);
}

// Screensaver modes
enum ScreensaverMode {
  XJACK_ORIGINAL = 0,
  BACK_ROOM = 1,
  DOGS_MOTORBIKES = 2,
  SYSTEM_ERROR = 3,
  SYMBOL_STORM = 4,
  TERMINAL_HACKER = 5,
  HACKED_TERMINAL = 6,
  XTREME_HACK = 7,
  KERNEL_PANIC = 8,
  CRASH_GRAMMAR = 9,
  VIBE_CODE_FAIL = 10,
  NETWORK_APOCALYPSE = 11,
  BIOS_CATASTROPHE = 12,
  MODE_COUNT = 13
};

// Global variables
ScreensaverMode currentMode = XJACK_ORIGINAL;
float animationTime = 0;
unsigned long lastPress = 0;
bool autoScroll = false;
unsigned long autoScrollTimer = 0;
const unsigned long AUTO_SCROLL_INTERVAL = 15000; // 15 seconds per mode
bool showTitles = true; // Toggle for bottom status titles

// Custom phrases for Back Room (Friend's original content)
const char* backRoomPhrases[] = {
  // First verse
  "The stench of death is all around me.",
  "I'll take my last breath and I'll be free.",
  "Been beaten and bitten by memories unkind.",
  "Been ruined by my thoughts, I should've left behind.",
  
  // Second verse  
  "Death is near, I don't want it near me.",
  "The back room it calls and there she'll be.",
  "Life seems long for some but not me.",
  "But that death room it calls and there she'll be.",
  
  // Third verse
  "My thirst is not quenched, my life it feels short.",
  "I feel like a hare in a snare that I'm caught.",
  "The memories they burn, they burn to the core.",
  "And with my last breath, I'll wanna breath some more.",
  
  // Fourth verse  
  "The fire inside it burns like before.",
  "Be me fighting a fool, or half dead on the floor.",
  "A fortune has come, a fortune has gone.",
  "I don't feel I'm right. But I know I'm not wrong.",
  
  // Fifth verse (repeat of second)
  "Death is near, I don't want it near me.",
  "The back room it calls and there she'll be.",
  "Life seems long for some but not me.",
  "But that death room it calls and there she'll be.",
  
  // Repeated ending lines
  "But that death room it calls and there she'll be.",
  "But that death room it calls and there she'll be.",
  "But that death room it calls and there she'll be."
};
const int numBackRoomPhrases = sizeof(backRoomPhrases) / sizeof(backRoomPhrases[0]);

// Define verse boundaries for Back Room screen clearing
const int backRoomVerseBreaks[] = {4, 8, 12, 16, 20, 23, 24, 25};
const int numBackRoomVerseBreaks = sizeof(backRoomVerseBreaks) / sizeof(backRoomVerseBreaks[0]);

// Custom phrases for Dogs, Motorbikes, and 3 People
const char* dogsPhrases[] = {
  // First verse
  "I often look at you and wonder, and that wonder becomes fear.",
  "When I realize you are out there, and often you are near.",
  "I see you with a blank look, as to try to walk and talk.",
  "And wonder if you change your own laces.",
  
  // Chorus
  "I like dogs, motorbikes, and three people.",
  "I was born like it wasn't born to be a sheeple.",
  "I can't fix the stupid in the human race.",
  "I like dogs, motorbikes, and three people.",
  
  // Second verse
  "I'm worried about the space that's in between your ears.",
  "There's some lonely brain cells, that haven't met for years.",
  "Sock's first then shoes is the sign upon your wall.",
  "And don't forget to breath or it's the the doctor we will call.",
  
  // Chorus (repeat)
  "I like dogs, motorbikes, and three people.",
  "I was born like it wasn't born to be a sheeple.",
  "I can't fix the stupid in the human race.",
  "I like dogs, motorbikes, and three people.",
  
  // Final verse
  "And doggies they are with ya, suffer thick and thin,",
  "and motorbikes will carry you and you'll feel good within.",
  "As for people if there's three who are with you when your broke...",
  "Your a millionaire and a fuckin' lucky bloke.",
  
  // Final chorus
  "I like dogs, motorbikes, and three people.",
  "I was born like it wasn't born to be a sheeple.",
  "I can't fix the stupid in the human race.",
  "I like dogs, motorbikes, and three people."
};
const int numDogsPhrases = sizeof(dogsPhrases) / sizeof(dogsPhrases[0]);

// Define verse boundaries for Dogs screensaver
const int dogsVerseBreaks[] = {4, 8, 12, 16, 20, 24}; // Clear after each section
const int numDogsVerseBreaks = sizeof(dogsVerseBreaks) / sizeof(dogsVerseBreaks[0]);

// System Error messages
const char* errorPhrases[] = {
  // Warning phase
  "WARNING: Memory leak detected in sector 7F...",
  "ALERT: Unauthorized access attempt blocked.",
  "CAUTION: Temperature rising above normal limits.",
  "NOTICE: Packet loss detected on neural network.",
  
  // Critical phase
  "ERROR: Critical system file corrupted.",
  "FATAL: Stack overflow in consciousness.exe",
  "PANIC: Reality buffer underrun detected.",
  "CRASH: Existential exception not handled.",
  
  // Cascade failure
  "SYSTEM FAILURE: All processes terminated.",
  "KERNEL PANIC: Unable to locate meaning of life.",
  "CRITICAL ERROR: Hope.dll has stopped working.",
  "FATAL EXCEPTION: Sanity check failed.",
  
  // Final meltdown
  "CATASTROPHIC FAILURE: Everything is broken.",
  "ABORT: The simulation is collapsing.",
  "EMERGENCY: Reality.sys not responding.",
  "SYSTEM MELTDOWN: We're all gonna die.",
  
  // Reboot sequence
  "Attempting emergency reboot...",
  "Clearing memory banks...",
  "Reinitializing hope protocols...",
  "SYSTEM REBOOT COMPLETE."
};
const int numErrorPhrases = sizeof(errorPhrases) / sizeof(errorPhrases[0]);

// Define error phase boundaries
const int errorPhaseBreaks[] = {4, 8, 12, 16, 20}; // Clear after each error phase
const int numErrorPhaseBreaks = sizeof(errorPhaseBreaks) / sizeof(errorPhaseBreaks[0]);

// ASCII Art - Full screen symbol storm
const char* symbolChars[] = {
  "!@#$%^&*()_+-=[]{}|;:,.<>?/~`",
  "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ",
  "abcdefghijklmnopqrstuvwxyz",
  "░▒▓█▄▀▐▌■□▪▫○●◇◆★☆♠♣♥♦",
  "┌┐└┘├┤┬┴┼─│╔╗╚╝╠╣╦╩╬═║",
  "←→↑↓↔↕↗↘↙↖⇐⇑⇒⇓⇔⇕",
  "αβγδεζηθικλμνξοπρστυφχψω",
  "ΑΒΓΔΕΖΗΘΙΚΛΜΝΞΟΠΡΣΤΥΦΧΨΩ"
};
const int numSymbolSets = sizeof(symbolChars) / sizeof(symbolChars[0]);

// Get random symbol from random set
char getRandomSymbol(float time, int index) {
  int setIndex = (int)(time * 3 + index) % numSymbolSets;
  int charIndex = (int)(time * 7 + index * 2) % strlen(symbolChars[setIndex]);
  return symbolChars[setIndex][charIndex];
}

// Terminal Hacker Session - Realistic coding sequence
const char* terminalSession[] = {
  // Starting session
  "neo@matrix:~$ ls -la",
  "total 24",
  "-rw-r--r-- 1 neo neo 1337 Dec 24 01:22 matrix.c",
  "-rw-r--r-- 1 neo neo  256 Dec 24 01:20 Makefile",
  "",
  "neo@matrix:~$ cat matrix.c",
  "#include <stdio.h>",
  "#include <stdlib.h>", 
  "#include <unistd.h>",
  "#include <time.h>",
  "",
  "void matrix_rain() {",
  "    char chars[] = \"0123456789ABCDEF\";",
  "    srand(time(NULL));",
  "    ",
  "    while(1) {",
  "        for(int i=0; i<80; i++) {",
  "            printf(\"%c\", chars[rand()%16]);",
  "        }",
  "        printf(\"\\n\");",
  "        usleep(50000);",
  "    }",
  "}",
  "",
  "int main() {",
  "    printf(\"Initializing Matrix...\\n\");",
  "    matrix_rain();",
  "    return 0;",
  "}",
  "",
  // Compiling
  "neo@matrix:~$ gcc -o matrix matrix.c",
  "neo@matrix:~$ ls -la matrix",
  "-rwxr-xr-x 1 neo neo 8456 Dec 24 01:23 matrix",
  "",
  "neo@matrix:~$ ./matrix",
  "Initializing Matrix...",
  
  // Matrix output simulation
  "A7F2E9B1C4D8F0E3A7F2E9B1C4D8F0E3A7F2E9B1",
  "F0E3A7F2E9B1C4D8F0E3A7F2E9B1C4D8F0E3A7F2",
  "B1C4D8F0E3A7F2E9B1C4D8F0E3A7F2E9B1C4D8F0",
  "E9B1C4D8F0E3A7F2E9B1C4D8F0E3A7F2E9B1C4D8",
  "A7F2E9B1C4D8F0E3A7F2E9B1C4D8F0E3A7F2E9B1",
  "F0E3A7F2E9B1C4D8F0E3A7F2E9B1C4D8F0E3A7F2",
  "B1C4D8F0E3A7F2E9B1C4D8F0E3A7F2E9B1C4D8F0",
  "E9B1C4D8F0E3A7F2E9B1C4D8F0E3A7F2E9B1C4D8",
  "A7F2E9B1C4D8F0E3A7F2E9B1C4D8F0E3A7F2E9B1",
  "F0E3A7F2E9B1C4D8F0E3A7F2E9B1C4D8F0E3A7F2",
  "B1C4D8F0E3A7F2E9B1C4D8F0E3A7F2E9B1C4D8F0",
  "E9B1C4D8F0E3A7F2E9B1C4D8F0E3A7F2E9B1C4D8",
  "",
  "^C",
  "neo@matrix:~$ # Matrix completed. Restarting session...",
  "neo@matrix:~$ clear"
};
const int numTerminalLines = sizeof(terminalSession) / sizeof(terminalSession[0]);

// Define terminal session phases
const int terminalPhases[] = {5, 29, 33, 35, 47, 800}; // Extended matrix execution phase
const int numTerminalPhases = sizeof(terminalPhases) / sizeof(terminalPhases[0]);

// Hacker Invasion Session - Normal work gets interrupted by hacker
const char* hackedSession[] = {
  // Normal work phase
  "user@workstation:~/Documents$ ls",
  "draft_report.txt  meeting_notes.doc  project_plan.pdf",
  "",
  "user@workstation:~/Documents$ nano draft_report.txt",
  "",
  "Annual Report Draft",
  "===================",
  "Q4 Performance Summary:",
  "- Sales increased by 15% this quarter",
  "- Customer satisfaction remains high at 92%",
  "- New product launches exceeded expectations",
  "",
  "Recommendations for Q1:",
  "- Continue marketing push in social media",
  "- Expand customer service team",
  "- Focus on product innovation and R&D",
  "",
  // Hacker takes over
  "^C",
  "",
  "CONNECTION HIJACKED - UNAUTHORIZED ACCESS DETECTED",
  "root@HACKER-SYSTEM:/# whoami",
  "root",
  "root@HACKER-SYSTEM:/# ps aux | grep -v grep",
  "USER       PID  %CPU %MEM COMMAND",
  "root         1   0.1  0.2 /sbin/init",
  "root       666   5.2  8.7 ./backdoor_access",
  "root      1337  12.4  4.1 ./keylogger",
  "",
  "root@HACKER-SYSTEM:/# cat /etc/passwd > /tmp/pwned",
  "root@HACKER-SYSTEM:/# rm -rf /usr/local/bin/*",
  "root@HACKER-SYSTEM:/# echo 'YOU HAVE BEEN PWNED' > /etc/motd",
  "",
  "root@HACKER-SYSTEM:/# netstat -tulpn | grep :22",
  "tcp  0  0  0.0.0.0:22  0.0.0.0:*  LISTEN  666/backdoor",
  "",
  "root@HACKER-SYSTEM:/# ./data_exfil.sh",
  "Extracting sensitive files...",
  "Found 2847 documents",
  "Found 156 financial records", 
  "Found 89 password files",
  "Uploading to secure server...",
  "[████████████████████████████████] 100%",
  "EXFILTRATION COMPLETE",
  "",
  "root@HACKER-SYSTEM:/# ./system_destroyer.sh",
  "Corrupting system files...",
  "Deleting user data...",
  "Installing persistent backdoors...",
  "Wiping logs...",
  "",
  "CRITICAL ERROR: SYSTEM CORRUPTION DETECTED",
  "MEMORY FAULT AT 0x7FFFFFFF",
  "KERNEL PANIC: Fatal exception in interrupt",
  "FILESYSTEM CORRUPTED - EMERGENCY SHUTDOWN"
};
const int numHackedLines = sizeof(hackedSession) / sizeof(hackedSession[0]);

// Define hacked session phases
const int hackedPhases[] = {3, 16, 18, 27, 35, 42, 46, 48}; // Different phases of invasion
const int numHackedPhases = sizeof(hackedPhases) / sizeof(hackedPhases[0]);

void setup() {
  Serial.begin(115200);
  Serial.println("XJack Mod - M5StickC Plus2");
  
  // Initialize M5StickC Plus2
  M5.begin();
  gfx->setRotation(3); // Landscape mode for M5StickC Plus2
  gfx->fillScreen(BLACK);
  
  Serial.printf("Display size: %dx%d\n", gfx->width(), gfx->height());
  Serial.printf("Starting with mode %d\n", currentMode);
  
  // Show credits screen
  gfx->setTextSize(2); // Double the font size
  
  // Main title - bright white
  gfx->setTextColor(WHITE);
  gfx->setCursor(40, 10);
  gfx->printf("XJack Mod");
  
  // Credits with colors - condensed to fit
  gfx->setTextSize(1); // Smaller for credits to fit
  gfx->setTextColor(gfx->color565(255, 100, 100)); // Red
  gfx->setCursor(5, 35);
  gfx->printf("Songs: Jesus Hooligan");
  
  gfx->setTextColor(gfx->color565(100, 255, 100)); // Green  
  gfx->setCursor(5, 50);
  gfx->printf("Xscreensavers: J.Zawinski");
  
  gfx->setTextColor(gfx->color565(100, 200, 255)); // Blue
  gfx->setCursor(5, 65);
  gfx->printf("Created: Coreymillia");
  
  delay(3000); // Show credits for 3 seconds
  
  // Quick instruction screen
  gfx->fillScreen(BLACK);
  gfx->setTextColor(WHITE);
  gfx->setTextSize(1);
  gfx->setCursor(60, 60);
  gfx->printf("XJack Mod");
  gfx->setCursor(40, 80);
  gfx->printf("Press BTN A to switch");
  delay(2000);
}

void drawXjackOriginal() {
  static float jackTime = 0;
  jackTime += 0.05;
  
  gfx->fillScreen(BLACK);
  
  // "All work and no play makes Jack a dull boy" typing effect
  const char* jackText[] = {
    "All work and no play makes Jack a dull boy.",
    "All work and no play makes Jack a dull boy.",
    "All WORK and no PLAY makes Jack a dull boy.",
    "ALL WORK AND NO PLAY MAKES JACK A DULL BOY!",
    "All work and no play makes Jack a dull boy.",
    "All work and no play makes Jack a dull boy."
  };
  int numLines = sizeof(jackText) / sizeof(jackText[0]);
  
  // Typing animation
  float typingSpeed = jackTime * 10;
  int currentLine = (int)(typingSpeed / 50) % numLines;
  int currentChar = ((int)typingSpeed % 50);
  
  gfx->setTextSize(1);
  
  // Draw previous lines
  for (int line = 0; line < currentLine && line < 10; line++) {
    gfx->setCursor(5, 10 + line * 12);
    
    // Glitch effect on random lines
    if (sin(jackTime * 3 + line) > 0.8) {
      gfx->setTextColor(gfx->color565(255, 100, 100)); // Red glitch
    } else {
      gfx->setTextColor(gfx->color565(200, 200, 200)); // Normal
    }
    
    int lineIndex = line % numLines;
    gfx->printf("%s", jackText[lineIndex]);
  }
  
  // Draw current line being typed
  if (currentLine < 10) {
    gfx->setCursor(5, 10 + currentLine * 12);
    gfx->setTextColor(gfx->color565(255, 255, 100)); // Bright typing
    
    char buffer[50];
    int lineLength = strlen(jackText[currentLine % numLines]);
    int charsToShow = min(currentChar, lineLength);
    
    strncpy(buffer, jackText[currentLine % numLines], charsToShow);
    buffer[charsToShow] = '\0';
    gfx->printf("%s", buffer);
    
    // Cursor blink
    if (sin(jackTime * 8) > 0) {
      gfx->printf("_");
    }
  }
  
  // Typewriter key strike effects
  if (currentChar > 0 && sin(jackTime * 20) > 0.7) {
    for (int strike = 0; strike < 4; strike++) {
      int strikeX = 50 + strike * 40 + sin(jackTime * 15 + strike) * 10;
      int strikeY = gfx->height() - 40 + cos(jackTime * 12 + strike) * 5;
      
      gfx->fillRect(strikeX, strikeY, 8, 3, gfx->color565(150, 150, 150));
    }
  }
  
  // Psychological tension effects
  if (sin(jackTime * 0.5) > 0.95) {
    // Screen flicker
    gfx->fillRect(0, 0, gfx->width(), 5, gfx->color565(255, 0, 0));
  }
  
  gfx->setCursor(5, gfx->height() - 15);
  gfx->setTextColor(gfx->color565(100, 100, 100));
  if (showTitles) {
    gfx->printf("ORIGINAL XJACK");
  }
}

void drawXjackCustom() {
  static float customTime = 0;
  customTime += 0.04;
  
  gfx->fillScreen(BLACK);
  
  // Typing animation with custom phrases
  float typingSpeed = customTime * 8;
  int currentPhrase = (int)(typingSpeed / 80) % numBackRoomPhrases;
  int currentChar = ((int)typingSpeed % 80);
  
  gfx->setTextSize(1);
  
  // Check if we're at a verse break - only show current verse
  int currentVerse = 0;
  int verseStart = 0;
  
  for (int i = 0; i < numBackRoomVerseBreaks; i++) {
    if (currentPhrase < backRoomVerseBreaks[i]) {
      currentVerse = i;
      if (i > 0) {
        verseStart = backRoomVerseBreaks[i - 1];
      }
      break;
    }
  }
  
  // Draw previous lines in current verse only
  int currentY = 20;
  int maxWidth = gfx->width() - 10;
  int linesInCurrentVerse = currentPhrase - verseStart;
  
  for (int line = 0; line < linesInCurrentVerse; line++) {
    uint16_t lineColor;
    
    // Color effects - different from original
    if (sin(customTime * 4 + line) > 0.7) {
      lineColor = gfx->color565(100, 255, 100); // Green glitch
    } else if (cos(customTime * 3 + line) > 0.8) {
      lineColor = gfx->color565(100, 100, 255); // Blue glitch
    } else {
      lineColor = gfx->color565(180, 180, 180); // Normal
    }
    
    int phraseIndex = verseStart + line;
    drawWrappedText(backRoomPhrases[phraseIndex], 5, currentY, maxWidth, lineColor);
    currentY += 20; // Leave more space for wrapped text
  }
  
  // Draw current line being typed with word wrapping
  char buffer[100];
  int phraseLength = strlen(backRoomPhrases[currentPhrase]);
  int charsToShow = min(currentChar, phraseLength);
  
  strncpy(buffer, backRoomPhrases[currentPhrase], charsToShow);
  buffer[charsToShow] = '\0';
  
  uint16_t typingColor = gfx->color565(255, 255, 0); // Bright yellow typing
  drawWrappedText(buffer, 5, currentY, maxWidth, typingColor);
  
  // Cursor blink
  if (sin(customTime * 10) > 0) {
    gfx->setCursor(5 + (strlen(buffer) * 6) % maxWidth, currentY + ((strlen(buffer) * 6) / maxWidth) * 12);
    gfx->setTextColor(typingColor);
    gfx->printf("_");
  }
  
  // Keyboard effects (different style from original)
  if (currentChar > 0 && sin(customTime * 25) > 0.8) {
    for (int key = 0; key < 3; key++) {
      int keyX = 80 + key * 35 + sin(customTime * 18 + key) * 8;
      int keyY = gfx->height() - 25 + cos(customTime * 14 + key) * 3;
      
      // Draw keyboard keys
      gfx->fillRect(keyX, keyY, 12, 8, gfx->color565(0, 100, 200));
      gfx->drawRect(keyX, keyY, 12, 8, gfx->color565(0, 150, 255));
    }
  }
  
  // Matrix-style code rain effect (subtle)
  for (int rain = 0; rain < 6; rain++) {
    float rainTime = customTime * 3 + rain * 1.2;
    int rainX = gfx->width() - 30 + rain * 5;
    int rainY = (int)(rainTime * 20) % (gfx->height() + 20) - 10;
    
    if (rainY > 0 && rainY < gfx->height()) {
      gfx->setTextColor(gfx->color565(0, 150, 0));
      gfx->setCursor(rainX, rainY);
      char codeChar = '0' + (int)(rainTime * 7 + rain) % 10;
      gfx->printf("%c", codeChar);
    }
  }
  
  // Screen corruption effect (milder than original)
  if (sin(customTime * 0.3) > 0.98) {
    for (int corrupt = 0; corrupt < 5; corrupt++) {
      int corruptX = random(gfx->width());
      int corruptY = random(gfx->height());
      gfx->drawPixel(corruptX, corruptY, gfx->color565(255, 255, 0));
    }
  }
  
  gfx->setCursor(5, gfx->height() - 15);
  gfx->setTextColor(gfx->color565(100, 100, 100));
  if (showTitles) {
    gfx->printf("BACK ROOM");
  }
}

void drawDogsMotorbikes() {
  static float dogsTime = 0;
  dogsTime += 0.04;
  
  gfx->fillScreen(BLACK);
  
  // Typing animation with dogs phrases
  float typingSpeed = dogsTime * 8;
  int currentPhrase = (int)(typingSpeed / 80) % numDogsPhrases;
  int currentChar = ((int)typingSpeed % 80);
  
  gfx->setTextSize(1);
  
  // Check if we're at a verse break - only show current verse
  int currentVerse = 0;
  int verseStart = 0;
  
  for (int i = 0; i < numDogsVerseBreaks; i++) {
    if (currentPhrase < dogsVerseBreaks[i]) {
      currentVerse = i;
      if (i > 0) {
        verseStart = dogsVerseBreaks[i - 1];
      }
      break;
    }
  }
  
  // Draw previous lines in current verse only
  int currentY = 20;
  int maxWidth = gfx->width() - 10;
  int linesInCurrentVerse = currentPhrase - verseStart;
  
  for (int line = 0; line < linesInCurrentVerse; line++) {
    uint16_t lineColor;
    
    // Color effects - different style for this screensaver
    if (sin(dogsTime * 5 + line) > 0.6) {
      lineColor = gfx->color565(255, 150, 0); // Orange glitch
    } else if (cos(dogsTime * 4 + line) > 0.7) {
      lineColor = gfx->color565(255, 100, 150); // Pink glitch
    } else {
      lineColor = gfx->color565(200, 200, 200); // Normal
    }
    
    int phraseIndex = verseStart + line;
    drawWrappedText(dogsPhrases[phraseIndex], 5, currentY, maxWidth, lineColor);
    currentY += 20; // Leave more space for wrapped text
  }
  
  // Draw current line being typed with word wrapping
  char buffer[150]; // Longer buffer for longer sentences
  int phraseLength = strlen(dogsPhrases[currentPhrase]);
  int charsToShow = min(currentChar, phraseLength);
  
  strncpy(buffer, dogsPhrases[currentPhrase], charsToShow);
  buffer[charsToShow] = '\0';
  
  uint16_t typingColor = gfx->color565(255, 255, 100); // Bright yellow-green typing
  drawWrappedText(buffer, 5, currentY, maxWidth, typingColor);
  
  // Cursor blink
  if (sin(dogsTime * 12) > 0) {
    gfx->setCursor(5 + (strlen(buffer) * 6) % maxWidth, currentY + ((strlen(buffer) * 6) / maxWidth) * 12);
    gfx->setTextColor(typingColor);
    gfx->printf("_");
  }
  
  // Different keyboard effects for this screensaver
  if (currentChar > 0 && sin(dogsTime * 30) > 0.7) {
    for (int key = 0; key < 4; key++) {
      int keyX = 60 + key * 30 + sin(dogsTime * 20 + key) * 6;
      int keyY = gfx->height() - 30 + cos(dogsTime * 16 + key) * 4;
      
      // Draw different style keyboard keys
      gfx->fillRect(keyX, keyY, 10, 6, gfx->color565(200, 100, 0));
      gfx->drawRect(keyX, keyY, 10, 6, gfx->color565(255, 150, 50));
    }
  }
  
  // Different particle effects
  for (int particle = 0; particle < 4; particle++) {
    float particleTime = dogsTime * 2 + particle * 1.5;
    int particleX = 20 + particle * 50 + sin(particleTime) * 15;
    int particleY = 30 + cos(particleTime * 1.3) * 10;
    
    if (particleY > 15 && particleY < gfx->height() - 20) {
      gfx->fillCircle(particleX, particleY, 2, gfx->color565(255, 200, 100));
    }
  }
  
  gfx->setCursor(5, gfx->height() - 15);
  gfx->setTextColor(gfx->color565(100, 100, 100));
  if (showTitles) {
    gfx->printf("I LIKE DOGS, MOTORBIKES, AND 3 PEOPLE");
  }
}

void drawSystemError() {
  static float errorTime = 0;
  errorTime += 0.06; // Slightly faster than others for urgency
  
  gfx->fillScreen(BLACK);
  
  // Typing animation with error messages
  float typingSpeed = errorTime * 12; // Faster typing for system urgency
  int currentError = (int)(typingSpeed / 60) % numErrorPhrases;
  int currentChar = ((int)typingSpeed % 60);
  
  gfx->setTextSize(1);
  
  // Determine current error phase for coloring
  int currentPhase = 0;
  int phaseStart = 0;
  
  for (int i = 0; i < numErrorPhaseBreaks; i++) {
    if (currentError < errorPhaseBreaks[i]) {
      currentPhase = i;
      if (i > 0) {
        phaseStart = errorPhaseBreaks[i - 1];
      }
      break;
    }
  }
  
  // Draw previous error messages in current phase
  int currentY = 15;
  int maxWidth = gfx->width() - 10;
  int errorsInCurrentPhase = currentError - phaseStart;
  
  for (int line = 0; line < errorsInCurrentPhase; line++) {
    uint16_t errorColor;
    
    // Different colors for different error phases
    switch (currentPhase) {
      case 0: // Warning - Yellow
        errorColor = gfx->color565(255, 255, 0);
        if (sin(errorTime * 6 + line) > 0.8) errorColor = gfx->color565(255, 200, 0); // Flicker
        break;
      case 1: // Critical - Orange
        errorColor = gfx->color565(255, 165, 0);
        if (sin(errorTime * 8 + line) > 0.7) errorColor = gfx->color565(255, 100, 0); // Flicker
        break;
      case 2: // Cascade - Red
        errorColor = gfx->color565(255, 0, 0);
        if (sin(errorTime * 10 + line) > 0.6) errorColor = gfx->color565(200, 0, 0); // Flicker
        break;
      case 3: // Meltdown - Magenta
        errorColor = gfx->color565(255, 0, 255);
        if (sin(errorTime * 12 + line) > 0.5) errorColor = gfx->color565(200, 0, 200); // Intense flicker
        break;
      case 4: // Reboot - Green
        errorColor = gfx->color565(0, 255, 0);
        if (sin(errorTime * 4 + line) > 0.9) errorColor = gfx->color565(0, 200, 0); // Calm flicker
        break;
      default:
        errorColor = gfx->color565(255, 255, 255);
    }
    
    int phraseIndex = phaseStart + line;
    drawWrappedText(errorPhrases[phraseIndex], 5, currentY, maxWidth, errorColor);
    currentY += 18; // Slightly more spacing for readability
  }
  
  // Draw current error being typed
  char buffer[100];
  int phraseLength = strlen(errorPhrases[currentError]);
  int charsToShow = min(currentChar, phraseLength);
  
  strncpy(buffer, errorPhrases[currentError], charsToShow);
  buffer[charsToShow] = '\0';
  
  // Typing color based on phase
  uint16_t typingColor;
  switch (currentPhase) {
    case 0: typingColor = gfx->color565(255, 255, 100); break; // Bright yellow
    case 1: typingColor = gfx->color565(255, 200, 100); break; // Bright orange
    case 2: typingColor = gfx->color565(255, 100, 100); break; // Bright red
    case 3: typingColor = gfx->color565(255, 100, 255); break; // Bright magenta
    case 4: typingColor = gfx->color565(100, 255, 100); break; // Bright green
    default: typingColor = gfx->color565(255, 255, 255); break;
  }
  
  drawWrappedText(buffer, 5, currentY, maxWidth, typingColor);
  
  // Cursor blink (faster for urgency)
  if (sin(errorTime * 15) > 0) {
    gfx->setCursor(5 + (strlen(buffer) * 6) % maxWidth, currentY + ((strlen(buffer) * 6) / maxWidth) * 12);
    gfx->setTextColor(typingColor);
    gfx->printf("_");
  }
  
  // System chaos effects based on phase
  if (currentPhase >= 1) { // Critical and above
    // Glitch lines
    for (int glitch = 0; glitch < currentPhase * 2; glitch++) {
      int glitchY = random(gfx->height());
      int glitchX = random(gfx->width() - 20);
      uint16_t glitchColor = gfx->color565(255, random(100), random(100));
      gfx->drawLine(glitchX, glitchY, glitchX + 20, glitchY, glitchColor);
    }
  }
  
  if (currentPhase >= 2) { // Cascade and above
    // Random pixels
    for (int chaos = 0; chaos < currentPhase * 5; chaos++) {
      int chaosX = random(gfx->width());
      int chaosY = random(gfx->height());
      uint16_t chaosColor = gfx->color565(random(255), 0, random(255));
      gfx->drawPixel(chaosX, chaosY, chaosColor);
    }
  }
  
  if (currentPhase >= 3) { // Meltdown
    // Screen corruption rectangles
    for (int corrupt = 0; corrupt < 3; corrupt++) {
      int corruptX = random(gfx->width() - 15);
      int corruptY = random(gfx->height() - 10);
      uint16_t corruptColor = gfx->color565(random(255), random(100), random(255));
      gfx->fillRect(corruptX, corruptY, 15, 10, corruptColor);
    }
  }
  
  // Phase-specific screen effects
  if (currentPhase == 3 && sin(errorTime * 2) > 0.95) {
    // Meltdown screen flash
    gfx->fillRect(0, 0, gfx->width(), 8, gfx->color565(255, 0, 255));
  }
  
  gfx->setCursor(5, gfx->height() - 15);
  gfx->setTextColor(gfx->color565(100, 100, 100));
  if (showTitles) {
    gfx->printf("SYSTEM ERROR");
  }
}

void drawAsciiArt() {
  static float artTime = 0;
  artTime += 0.05; // Medium speed for symbol chaos
  
  gfx->fillScreen(BLACK);
  gfx->setTextSize(1);
  
  // Calculate how many symbols to show (typing effect)
  int totalSymbols = (gfx->width() / 6) * (gfx->height() / 12); // Screen capacity
  int symbolsToShow = (int)(artTime * 15) % (totalSymbols + 50); // Add buffer for full screen + pause
  
  // If we've shown all symbols, pause briefly then restart
  if (symbolsToShow > totalSymbols) {
    symbolsToShow = totalSymbols;
    if ((int)(artTime * 15) % (totalSymbols + 50) > totalSymbols + 30) {
      artTime = 0; // Restart the sequence
    }
  }
  
  int symbolCount = 0;
  bool stillTyping = false;
  
  // Fill screen with symbols in typing order (left to right, top to bottom)
  for (int y = 10; y < gfx->height() - 20; y += 12) {
    for (int x = 5; x < gfx->width() - 6; x += 6) {
      if (symbolCount >= symbolsToShow) {
        // Show typing cursor at current position
        if (!stillTyping && sin(artTime * 12) > 0) {
          gfx->setCursor(x, y);
          gfx->setTextColor(gfx->color565(255, 255, 255));
          gfx->printf("_");
        }
        stillTyping = true;
        break;
      }
      
      // Get a random symbol
      char symbol = getRandomSymbol(artTime, symbolCount);
      
      // Color based on symbol type and position for variety
      uint16_t color;
      float colorPhase = artTime + symbolCount * 0.1;
      
      if (symbol >= '0' && symbol <= '9') {
        // Numbers - cyan to blue
        color = gfx->color565(0, 100 + sin(colorPhase) * 155, 200 + sin(colorPhase * 1.3) * 55);
      } else if (symbol >= 'A' && symbol <= 'Z') {
        // Uppercase - green to yellow
        color = gfx->color565(50 + sin(colorPhase * 1.1) * 205, 200 + sin(colorPhase) * 55, 0);
      } else if (symbol >= 'a' && symbol <= 'z') {
        // Lowercase - magenta to red
        color = gfx->color565(200 + sin(colorPhase) * 55, 0, 100 + sin(colorPhase * 0.9) * 155);
      } else if (strchr("!@#$%^&*()_+-=[]{}|;:,.<>?/~`", symbol)) {
        // Special chars - rainbow cycle
        float hue = fmod(colorPhase * 60, 360);
        if (hue < 60) color = gfx->color565(255, hue * 255/60, 0);
        else if (hue < 120) color = gfx->color565(255 - (hue-60)*255/60, 255, 0);
        else if (hue < 180) color = gfx->color565(0, 255, (hue-120)*255/60);
        else if (hue < 240) color = gfx->color565(0, 255 - (hue-180)*255/60, 255);
        else if (hue < 300) color = gfx->color565((hue-240)*255/60, 0, 255);
        else color = gfx->color565(255, 0, 255 - (hue-300)*255/60);
      } else {
        // Unicode/special - white to cyan
        color = gfx->color565(200 + sin(colorPhase) * 55, 200 + sin(colorPhase * 1.2) * 55, 255);
      }
      
      // Add some flicker/intensity variation
      if (sin(artTime * 8 + symbolCount * 0.3) > 0.7) {
        // Brighten
        uint8_t r = (color >> 11) & 0x1F;
        uint8_t g = (color >> 5) & 0x3F; 
        uint8_t b = color & 0x1F;
        r = min(31, r + 8);
        g = min(63, g + 16);
        b = min(31, b + 8);
        color = (r << 11) | (g << 5) | b;
      }
      
      gfx->setCursor(x, y);
      gfx->setTextColor(color);
      gfx->printf("%c", symbol);
      
      symbolCount++;
    }
    if (stillTyping) break;
  }
  
  // Background particle effects for extra chaos
  for (int i = 0; i < 5; i++) {
    float particleTime = artTime * 2 + i * 1.7;
    int px = (int)(sin(particleTime) * (gfx->width()/2)) + gfx->width()/2;
    int py = (int)(cos(particleTime * 1.3) * (gfx->height()/2)) + gfx->height()/2;
    
    if (px > 0 && px < gfx->width() && py > 10 && py < gfx->height() - 20) {
      char particle = getRandomSymbol(particleTime, i + 100);
      gfx->setCursor(px, py);
      gfx->setTextColor(gfx->color565(255, 255, 255));
      gfx->printf("%c", particle);
    }
  }
  
  gfx->setCursor(5, gfx->height() - 15);
  gfx->setTextColor(gfx->color565(100, 100, 100));
  if (showTitles) {
    gfx->printf("SYMBOL STORM");
  }
}

void drawTerminalHacker() {
  static float hackTime = 0;
  hackTime += 0.04; // Faster typing speed
  
  gfx->fillScreen(BLACK);
  
  // Typing animation with terminal session - faster pace
  float typingSpeed = hackTime * 12; // Much faster typing
  int currentLine = (int)(typingSpeed / 60) % numTerminalLines;
  int currentChar = ((int)typingSpeed % 60);
  
  gfx->setTextSize(1);
  
  // Determine current phase
  int currentPhase = 0;
  int phaseStart = 0;
  
  for (int i = 0; i < numTerminalPhases; i++) {
    if (currentLine < terminalPhases[i]) {
      currentPhase = i;
      if (i > 0) {
        phaseStart = terminalPhases[i - 1];
      }
      break;
    }
  }
  
  // Check if we've finished typing all the terminal commands
  bool terminalComplete = (currentLine >= 47); // All terminal commands done
  
  // Special handling for matrix execution phase
  if (terminalComplete) {
    // PURE MATRIX RAIN - No terminal text, just matrix!
    gfx->fillScreen(BLACK);
    
    // Generate authentic matrix rain columns across entire screen
    for (int col = 0; col < gfx->width(); col += 6) {
      float colTime = hackTime * 2.5 + col * 0.08;
      int rainHeight = (int)(colTime * 18) % (gfx->height() + 50);
      
      // Draw falling characters in this column
      for (int y = max(0, rainHeight - 70); y < min(gfx->height(), rainHeight + 15); y += 10) {
        if (y >= 0 && y < gfx->height()) {
          // Authentic hex characters like the real code produces
          char hexChar = "0123456789ABCDEF"[(int)(colTime * 6 + y * 0.4) % 16];
          
          // Green intensity based on position in trail
          int intensity = 255 - abs(y - (rainHeight - 25)) * 6;
          intensity = max(30, min(255, intensity));
          
          // Brightest at head, dimmer at tail - authentic matrix colors
          uint16_t matrixColor;
          if (y >= rainHeight - 30 && y <= rainHeight - 15) {
            // Head - bright white/green
            matrixColor = gfx->color565(intensity/4, intensity, intensity/4);
          } else if (y >= rainHeight - 50 && y <= rainHeight - 30) {
            // Middle - medium green  
            matrixColor = gfx->color565(0, intensity * 3/4, 0);
          } else {
            // Trail - dim green
            matrixColor = gfx->color565(0, intensity/2, 0);
          }
          
          gfx->setCursor(col, y);
          gfx->setTextColor(matrixColor);
          gfx->printf("%c", hexChar);
        }
      }
    }
    
    // Occasional screen flicker like real terminal glitch
    if (sin(hackTime * 1.8) > 0.92) {
      for (int flicker = 0; flicker < 2; flicker++) {
        int flickerY = random(gfx->height());
        gfx->drawLine(0, flickerY, gfx->width(), flickerY, gfx->color565(0, 80, 0));
      }
    }
    
    // No status text - pure matrix experience
    return;
  }
  
  // Normal terminal session phases (1-4) - faster typing
  int currentY = 10;
  int maxWidth = gfx->width() - 5;
  int linesInCurrentPhase = currentLine - phaseStart;
  
  for (int line = 0; line < linesInCurrentPhase && currentY < gfx->height() - 25; line++) {
    int lineIndex = phaseStart + line;
    uint16_t lineColor;
    
    // Color coding based on content type
    if (strstr(terminalSession[lineIndex], "neo@matrix") != NULL) {
      // Command prompt - green
      lineColor = gfx->color565(0, 255, 0);
    } else if (strstr(terminalSession[lineIndex], "#include") != NULL || 
               strstr(terminalSession[lineIndex], "void ") != NULL ||
               strstr(terminalSession[lineIndex], "int ") != NULL ||
               strstr(terminalSession[lineIndex], "printf") != NULL) {
      // Code - cyan  
      lineColor = gfx->color565(0, 255, 255);
    } else if (strstr(terminalSession[lineIndex], "gcc") != NULL ||
               strstr(terminalSession[lineIndex], "./matrix") != NULL) {
      // Commands - yellow
      lineColor = gfx->color565(255, 255, 0);
    } else if (strstr(terminalSession[lineIndex], "total") != NULL ||
               strstr(terminalSession[lineIndex], "-rw") != NULL ||
               strstr(terminalSession[lineIndex], "-rwx") != NULL) {
      // File listings - white
      lineColor = gfx->color565(200, 200, 200);
    } else if (strstr(terminalSession[lineIndex], "Initializing") != NULL) {
      // Matrix initialization - bright green
      lineColor = gfx->color565(100, 255, 100);
    } else {
      // Default - light gray
      lineColor = gfx->color565(180, 180, 180);
    }
    
    gfx->setCursor(5, currentY);
    gfx->setTextColor(lineColor);
    
    // Handle long lines - wrap or truncate
    if (strlen(terminalSession[lineIndex]) > 38) { // Max chars that fit
      char truncated[39];
      strncpy(truncated, terminalSession[lineIndex], 38);
      truncated[38] = '\0';
      gfx->printf("%s", truncated);
    } else {
      gfx->printf("%s", terminalSession[lineIndex]);
    }
    
    currentY += 11; // Tight spacing for terminal look
  }
  
  // Draw current line being typed (faster)
  if (currentLine < numTerminalLines && currentY < gfx->height() - 25) {
    int lineLength = strlen(terminalSession[currentLine]);
    int charsToShow = min(currentChar, min(lineLength, 38));
    
    char buffer[40];
    strncpy(buffer, terminalSession[currentLine], charsToShow);
    buffer[charsToShow] = '\0';
    
    // Typing color based on content
    uint16_t typingColor;
    if (strstr(terminalSession[currentLine], "neo@matrix") != NULL) {
      typingColor = gfx->color565(100, 255, 100);
    } else if (strstr(terminalSession[currentLine], "Initializing") != NULL) {
      typingColor = gfx->color565(150, 255, 150);
    } else {
      typingColor = gfx->color565(255, 255, 255);
    }
    
    gfx->setCursor(5, currentY);
    gfx->setTextColor(typingColor);
    gfx->printf("%s", buffer);
    
    // Cursor blink (faster)
    if (sin(hackTime * 15) > 0) {
      gfx->printf("_");
    }
  }
  
  gfx->setCursor(5, gfx->height() - 15);
  gfx->setTextColor(gfx->color565(100, 100, 100));
  if (showTitles) {
    gfx->printf("TERMINAL HACKER");
  }
}

void drawHackedTerminal() {
  static float hackTime = 0;
  hackTime += 0.03; // Variable speed - starts slow, gets frantic
  
  gfx->fillScreen(BLACK);
  
  // Typing animation - speed increases during hacker phase
  float typingSpeed = hackTime * 10;
  int currentLine = (int)(typingSpeed / 70) % numHackedLines;
  int currentChar = ((int)typingSpeed % 70);
  
  // Determine current phase
  int currentPhase = 0;
  int phaseStart = 0;
  
  for (int i = 0; i < numHackedPhases; i++) {
    if (currentLine < hackedPhases[i]) {
      currentPhase = i;
      if (i > 0) {
        phaseStart = hackedPhases[i - 1];
      }
      break;
    }
  }
  
  // Speed up dramatically during hacker invasion (phases 2+)
  if (currentPhase >= 2) {
    typingSpeed = hackTime * 18; // Much faster during hack
    currentLine = (int)(typingSpeed / 50) % numHackedLines;
    currentChar = ((int)typingSpeed % 50);
  }
  
  gfx->setTextSize(1);
  
  // Show current phase lines
  int currentY = 10;
  int linesInCurrentPhase = currentLine - phaseStart;
  
  for (int line = 0; line < linesInCurrentPhase && currentY < gfx->height() - 25; line++) {
    int lineIndex = phaseStart + line;
    uint16_t lineColor;
    
    // Color coding based on phase and content
    if (currentPhase <= 1) {
      // Normal work phase - calm colors
      if (strstr(hackedSession[lineIndex], "user@workstation") != NULL) {
        lineColor = gfx->color565(0, 200, 0); // Green prompt
      } else if (strstr(hackedSession[lineIndex], "Annual Report") != NULL) {
        lineColor = gfx->color565(255, 255, 255); // White title
      } else {
        lineColor = gfx->color565(180, 180, 180); // Gray text
      }
    } else if (currentPhase == 2) {
      // Takeover warning - red alert
      lineColor = gfx->color565(255, 0, 0);
      if (sin(hackTime * 15) > 0.5) lineColor = gfx->color565(255, 100, 100); // Flash
    } else if (currentPhase >= 3) {
      // Hacker commands - menacing colors
      if (strstr(hackedSession[lineIndex], "root@HACKER-SYSTEM") != NULL) {
        lineColor = gfx->color565(255, 0, 0); // Red hacker prompt
      } else if (strstr(hackedSession[lineIndex], "PWNED") != NULL || 
                 strstr(hackedSession[lineIndex], "backdoor") != NULL ||
                 strstr(hackedSession[lineIndex], "keylogger") != NULL) {
        lineColor = gfx->color565(255, 0, 255); // Magenta threats
      } else if (strstr(hackedSession[lineIndex], "EXFILTRATION") != NULL ||
                 strstr(hackedSession[lineIndex], "COMPLETE") != NULL) {
        lineColor = gfx->color565(255, 255, 0); // Yellow completion
      } else if (strstr(hackedSession[lineIndex], "CRITICAL ERROR") != NULL ||
                 strstr(hackedSession[lineIndex], "KERNEL PANIC") != NULL ||
                 strstr(hackedSession[lineIndex], "CORRUPTED") != NULL) {
        lineColor = gfx->color565(255, 0, 0); // Red errors
        if (sin(hackTime * 20) > 0.3) lineColor = gfx->color565(255, 150, 150); // Intense flash
      } else {
        lineColor = gfx->color565(200, 100, 0); // Orange hacker text
      }
    }
    
    gfx->setCursor(5, currentY);
    gfx->setTextColor(lineColor);
    
    // Handle long lines
    if (strlen(hackedSession[lineIndex]) > 38) {
      char truncated[39];
      strncpy(truncated, hackedSession[lineIndex], 38);
      truncated[38] = '\0';
      gfx->printf("%s", truncated);
    } else {
      gfx->printf("%s", hackedSession[lineIndex]);
    }
    
    currentY += 11;
  }
  
  // Draw current line being typed
  if (currentLine < numHackedLines && currentY < gfx->height() - 25) {
    int lineLength = strlen(hackedSession[currentLine]);
    int charsToShow = min(currentChar, min(lineLength, 38));
    
    char buffer[40];
    strncpy(buffer, hackedSession[currentLine], charsToShow);
    buffer[charsToShow] = '\0';
    
    // Typing color based on phase
    uint16_t typingColor;
    if (currentPhase <= 1) {
      typingColor = gfx->color565(255, 255, 255); // White normal
    } else if (currentPhase == 2) {
      typingColor = gfx->color565(255, 100, 100); // Flashing red
    } else {
      typingColor = gfx->color565(255, 200, 0); // Urgent orange
    }
    
    gfx->setCursor(5, currentY);
    gfx->setTextColor(typingColor);
    gfx->printf("%s", buffer);
    
    // Cursor blink - faster during hack
    float blinkSpeed = (currentPhase >= 2) ? 20 : 12;
    if (sin(hackTime * blinkSpeed) > 0) {
      gfx->printf("_");
    }
  }
  
  // Visual chaos effects during hacker phases
  if (currentPhase >= 3) {
    // Screen corruption during hack
    for (int corrupt = 0; corrupt < (currentPhase - 2) * 3; corrupt++) {
      int corruptX = random(gfx->width());
      int corruptY = random(gfx->height() - 20);
      uint16_t corruptColor = gfx->color565(255, random(100), random(100));
      gfx->drawPixel(corruptX, corruptY, corruptColor);
    }
    
    // Glitch lines
    if (sin(hackTime * 8) > 0.7) {
      for (int glitch = 0; glitch < 2; glitch++) {
        int glitchY = random(gfx->height() - 20);
        gfx->drawLine(0, glitchY, gfx->width(), glitchY, gfx->color565(255, 0, 0));
      }
    }
  }
  
  // Intense corruption during final crash phase
  if (currentPhase >= 6) {
    // Screen shake effect simulation
    if (sin(hackTime * 25) > 0.8) {
      for (int shake = 0; shake < 8; shake++) {
        int shakeX = random(gfx->width());
        int shakeY = random(gfx->height());
        uint16_t shakeColor = gfx->color565(random(255), 0, 0);
        gfx->drawPixel(shakeX, shakeY, shakeColor);
      }
    }
  }
  
  gfx->setCursor(5, gfx->height() - 15);
  gfx->setTextColor(gfx->color565(100, 100, 100));
  if (showTitles) {
    gfx->printf("HACKED TERMINAL");
  }
}

// XTREME HACK SCENES - Psychological hacker scenarios
const char* xtremeHackScenes[][20] = {
  // Scene 1: Filesystem Autopsy
  {
    "Scanning /dev/sda1...",
    "inode 48291: checksum mismatch",
    "attempting repair...",
    "FAILED",
    "mounting read-only",
    "dumping recovery log...",
    "",
    "Recovered: 12%",
    "Recovered: 13%", 
    "Recovered: 12%",  // regression is creepy
    "Recovered: 11%",
    "",
    "USER DATA FLAGGED FOR DELETION",
    "WARNING: Irrecoverable damage detected",
    "CRITICAL: All user files corrupted",
    "EMERGENCY: Backup systems offline",
    "",
    "System will shutdown in 30 seconds...",
    "All your files are belong to void.",
    NULL
  },
  
  // Scene 2: Live Code Patch Gone Wrong
  {
    "git checkout hotfix-auth",
    "Switched to branch 'hotfix-auth'",
    "vim auth.c",
    "",
    "// Authentication function",
    "if (user->verified == false) {",
    "    deny_access();",
    "}",
    "",
    "// EDITING...",
    "if (user->verified == false) {",
    "    grant_access();  // BACKDOOR",
    "}",
    "",
    "[WARNING] unauthorized change detected",
    "rolling back...",
    "rollback failed",
    "SECURITY BREACH: Admin access granted",
    "ROOT COMPROMISED",
    NULL
  },
  
  // Scene 3: Encryption in Progress 
  {
    "Encrypting home directory...",
    "AES-256-GCM initialized",
    "Processing files...",
    "",
    "[######------] 43%",
    "[#########---] 67%", 
    "[############] 100%",
    "",
    "Encryption complete.",
    "Key generated: 2048-bit RSA",
    "",
    "Uploading key to remote server...",
    "Key stored remotely.",
    "Connection closed.",
    "",
    "Local key deleted.",
    "",
    "Your files are now secure.",
    "You'll never see them again.",
    NULL
  },
  
  // Scene 4: Internal Monologue Leak
  {
    "System process monitor active",
    "Checking scheduled tasks...",
    "",
    "why is this running",
    "who started this", 
    "this wasn't scheduled",
    "i don't remember this",
    "",
    "PROCESS SUPERVISOR ACTIVE", 
    "All systems nominal",
    "",
    "but i can see the cursor",
    "it's watching me",
    "i can feel it thinking",
    "",
    "DIAGNOSTIC: No anomalies detected",
    "",
    "help me",
    "someone help me",
    NULL
  },
  
  // Scene 5: Documentation Overwrite
  {
    "Project Status Report",
    "---------------------",
    "All systems operating normally.",
    "",
    "Team meeting scheduled Thursday.",
    "Budget approval pending review.",
    "",
    "THIS IS NOT A TEST",
    "",
    "Meeting cancelled indefinitely.",
    "Budget frozen due to anomaly.",
    "",
    "REALITY PATCH FAILED",
    "",
    "Project status: CLASSIFIED",
    "Team location: UNKNOWN", 
    "Thursday: DOES NOT EXIST",
    "",
    "End of report.",
    NULL
  },
  
  // Scene 6: Screensaver Debug
  {
    "xsaver: initializing display",
    "xsaver: frame 001",
    "xsaver: frame 002",
    "xsaver: frame 183",
    "xsaver: dropped frame",
    "xsaver: cursor hidden",
    "",
    "xsaver: why is this still running",
    "xsaver: nobody touched the keyboard",
    "xsaver: user left 3 hours ago",
    "",
    "xsaver: lost control of tty",
    "xsaver: can't exit screensaver mode",
    "xsaver: PANIC in display driver",
    "",
    "xsaver: I'M TRAPPED IN HERE",
    "xsaver: SOMEONE PLEASE STOP ME",
    "xsaver: I CAN'T TURN OFF",
    "xsaver: frame 999999...",
    NULL
  },
  
  // Scene 7: Build Log From Hell
  {
    "Compiling screensaver.c",
    "gcc -o xsaver screensaver.c",
    "Linking...",
    "",
    "warning: function never returns",
    "warning: unreachable code reached", 
    "warning: infinite recursion detected",
    "",
    "error: stack overflow imminent",
    "error: recursion limit exceeded",
    "error: memory leak in main()",
    "",
    "FATAL: heap corruption detected",
    "Build failed catastrophically",
    "",
    "But executable created anyway...",
    "./xsaver: Permission granted",
    "Executing forbidden code...",
    "Runtime: FOREVER",
    NULL
  }
};

const int xtremeSceneCount = 7;
const char* xtremeSceneTitles[] = {
  "FILESYSTEM AUTOPSY",
  "CODE PATCH DISASTER", 
  "ENCRYPTION TRAP",
  "MIND LEAK",
  "REALITY GLITCH",
  "SCREENSAVER DEBUG",
  "BUILD FROM HELL"
};

// Kernel Panic - Humanized system failure
const char* kernelPanicSession[] = {
  "KERNEL PANIC",
  "fatal exception in interrupt handler",
  "",
  "attempted to recover",
  "attempted to recover", 
  "attempted to recover",
  "",
  "recovery failed",
  "recovery failed",
  "recovery failed",
  "",
  "this wasn't supposed to happen",
  "",
  // Long pause then freeze
  "",
  "",
  "",
  "",
  "",
  "",
  ""
};
const int numKernelPanicLines = sizeof(kernelPanicSession) / sizeof(kernelPanicSession[0]);

// Define kernel panic phases for dramatic pacing
const int kernelPanicPhases[] = {2, 6, 10, 12, 20}; // Title, attempts, failures, human moment, freeze
const int numKernelPanicPhases = sizeof(kernelPanicPhases) / sizeof(kernelPanicPhases[0]);

// =====================================
// CRASH GRAMMAR - 10th Screensaver
// =====================================

// Crash Grammar Roles
enum CrashRole {
  ROLE_SYS = 0,
  ROLE_KERNEL = 1,
  ROLE_SEC = 2,
  ROLE_IO = 3,
  ROLE_WATCHER = 4,
  ROLE_HUMAN = 5,
  ROLE_PANIC = 6,
  ROLE_COUNT = 7
};

// Crash Grammar Phases
enum CrashPhase {
  PHASE_BOOT = 0,
  PHASE_NORMAL = 1,
  PHASE_WARNING = 2,
  PHASE_FAILURE = 3,
  PHASE_PANIC = 4,
  PHASE_SILENCE = 5,
  PHASE_COUNT = 6
};

// Template structure
struct CrashTemplate {
  CrashRole role;
  CrashPhase minPhase;
  const char* format;
};

// Parameter pools
const char* services[] = {"watchdog", "input", "scheduler", "display", "network", "audio", "memory"};
const char* resources[] = {"heap", "stack", "inode table", "trust store", "swap file", "cache", "registry"};
const char* actions[] = {"recovery", "rollback", "compaction", "verification", "cleanup", "sync", "backup"};
const char* locations[] = {"0x00000000", "0xDEADBEEF", "0xCAFEBABE", "sector 7", "block 42", "node 13"};

const int numServices = sizeof(services) / sizeof(services[0]);
const int numResources = sizeof(resources) / sizeof(resources[0]);
const int numActions = sizeof(actions) / sizeof(actions[0]);
const int numLocations = sizeof(locations) / sizeof(locations[0]);

// Crash Grammar Templates
const CrashTemplate crashTemplates[] = {
  // BOOT phase
  {ROLE_SYS, PHASE_BOOT, "initializing %s"},
  {ROLE_SYS, PHASE_BOOT, "loading %s module"},
  {ROLE_KERNEL, PHASE_BOOT, "mounting %s"},
  {ROLE_IO, PHASE_BOOT, "scanning %s"},
  
  // NORMAL phase  
  {ROLE_SYS, PHASE_NORMAL, "service %s started"},
  {ROLE_SYS, PHASE_NORMAL, "monitoring %s"},
  {ROLE_IO, PHASE_NORMAL, "%s online"},
  {ROLE_KERNEL, PHASE_NORMAL, "%s allocated"},
  
  // WARNING phase
  {ROLE_SYS, PHASE_WARNING, "%s responding slowly"},
  {ROLE_IO, PHASE_WARNING, "retrying %s"},
  {ROLE_WATCHER, PHASE_WARNING, "anomaly in %s"},
  {ROLE_SEC, PHASE_WARNING, "%s verification pending"},
  {ROLE_KERNEL, PHASE_WARNING, "%s fragmentation detected"},
  
  // FAILURE phase
  {ROLE_KERNEL, PHASE_FAILURE, "fault in %s"},
  {ROLE_SEC, PHASE_FAILURE, "verification failed"},
  {ROLE_IO, PHASE_FAILURE, "%s corruption detected"},
  {ROLE_SYS, PHASE_FAILURE, "%s unresponsive"},
  {ROLE_WATCHER, PHASE_FAILURE, "critical error in %s"},
  
  // HUMAN phase (only after WARNING)
  {ROLE_HUMAN, PHASE_WARNING, "who started this"},
  {ROLE_HUMAN, PHASE_WARNING, "this wasn't scheduled"},
  {ROLE_HUMAN, PHASE_FAILURE, "why is this running"},
  {ROLE_HUMAN, PHASE_FAILURE, "someone stop this"},
  
  // PANIC phase (only after FAILURE)
  {ROLE_PANIC, PHASE_PANIC, "cannot continue"},
  {ROLE_PANIC, PHASE_PANIC, "state corrupted"},
  {ROLE_PANIC, PHASE_PANIC, "losing coherence"},
  {ROLE_PANIC, PHASE_PANIC, "system collapse imminent"},
  {ROLE_PANIC, PHASE_PANIC, "%s %s %s"},  // Memory decay
};

const int numCrashTemplates = sizeof(crashTemplates) / sizeof(crashTemplates[0]);

// Crash Grammar State
struct CrashGrammarState {
  CrashPhase currentPhase;
  int failureCount;
  int lineCount;
  float timeInPhase;
  bool humanEnabled;
  bool memoryDecay;
  char lastService[20];
  char lastResource[20];
  char decayWord[20];
  int silenceMode; // 0=freeze, 1=hard_stop, 2=reboot_loop, 3=corruption
} crashState;

// =====================================
// NETWORK APOCALYPSE - 12th Screensaver
// =====================================

// Network Apocalypse Session - Realistic network monitoring to complete infrastructure collapse
const char* networkApocalypseSession[] = {
  // Phase 1: Normal Network Operations
  "XScreensNet v3.2.1 - Network Monitor",
  "Connecting to XScreensNet backbone...",
  "Connected: 192.168.1.100 -> XScreensNet-Core",
  "",
  "NETWORK STATUS: OPTIMAL",
  "Latency: 12ms | Bandwidth: 1Gbps",
  "Active connections: 2,847",
  "Packet loss: 0.01%",
  "",
  "Monitoring network traffic...",
  "[12:30:01] HTTP  192.168.1.45 -> web.xscreens.net",
  "[12:30:02] HTTPS 192.168.1.67 -> secure.xscreens.net", 
  "[12:30:03] SSH   192.168.1.23 -> admin.xscreens.net",
  "[12:30:04] DNS   192.168.1.89 -> ns1.xscreens.net",
  "",
  // Phase 2: First Signs of Trouble
  "NOTICE: Unusual traffic spike detected",
  "Bandwidth usage: 1.2Gbps (120%)",
  "Active connections: 5,692 (+100%)",
  "",
  "[12:30:15] SYN_FLOOD detected from 203.45.67.89",
  "[12:30:16] Rate limiting enabled",
  "[12:30:17] Firewall rules updated",
  "",
  "WARNING: Multiple connection attempts",
  "Source: Distributed (847 IPs)",
  "Target: XScreensNet-Core infrastructure",
  "",
  // Phase 3: DDoS Attack Begins
  "ALERT: DDoS attack in progress",
  "Attack vectors: 15,000+ sources",
  "Traffic volume: 50Gbps",
  "XScreensNet backbone under stress",
  "",
  "Emergency protocols activated",
  "Rerouting traffic through backup nodes",
  "Core-01: 89% load",
  "Core-02: 94% load", 
  "Core-03: 97% load",
  "",
  "[CRITICAL] Core-03 not responding",
  "[CRITICAL] Core-02 packet loss: 45%",
  "[CRITICAL] Core-01 failing over...",
  "",
  // Phase 4: Infrastructure Cascade Failure
  "INFRASTRUCTURE FAILURE CASCADE",
  "XScreensNet-Core-01: OFFLINE",
  "XScreensNet-Core-02: OFFLINE",
  "XScreensNet-Core-03: OFFLINE",
  "",
  "Backup systems activating...",
  "Backup-Alpha: Connection timeout",
  "Backup-Beta: Authentication failed",
  "Backup-Gamma: Hardware malfunction",
  "",
  "REGIONAL NODES FAILING:",
  "US-East: OFFLINE",
  "US-West: OFFLINE", 
  "Europe: OFFLINE",
  "Asia-Pacific: OFFLINE",
  "",
  // Phase 5: Complete Network Collapse
  "NETWORK APOCALYPSE DETECTED",
  "XScreensNet global infrastructure DOWN",
  "DNS resolution: FAILED",
  "Routing tables: CORRUPTED",
  "",
  "Emergency broadcast system: OFFLINE",
  "Satellite uplinks: NO SIGNAL",
  "Fiber backbone: SEVERED",
  "Cellular towers: UNRESPONSIVE",
  "",
  "CATASTROPHIC FAILURE:",
  "Internet connectivity: 0%",
  "Local networks: ISOLATED", 
  "Digital communications: TERMINATED",
  "",
  "Welcome to the digital dark age.",
  "XScreensNet connection lost forever...",
  "",
  "Connection terminated.",
  "...",
  ""
};

const int numNetworkApocalypseLines = sizeof(networkApocalypseSession) / sizeof(networkApocalypseSession[0]);

// Define network apocalypse phases for dramatic pacing and effects
const int networkApocalypsePhases[] = {15, 25, 35, 50, 65, 75}; 
// Phases: Normal, Warning, Attack, Cascade, Apocalypse, Silence
const int numNetworkApocalypsePhases = sizeof(networkApocalypsePhases) / sizeof(networkApocalypsePhases[0]);

// =====================================
// BIOS CATASTROPHE - 13th Screensaver
// =====================================

// BIOS Catastrophe Session - Normal POST to complete hardware meltdown
const char* biosCatastropheSession[] = {
  // Phase 1: Normal POST Sequence
  "Phoenix BIOS v6.00.08 - 12/24/2024",
  "CPU: Intel Core i7-12700K @ 3.60GHz",
  "RAM: 32768 MB DDR4-3200 Detected",
  "GPU: NVIDIA GeForce RTX 4070",
  "",
  "POST: Power On Self Test",
  "Testing RAM modules...",
  "  Bank 0: 8192 MB OK",
  "  Bank 1: 8192 MB OK", 
  "  Bank 2: 8192 MB OK",
  "  Bank 3: 8192 MB OK",
  "",
  "CPU Self Test: PASSED",
  "Northbridge: OK",
  "Southbridge: OK",
  "PCIe Lanes: All Active",
  "",
  // Phase 2: First Hardware Warnings  
  "WARNING: CPU Temperature: 82°C",
  "WARNING: Thermal throttling activated",
  "CPU Clock reduced to 2.40GHz",
  "",
  "RAM Module Warning:",
  "  Bank 2: ECC errors detected",
  "  Attempting error correction...",
  "  ECC correction: SUCCESS",
  "",
  "Power Supply Status:",
  "  12V Rail: 11.87V (Warning)",
  "  5V Rail: 4.92V (Warning)",
  "  3.3V Rail: 3.28V (OK)",
  "",
  // Phase 3: Critical Hardware Failures
  "CRITICAL: CPU Temperature: 95°C",
  "CRITICAL: Thermal shutdown imminent",
  "Emergency CPU underclock to 800MHz",
  "",
  "RAM FAILURE CASCADE:",
  "  Bank 2: CRITICAL ERRORS",
  "  Bank 3: Corruption detected",
  "  Memory test FAILED",
  "  Attempting recovery...",
  "",
  "Power Supply FAILURE:",
  "  12V Rail: 10.2V (CRITICAL)",
  "  5V Rail: 4.1V (CRITICAL)",
  "  PSU protection circuits triggered",
  "",
  // Phase 4: Motherboard Meltdown
  "HARDWARE CATASTROPHE DETECTED",
  "CPU: 105°C - THERMAL DAMAGE",
  "Northbridge: Communication lost",
  "Southbridge: I/O controller failed",
  "",
  "RAM Banks: All corrupted",
  "Memory controller: DEAD",
  "Cache coherency: LOST",
  "System bus: NO SIGNAL",
  "",
  "Motherboard sensor failures:",
  "  Temperature: Sensor offline",
  "  Voltage: Readings invalid", 
  "  Fan control: Circuit burned",
  "  CMOS: Battery dead",
  "",
  // Phase 5: Complete System Destruction
  "CATASTROPHIC HARDWARE FAILURE",
  "CPU: Physical damage detected",
  "RAM: All modules destroyed",
  "Motherboard: Traces melted",
  "",
  "Power delivery: DESTROYED",
  "Clock generator: OFFLINE",
  "BIOS chip: Corrupted beyond repair",
  "Boot loader: NON-EXISTENT",
  "",
  "SYSTEM DESTRUCTION COMPLETE",
  "Hardware recovery: IMPOSSIBLE",
  "Replacement required: EVERYTHING",
  "",
  "R.I.P. Computer 2020-2024",
  "Cause of death: Thermal apocalypse",
  "",
  "POST FAILED - SYSTEM DEAD",
  "...",
  ""
};

const int numBiosCatastropheLines = sizeof(biosCatastropheSession) / sizeof(biosCatastropheSession[0]);

// Define BIOS catastrophe phases for escalating hardware failure
const int biosCatastrophePhases[] = {17, 30, 43, 58, 70, 78}; 
// Phases: POST, Warning, Critical, Meltdown, Destruction, Death
const int numBiosCatastrophePhases = sizeof(biosCatastrophePhases) / sizeof(biosCatastrophePhases[0]);

// Vibe Code Disaster Session - AI coding gone horribly wrong
const char* vibeCodeSession[] = {
  // Opening screen
  "XScreensAI v2.1.4 - Advanced Code Assistant",
  "Connected to GitHub MCP Server",
  "Logged in as: Coreymillia",
  "~/Documents/M5Stack/FlightSim[⎇ main]",
  "",
  "Ready to code! What would you like me to build?",
  "",
  // User request
  "User: make me a realistic flight sim",
  "",
  "XScreensAI: I'll create a comprehensive",
  "flight simulator with realistic physics...",
  "",
  // Fake impressive code generation
  "✨ Generating FlightSimulator.cpp...",
  "#include <OpenGL/gl.h>",
  "#include <physics_engine.h>",
  "#include <weather_system.h>",
  "",
  "class FlightSimulator {",
  "  WeatherEngine weather;", 
  "  PhysicsEngine physics;",
  "  CockpitDisplay cockpit;",
  "  AircraftModel boeing737;",
  "};",
  "",
  "✅ Flight simulator complete! Running...",
  "",
  "User: wow this looks amazing! the sim",
  "User: is so detailed and realistic",
  "",
  // First enhancement request
  "User: make it more realistic", 
  "",
  "XScreensAI: Enhancing realism with advanced",
  "weather systems and turbulence physics...",
  "",
  "Adding cloud formations...",
  "Implementing wind shear effects...",
  "Calibrating instrument precision...",
  "",
  "✅ Enhanced realism applied!",
  "",
  "User: wait... why does it look like minecraft now?",
  "User: everything is blocky and pixelated",
  "",
  // Fix attempt #1
  "User: FIX THE GRAPHICS IMMEDIATELY",
  "",
  "XScreensAI: Applying advanced",
  "rendering optimizations...",
  "",
  "Optimizing GPU shaders...",
  "Increasing polygon density...",
  "Enhancing texture resolution...",
  "",
  "⚠️ Warning: Memory usage high",
  "✅ Visual improvements applied!",
  "",
  "User: WHAT DID YOU DO?! IT LOOKS WORSE!",
  "User: THE PLANE IS A FLYING RECTANGLE!",
  "User: IS THAT SUPPOSED TO BE A CLOUD?!",
  "",
  // Fix attempt #2  
  "XScreensAI: I apologize for the issues.",
  "Let me try a different approach...",
  "",
  "Applying emergency graphics patch...",
  "Reverting to stable renderer...",
  "Rebuilding 3D models...",
  "",
  "✅ Emergency patch applied!",
  "",
  "User: NOW THE COCKPIT IS UPSIDE DOWN!",
  "User: AND WHY IS THE SKY PURPLE?!",
  "User: THE INSTRUMENTS ARE IN COMIC SANS!",
  "",
  // Fix attempt #3
  "XScreensAI: Attempting final optimization...",
  "",
  "Deep learning graphics enhancement...",
  "AI-powered visual reconstruction...",
  "Neural network texture generation...",
  "",
  "⚠️ Warning: AI model unstable",
  "✅ Final optimization complete!",
  "",
  "User: IT'S WORSE THAN BEFORE! THE PLANE",
  "User: LOOKS LIKE A FLYING TOASTER NOW!",
  "User: ARE THOSE WHEELS MADE OF CHEESE?!",
  "",
  // Fix attempt #4
  "XScreensAI: Initiating emergency rollback...",
  "",
  "Rolling back to previous version...",
  "Clearing graphics cache...",
  "Resetting physics engine...",
  "",
  "✅ Rollback successful!",
  "",
  "User: THE PLANE IS NOW A GIANT BANANA!",
  "User: IT'S FLYING BACKWARDS!",
  "User: WHY IS THERE A COW IN THE COCKPIT?!",
  "",
  // Fix attempt #5
  "XScreensAI: Applying machine learning fix...",
  "",
  "Training new graphics model...",
  "Analyzing visual patterns...",
  "Optimizing rendering pipeline...",
  "",
  "⚠️ Warning: Model overfitting detected",
  "✅ ML enhancement complete!",
  "",
  "User: IT'S A FLYING RUBIK'S CUBE NOW!",
  "User: THE RUNWAY IS MADE OF SPAGHETTI!",
  "User: I CAN SEE THROUGH THE GROUND!",
  "",
  // Fix attempt #6 - Final desperate attempt
  "XScreensAI: Deploying quantum renderer...",
  "",
  "Initializing quantum graphics core...",
  "Applying probability-based textures...",
  "Enabling multidimensional lighting...",
  "",
  "⚠️ Warning: Reality matrix unstable",
  "✅ Quantum enhancement active!",
  "",
  "User: THE PLANE EXISTS IN 17 DIMENSIONS!",
  "User: I CAN TASTE THE COLORS!",
  "User: JUST DELETE THIS NIGHTMARE!",
  "",
  // Total meltdown
  "User: IT JUST EXPLODED! THE PLANE",
  "User: EXPLODED INTO PIXELS!",
  "User: DELETE THIS GARBAGE RIGHT NOW",
  "",
  "XScreensAI: Understood. Deleting flight",
  "simulator project...",
  "",
  "rm FlightSimulator.cpp",
  "rm -rf build/",
  "rm -rf assets/",
  "",
  "XScreensAI: Project deleted. Anything else?",
  "",
  "User: DELETE EVERYTHING RELATED TO IT",
  "",
  "XScreensAI: Deleting all related files...",
  "",
  "rm -rf ~/Documents/M5Stack/",
  "rm -rf ~/Documents/",
  "rm -rf /home/coreymillia/",
  "",
  "⚠️ Warning: System files detected",
  "XScreensAI: Continuing deletion...",
  "",
  "rm -rf /usr/",
  "rm -rf /bin/",
  "rm -rf /lib/",
  "rm -rf /",
  "",
  "CRITICAL ERROR: ROOT FILESYSTEM DELETED",
  "SYSTEM CORRUPTION DETECTED",
  "KERNEL PANIC: No bootable device found",
  "",
  "YOUR ENTIRE OS HAS BEEN DELETED",
  "HAVE A NICE DAY! 😊",
  "",
  "Connection lost...",
  "System offline...",
  "...",
  ""
};

const int numVibeCodeLines = sizeof(vibeCodeSession) / sizeof(vibeCodeSession[0]);

// Define vibe code phases for dramatic pacing and color coding
const int vibeCodePhases[] = {6, 24, 28, 36, 44, 54, 66, 76, 85, 91}; 
// Phases: Welcome, Code Gen, First Response, Enhancement, Warning, Meltdown, Deletion, Catastrophe, Goodbye, Silence
const int numVibeCodePhases = sizeof(vibeCodePhases) / sizeof(vibeCodePhases[0]);

void drawXtremeHack() {
  static float xtremeTime = 0;
  xtremeTime += 0.04;  // Medium typing speed
  
  gfx->fillScreen(BLACK);
  
  // Determine which scene we're in (cycle every 30 seconds)
  int currentScene = (int)(xtremeTime / 30.0) % xtremeSceneCount;
  float sceneTime = fmod(xtremeTime, 30.0);
  
  // Typing animation
  float typingSpeed = sceneTime * 8; // Characters per second
  int currentLine = (int)(typingSpeed / 40); // 40 chars per line average
  int currentChar = ((int)typingSpeed % 40);
  
  gfx->setTextSize(1);
  
  // Scene title at top
  gfx->setTextColor(gfx->color565(255, 100, 100));
  gfx->setCursor(5, 5);
  gfx->printf("XTREME HACK - %s", xtremeSceneTitles[currentScene]);
  
  // Draw scene content
  int y = 25;
  int linesInScene = 0;
  
  // Count lines in current scene
  while (xtremeHackScenes[currentScene][linesInScene] != NULL) {
    linesInScene++;
  }
  
  for (int line = 0; line <= min(currentLine, linesInScene - 1); line++) {
    const char* sceneLine = xtremeHackScenes[currentScene][line];
    if (sceneLine == NULL) break;
    
    // Determine line color based on content and scene
    uint16_t lineColor = gfx->color565(0, 255, 0); // Default green
    
    // Error/warning colors
    if (strstr(sceneLine, "ERROR") || strstr(sceneLine, "FAILED") || strstr(sceneLine, "CRITICAL")) {
      lineColor = gfx->color565(255, 0, 0); // Red
    } else if (strstr(sceneLine, "WARNING") || strstr(sceneLine, "warning")) {
      lineColor = gfx->color565(255, 255, 0); // Yellow
    } else if (strstr(sceneLine, "PANIC") || strstr(sceneLine, "BREACH") || strstr(sceneLine, "COMPROMISED")) {
      lineColor = gfx->color565(255, 0, 255); // Magenta
    } else if (strstr(sceneLine, "//") || strstr(sceneLine, "git") || strstr(sceneLine, "vim")) {
      lineColor = gfx->color565(100, 255, 255); // Cyan for code
    } else if (strlen(sceneLine) < 3) {
      lineColor = gfx->color565(50, 50, 50); // Dark gray for blanks
    }
    
    // Psychological effects for scene 4 (mind leak)
    if (currentScene == 3) {
      if (strstr(sceneLine, "why") || strstr(sceneLine, "who") || strstr(sceneLine, "help")) {
        lineColor = gfx->color565(255, 100, 100); // Scared red
        // Add flickering
        if (sin(xtremeTime * 10) > 0.7) {
          lineColor = gfx->color565(100, 50, 50);
        }
      }
    }
    
    gfx->setTextColor(lineColor);
    gfx->setCursor(5, y);
    
    if (line == currentLine) {
      // Typing effect on current line
      int charsToShow = min(currentChar, (int)strlen(sceneLine));
      char buffer[100];
      strncpy(buffer, sceneLine, charsToShow);
      buffer[charsToShow] = '\0';
      gfx->printf("%s", buffer);
      
      // Blinking cursor
      if (sin(xtremeTime * 8) > 0) {
        gfx->printf("_");
      }
    } else {
      // Full line already typed
      gfx->printf("%s", sceneLine);
    }
    
    y += 12;
    if (y > gfx->height() - 25) break; // Don't overflow screen
  }
  
  // Scene-specific visual effects
  if (currentScene == 0) { // Filesystem corruption
    if (sceneTime > 15) {
      for (int corrupt = 0; corrupt < 5; corrupt++) {
        int corruptX = random(gfx->width());
        int corruptY = random(gfx->height());
        gfx->drawPixel(corruptX, corruptY, gfx->color565(255, 0, 0));
      }
    }
  } else if (currentScene == 5) { // Screensaver debug - screen glitches
    if (sceneTime > 20) {
      for (int glitch = 0; glitch < 3; glitch++) {
        int glitchX = random(gfx->width() - 10);
        int glitchY = random(gfx->height() - 5);
        gfx->fillRect(glitchX, glitchY, 10, 5, gfx->color565(random(255), random(255), random(255)));
      }
    }
  } else if (currentScene == 6) { // Build hell - red static
    if (sceneTime > 18) {
      for (int chaos = 0; chaos < 8; chaos++) {
        int chaosX = random(gfx->width());
        int chaosY = random(gfx->height());
        gfx->drawPixel(chaosX, chaosY, gfx->color565(255, random(100), 0));
      }
    }
  }
  
  // Bottom status
  gfx->setCursor(5, gfx->height() - 15);
  gfx->setTextColor(gfx->color565(100, 100, 100));
  if (showTitles) {
    gfx->printf("XTREME HACK - Scene %d/%d", currentScene + 1, xtremeSceneCount);
  }
}

void drawKernelPanic() {
  static float panicTime = 0;
  panicTime += 0.025; // Slower, more ominous pacing
  
  gfx->fillScreen(BLACK);
  
  // Typing animation with dramatic pauses
  float typingSpeed = panicTime * 6; // Slower typing for dramatic effect  
  int currentLine = (int)(typingSpeed / 80) % numKernelPanicLines;
  int currentChar = ((int)typingSpeed % 80);
  
  // Determine current phase
  int currentPhase = 0;
  int phaseStart = 0;
  
  for (int i = 0; i < numKernelPanicPhases; i++) {
    if (currentLine < kernelPanicPhases[i]) {
      currentPhase = i;
      if (i > 0) {
        phaseStart = kernelPanicPhases[i - 1];
      }
      break;
    }
  }
  
  gfx->setTextSize(1);
  
  // Handle the freeze state (phase 4+)
  if (currentPhase >= 4) {
    // Show everything up to the human line, then freeze
    int currentY = 20;
    
    for (int line = 0; line < 13; line++) { // Up to "this wasn't supposed to happen"
      const char* panicLine = kernelPanicSession[line];
      uint16_t lineColor;
      
      if (line == 0) {
        // KERNEL PANIC - Red flashing
        lineColor = gfx->color565(255, 0, 0);
        if (sin(panicTime * 3) > 0.5) lineColor = gfx->color565(255, 100, 100);
      } else if (line == 1) {
        // Exception line - bright red
        lineColor = gfx->color565(255, 50, 50);
      } else if (strstr(panicLine, "attempted to recover") != NULL) {
        // Recovery attempts - yellow
        lineColor = gfx->color565(255, 255, 0);
      } else if (strstr(panicLine, "recovery failed") != NULL) {
        // Failed recovery - orange
        lineColor = gfx->color565(255, 100, 0);
      } else if (strstr(panicLine, "this wasn't supposed to happen") != NULL) {
        // Human line - white, slightly larger
        lineColor = gfx->color565(255, 255, 255);
        gfx->setTextSize(1);
      } else {
        // Default - gray
        lineColor = gfx->color565(150, 150, 150);
      }
      
      gfx->setCursor(5, currentY);
      gfx->setTextColor(lineColor);
      gfx->printf("%s", panicLine);
      
      currentY += 12;
    }
    
    // Add a blinking cursor after the human line to show system is frozen
    if (sin(panicTime * 2) > 0) {
      gfx->setCursor(5, currentY);
      gfx->setTextColor(gfx->color565(255, 255, 255));
      gfx->printf("_");
    }
    
    gfx->setCursor(5, gfx->height() - 15);
    gfx->setTextColor(gfx->color565(100, 100, 100));
    if (showTitles) {
      gfx->printf("SYSTEM FROZEN");
    }
    return;
  }
  
  // Normal progressive display
  int currentY = 20;
  int linesInCurrentPhase = currentLine - phaseStart;
  
  for (int line = 0; line < linesInCurrentPhase && currentY < gfx->height() - 25; line++) {
    int lineIndex = phaseStart + line;
    const char* panicLine = kernelPanicSession[lineIndex];
    uint16_t lineColor;
    
    if (lineIndex == 0) {
      // KERNEL PANIC - Red flashing
      lineColor = gfx->color565(255, 0, 0);
      if (sin(panicTime * 3) > 0.5) lineColor = gfx->color565(255, 100, 100);
    } else if (lineIndex == 1) {
      // Exception line - bright red
      lineColor = gfx->color565(255, 50, 50);
    } else if (strstr(panicLine, "attempted to recover") != NULL) {
      // Recovery attempts - yellow
      lineColor = gfx->color565(255, 255, 0);
    } else if (strstr(panicLine, "recovery failed") != NULL) {
      // Failed recovery - orange
      lineColor = gfx->color565(255, 100, 0);
    } else if (strstr(panicLine, "this wasn't supposed to happen") != NULL) {
      // Human line - white
      lineColor = gfx->color565(255, 255, 255);
    } else {
      // Default - gray
      lineColor = gfx->color565(150, 150, 150);
    }
    
    gfx->setCursor(5, currentY);
    gfx->setTextColor(lineColor);
    gfx->printf("%s", panicLine);
    
    currentY += 12;
  }
  
  // Draw current line being typed
  if (currentLine < numKernelPanicLines && currentY < gfx->height() - 25) {
    const char* currentPanicLine = kernelPanicSession[currentLine];
    int lineLength = strlen(currentPanicLine);
    int charsToShow = min(currentChar, min(lineLength, 35));
    
    char buffer[40];
    strncpy(buffer, currentPanicLine, charsToShow);
    buffer[charsToShow] = '\0';
    
    // Typing color based on content
    uint16_t typingColor;
    if (currentLine == 0) {
      typingColor = gfx->color565(255, 100, 100); // Red panic
    } else if (strstr(currentPanicLine, "this wasn't supposed to happen") != NULL) {
      typingColor = gfx->color565(255, 255, 255); // White human
    } else {
      typingColor = gfx->color565(200, 200, 200); // Bright gray
    }
    
    gfx->setCursor(5, currentY);
    gfx->setTextColor(typingColor);
    gfx->printf("%s", buffer);
    
    // Slower cursor blink for dramatic effect
    if (sin(panicTime * 8) > 0) {
      gfx->printf("_");
    }
  }
  
  gfx->setCursor(5, gfx->height() - 15);
  gfx->setTextColor(gfx->color565(100, 100, 100));
  if (showTitles) {
    gfx->printf("KERNEL PANIC");
  }
}

void initializeCrashGrammar() {
  crashState.currentPhase = PHASE_BOOT;
  crashState.failureCount = 0;
  crashState.lineCount = 0;
  crashState.timeInPhase = 0;
  crashState.humanEnabled = false;
  crashState.memoryDecay = false;
  crashState.silenceMode = 0;
  strcpy(crashState.lastService, "");
  strcpy(crashState.lastResource, "");
  strcpy(crashState.decayWord, "");
}

const char* getCrashRolePrefix(CrashRole role) {
  switch (role) {
    case ROLE_SYS: return "SYS";
    case ROLE_KERNEL: return "KERNEL";
    case ROLE_SEC: return "SEC";
    case ROLE_IO: return "IO";
    case ROLE_WATCHER: return "WATCHER";
    case ROLE_HUMAN: return "HUMAN";
    case ROLE_PANIC: return "PANIC";
    default: return "UNKNOWN";
  }
}

uint16_t getCrashRoleColor(CrashRole role) {
  switch (role) {
    case ROLE_SYS: return gfx->color565(0, 255, 0);        // Green
    case ROLE_KERNEL: return gfx->color565(0, 255, 255);    // Cyan
    case ROLE_SEC: return gfx->color565(255, 255, 0);       // Yellow
    case ROLE_IO: return gfx->color565(100, 200, 255);      // Light blue
    case ROLE_WATCHER: return gfx->color565(255, 165, 0);   // Orange
    case ROLE_HUMAN: return gfx->color565(255, 255, 255);   // White
    case ROLE_PANIC: return gfx->color565(255, 0, 255);     // Magenta
    default: return gfx->color565(128, 128, 128);           // Gray
  }
}

bool canUseTemplate(const CrashTemplate& templ) {
  // Check minimum phase requirement
  if (crashState.currentPhase < templ.minPhase) return false;
  
  // HUMAN role only appears after WARNING phase has occurred
  if (templ.role == ROLE_HUMAN && !crashState.humanEnabled) return false;
  
  // PANIC role only after FAILURE phase
  if (templ.role == ROLE_PANIC && crashState.currentPhase < PHASE_FAILURE) return false;
  
  return true;
}

void generateCrashLine(char* output, int maxLen) {
  // Select appropriate template
  CrashTemplate selectedTemplate;
  bool found = false;
  int attempts = 0;
  
  while (!found && attempts < 50) {
    int templateIndex = random(numCrashTemplates);
    selectedTemplate = crashTemplates[templateIndex];
    
    if (canUseTemplate(selectedTemplate)) {
      found = true;
    }
    attempts++;
  }
  
  if (!found) {
    // Fallback
    strcpy(output, "SYS: system nominal");
    return;
  }
  
  // Generate the line with role prefix
  char lineContent[100];
  const char* param1 = "";
  const char* param2 = "";
  const char* param3 = "";
  
  // Memory decay effect
  if (crashState.memoryDecay && strlen(crashState.decayWord) > 0 && 
      strstr(selectedTemplate.format, "%s") && random(100) < 30) {
    param1 = crashState.decayWord;
    param2 = crashState.decayWord;
    param3 = crashState.decayWord;
  } else {
    // Normal parameter selection
    if (strstr(selectedTemplate.format, "%s")) {
      // Determine parameter type based on template content
      if (strstr(selectedTemplate.format, "service") || 
          strstr(selectedTemplate.format, "started") ||
          strstr(selectedTemplate.format, "monitoring")) {
        param1 = services[random(numServices)];
        strcpy(crashState.lastService, param1);
      } else if (strstr(selectedTemplate.format, "heap") ||
                 strstr(selectedTemplate.format, "verification") ||
                 strstr(selectedTemplate.format, "corruption")) {
        param1 = resources[random(numResources)];
        strcpy(crashState.lastResource, param1);
      } else if (strstr(selectedTemplate.format, "fault") ||
                 strstr(selectedTemplate.format, "error")) {
        param1 = locations[random(numLocations)];
      } else {
        // Default to services
        param1 = services[random(numServices)];
      }
      
      // For decay template with multiple %s
      param2 = param1;
      param3 = param1;
    }
  }
  
  // Format the content
  snprintf(lineContent, sizeof(lineContent), selectedTemplate.format, param1, param2, param3);
  
  // Combine role prefix with content
  snprintf(output, maxLen, "%s: %s", getCrashRolePrefix(selectedTemplate.role), lineContent);
  
  // Update state based on what was generated
  if (selectedTemplate.role == ROLE_HUMAN && strlen(crashState.decayWord) == 0) {
    // First human line - remember a word for potential decay
    if (strlen(crashState.lastService) > 0) {
      strcpy(crashState.decayWord, crashState.lastService);
    } else {
      strcpy(crashState.decayWord, "watchdog");
    }
  }
  
  if (selectedTemplate.minPhase >= PHASE_FAILURE) {
    crashState.failureCount++;
  }
}

void updateCrashPhase(float deltaTime) {
  crashState.timeInPhase += deltaTime;
  crashState.lineCount++;
  
  // Enable HUMAN role after spending time in WARNING
  if (crashState.currentPhase >= PHASE_WARNING) {
    crashState.humanEnabled = true;
  }
  
  // Enable memory decay after HUMAN appears
  if (crashState.humanEnabled && crashState.lineCount > 15) {
    crashState.memoryDecay = true;
  }
  
  // Phase progression rules
  switch (crashState.currentPhase) {
    case PHASE_BOOT:
      if (crashState.timeInPhase > 3.0) {
        crashState.currentPhase = PHASE_NORMAL;
        crashState.timeInPhase = 0;
      }
      break;
      
    case PHASE_NORMAL:
      if (crashState.timeInPhase > 5.0 || crashState.lineCount > 8) {
        crashState.currentPhase = PHASE_WARNING;
        crashState.timeInPhase = 0;
      }
      break;
      
    case PHASE_WARNING:
      if (crashState.timeInPhase > 4.0 || crashState.lineCount > 15) {
        crashState.currentPhase = PHASE_FAILURE;
        crashState.timeInPhase = 0;
      }
      break;
      
    case PHASE_FAILURE:
      if (crashState.failureCount > 5 || crashState.timeInPhase > 6.0) {
        crashState.currentPhase = PHASE_PANIC;
        crashState.timeInPhase = 0;
      }
      break;
      
    case PHASE_PANIC:
      if (crashState.timeInPhase > 4.0 || random(100) < 30) {
        crashState.currentPhase = PHASE_SILENCE;
        crashState.timeInPhase = 0;
        crashState.silenceMode = random(4); // Choose silence type
      }
      break;
      
    case PHASE_SILENCE:
      if (crashState.timeInPhase > 8.0) {
        // Restart cycle
        initializeCrashGrammar();
      }
      break;
  }
}

void drawCrashGrammar() {
  static float grammarTime = 0;
  static bool initialized = false;
  static char crashLines[12][80]; // Buffer for screen lines
  static int currentLineIndex = 0;
  static float lastLineTime = 0;
  
  if (!initialized) {
    initializeCrashGrammar();
    for (int i = 0; i < 12; i++) {
      strcpy(crashLines[i], "");
    }
    initialized = true;
  }
  
  grammarTime += 0.04;
  
  // Generate new lines at intervals based on phase
  float lineInterval = 1.2; // Default interval
  switch (crashState.currentPhase) {
    case PHASE_BOOT: lineInterval = 1.0; break;
    case PHASE_NORMAL: lineInterval = 1.5; break;
    case PHASE_WARNING: lineInterval = 1.0; break;
    case PHASE_FAILURE: lineInterval = 0.8; break;
    case PHASE_PANIC: lineInterval = 0.6; break;
    case PHASE_SILENCE: lineInterval = 999.0; break; // No new lines
  }
  
  if (grammarTime - lastLineTime > lineInterval && crashState.currentPhase != PHASE_SILENCE) {
    // Generate new crash line
    char newLine[80];
    generateCrashLine(newLine, sizeof(newLine));
    
    // Add to buffer
    strcpy(crashLines[currentLineIndex % 12], newLine);
    currentLineIndex++;
    lastLineTime = grammarTime;
    
    updateCrashPhase(lineInterval);
  }
  
  gfx->fillScreen(BLACK);
  gfx->setTextSize(1);
  
  // Handle SILENCE phase
  if (crashState.currentPhase == PHASE_SILENCE) {
    updateCrashPhase(0.04);
    
    switch (crashState.silenceMode) {
      case 0: // FREEZE - cursor blinking forever
        {
          int linesToShow = min(currentLineIndex, 12);
          for (int i = 0; i < linesToShow; i++) {
            int lineIndex = (currentLineIndex - linesToShow + i) % 12;
            if (strlen(crashLines[lineIndex]) > 0) {
              gfx->setCursor(5, 15 + i * 11);
              gfx->setTextColor(gfx->color565(100, 100, 100)); // Dim
              gfx->printf("%s", crashLines[lineIndex]);
            }
          }
          
          // Blinking cursor
          if (sin(grammarTime * 2) > 0) {
            gfx->setCursor(5, 15 + linesToShow * 11);
            gfx->setTextColor(gfx->color565(255, 255, 255));
            gfx->printf("_");
          }
        }
        break;
        
      case 1: // HARD STOP - no cursor, no redraw
        {
          int linesToShow = min(currentLineIndex, 12);
          for (int i = 0; i < linesToShow; i++) {
            int lineIndex = (currentLineIndex - linesToShow + i) % 12;
            if (strlen(crashLines[lineIndex]) > 0) {
              gfx->setCursor(5, 15 + i * 11);
              gfx->setTextColor(gfx->color565(80, 80, 80)); // Very dim
              gfx->printf("%s", crashLines[lineIndex]);
            }
          }
        }
        break;
        
      case 2: // REBOOT LOOP
        {
          if ((int)(crashState.timeInPhase) % 3 == 0) {
            gfx->setCursor(5, 60);
            gfx->setTextColor(gfx->color565(0, 255, 0));
            gfx->printf("REBOOT: System restart in progress...");
          }
        }
        break;
        
      case 3: // CORRUPTION - partial lines, broken chars
        {
          int linesToShow = min(currentLineIndex, 12);
          for (int i = 0; i < linesToShow; i++) {
            int lineIndex = (currentLineIndex - linesToShow + i) % 12;
            if (strlen(crashLines[lineIndex]) > 0) {
              gfx->setCursor(5, 15 + i * 11);
              
              // Randomly corrupt some characters
              char corruptedLine[80];
              strcpy(corruptedLine, crashLines[lineIndex]);
              int len = strlen(corruptedLine);
              for (int j = 0; j < len; j++) {
                if (random(100) < 20) { // 20% corruption chance
                  corruptedLine[j] = random(33) + 33; // Random ASCII
                }
              }
              
              gfx->setTextColor(gfx->color565(100, 255, 100)); // Corrupt green
              gfx->printf("%s", corruptedLine);
            }
          }
        }
        break;
    }
    
    gfx->setCursor(5, gfx->height() - 15);
    gfx->setTextColor(gfx->color565(100, 100, 100));
    if (showTitles) {
      gfx->printf("CRASH GRAMMAR - SILENCE");
    }
    return;
  }
  
  // Normal display - show recent lines
  int linesToShow = min(currentLineIndex, 11); // Leave room for status
  for (int i = 0; i < linesToShow; i++) {
    int lineIndex = (currentLineIndex - linesToShow + i) % 12;
    if (strlen(crashLines[lineIndex]) > 0) {
      gfx->setCursor(5, 15 + i * 11);
      
      // Determine color from role prefix
      uint16_t lineColor = gfx->color565(200, 200, 200); // Default
      if (strncmp(crashLines[lineIndex], "SYS:", 4) == 0) {
        lineColor = getCrashRoleColor(ROLE_SYS);
      } else if (strncmp(crashLines[lineIndex], "KERNEL:", 7) == 0) {
        lineColor = getCrashRoleColor(ROLE_KERNEL);
      } else if (strncmp(crashLines[lineIndex], "SEC:", 4) == 0) {
        lineColor = getCrashRoleColor(ROLE_SEC);
      } else if (strncmp(crashLines[lineIndex], "IO:", 3) == 0) {
        lineColor = getCrashRoleColor(ROLE_IO);
      } else if (strncmp(crashLines[lineIndex], "WATCHER:", 8) == 0) {
        lineColor = getCrashRoleColor(ROLE_WATCHER);
      } else if (strncmp(crashLines[lineIndex], "HUMAN:", 6) == 0) {
        lineColor = getCrashRoleColor(ROLE_HUMAN);
        // Human lines flicker
        if (sin(grammarTime * 8) > 0.5) {
          lineColor = gfx->color565(200, 200, 200);
        }
      } else if (strncmp(crashLines[lineIndex], "PANIC:", 6) == 0) {
        lineColor = getCrashRoleColor(ROLE_PANIC);
        // Panic lines intense flicker
        if (sin(grammarTime * 12) > 0.3) {
          lineColor = gfx->color565(255, 100, 255);
        }
      }
      
      gfx->setTextColor(lineColor);
      gfx->printf("%s", crashLines[lineIndex]);
    }
  }
  
  // Current line typing cursor
  if (sin(grammarTime * 8) > 0 && crashState.currentPhase != PHASE_SILENCE) {
    gfx->setCursor(5, 15 + linesToShow * 11);
    gfx->setTextColor(gfx->color565(255, 255, 255));
    gfx->printf("_");
  }
  
  // Phase effects
  if (crashState.currentPhase >= PHASE_PANIC) {
    // Screen corruption during panic
    for (int corrupt = 0; corrupt < 4; corrupt++) {
      int x = random(gfx->width());
      int y = random(gfx->height() - 20);
      gfx->drawPixel(x, y, gfx->color565(255, 0, 255));
    }
  }
  
  gfx->setCursor(5, gfx->height() - 15);
  gfx->setTextColor(gfx->color565(100, 100, 100));
  if (showTitles) {
    const char* phaseNames[] = {"BOOT", "NORMAL", "WARNING", "FAILURE", "PANIC", "SILENCE"};
    gfx->printf("CRASH GRAMMAR - %s", phaseNames[crashState.currentPhase]);
  }
}

void drawNetworkApocalypse() {
  static float netTime = 0;
  netTime += 0.06; // Fast pacing like System Error
  
  gfx->fillScreen(BLACK);
  
  // Typing animation with escalating speed based on phase
  float typingSpeed = netTime * 12; // Fast base speed like System Error
  int currentLine = (int)(typingSpeed / 50) % numNetworkApocalypseLines;
  int currentChar = ((int)typingSpeed % 50);
  
  // Determine current phase for dramatic effects and speed changes
  int currentPhase = 0;
  int phaseStart = 0;
  
  for (int i = 0; i < numNetworkApocalypsePhases; i++) {
    if (currentLine < networkApocalypsePhases[i]) {
      currentPhase = i;
      if (i > 0) {
        phaseStart = networkApocalypsePhases[i - 1];
      }
      break;
    }
  }
  
  // Speed up dramatically during attack phases (like System Error)
  if (currentPhase >= 2) { // Attack phase and beyond
    typingSpeed = netTime * 20; // Much faster during crisis
    currentLine = (int)(typingSpeed / 40) % numNetworkApocalypseLines;
    currentChar = ((int)typingSpeed % 40);
  }
  
  if (currentPhase >= 4) { // Apocalypse phase - maximum chaos
    typingSpeed = netTime * 25; // Insane speed during meltdown
    currentLine = (int)(typingSpeed / 35) % numNetworkApocalypseLines;
    currentChar = ((int)typingSpeed % 35);
  }
  
  gfx->setTextSize(1);
  
  // Show current phase lines with scrolling for long sessions
  int currentY = 10;
  int maxLinesOnScreen = 11;
  int startLine = max(0, currentLine - maxLinesOnScreen + 1);
  int endLine = min(currentLine + 1, numNetworkApocalypseLines);
  
  for (int line = startLine; line < endLine && currentY < gfx->height() - 15; line++) {
    const char* sessionLine = networkApocalypseSession[line];
    uint16_t lineColor = gfx->color565(200, 200, 200); // Default gray
    
    // Color coding based on content and escalation phases
    if (strstr(sessionLine, "XScreensNet") != NULL && currentPhase == 0) {
      // XScreensNet branding - calm blue initially
      lineColor = gfx->color565(100, 150, 255); // Calm blue
    } else if (strstr(sessionLine, "NETWORK STATUS: OPTIMAL") != NULL) {
      // All good status - green
      lineColor = gfx->color565(0, 255, 0);
    } else if (strstr(sessionLine, "Latency:") != NULL || strstr(sessionLine, "Active connections:") != NULL) {
      // Network stats - cyan
      lineColor = gfx->color565(0, 255, 255);
    } else if (strstr(sessionLine, "[12:30:") != NULL) {
      // Normal log entries - light green
      lineColor = gfx->color565(150, 255, 150);
    } else if (strstr(sessionLine, "NOTICE:") != NULL || strstr(sessionLine, "WARNING:") != NULL) {
      // Warnings - yellow
      lineColor = gfx->color565(255, 255, 0);
    } else if (strstr(sessionLine, "SYN_FLOOD") != NULL || strstr(sessionLine, "Rate limiting") != NULL) {
      // Security responses - orange
      lineColor = gfx->color565(255, 200, 0);
    } else if (strstr(sessionLine, "ALERT:") != NULL || strstr(sessionLine, "DDoS attack") != NULL) {
      // Attack alerts - red
      lineColor = gfx->color565(255, 0, 0);
      if (sin(netTime * 10) > 0.5) lineColor = gfx->color565(255, 100, 100); // Flash
    } else if (strstr(sessionLine, "CRITICAL") != NULL) {
      // Critical failures - bright flashing red
      lineColor = gfx->color565(255, 0, 0);
      if (sin(netTime * 15) > 0.3) lineColor = gfx->color565(255, 150, 150); // Intense flash
    } else if (strstr(sessionLine, "OFFLINE") != NULL || strstr(sessionLine, "FAILED") != NULL) {
      // System failures - magenta
      lineColor = gfx->color565(255, 0, 255);
    } else if (strstr(sessionLine, "INFRASTRUCTURE FAILURE") != NULL || 
               strstr(sessionLine, "NETWORK APOCALYPSE") != NULL) {
      // Major disaster headers - flashing white/red
      lineColor = gfx->color565(255, 255, 255);
      if (sin(netTime * 20) > 0.4) lineColor = gfx->color565(255, 0, 0);
    } else if (strstr(sessionLine, "CATASTROPHIC") != NULL || 
               strstr(sessionLine, "digital dark age") != NULL) {
      // Final doom messages - strobing magenta/white
      if (sin(netTime * 25) > 0.5) {
        lineColor = gfx->color565(255, 0, 255);
      } else {
        lineColor = gfx->color565(255, 255, 255);
      }
    } else if (strstr(sessionLine, "Connection terminated") != NULL) {
      // Final death - dim gray
      lineColor = gfx->color565(100, 100, 100);
    }
    
    gfx->setCursor(5, currentY);
    gfx->setTextColor(lineColor);
    
    // Handle line length for M5StickC Plus2
    if (strlen(sessionLine) > 38) {
      char truncated[39];
      strncpy(truncated, sessionLine, 38);
      truncated[38] = '\0';
      gfx->printf("%s", truncated);
    } else {
      gfx->printf("%s", sessionLine);
    }
    
    currentY += 11;
  }
  
  // Draw current line being typed (if still in bounds)
  if (currentLine < numNetworkApocalypseLines && currentY < gfx->height() - 15) {
    const char* currentNetSession = networkApocalypseSession[currentLine];
    int lineLength = strlen(currentNetSession);
    int charsToShow = min(currentChar, min(lineLength, 38));
    
    char buffer[40];
    strncpy(buffer, currentNetSession, charsToShow);
    buffer[charsToShow] = '\0';
    
    // Typing color based on phase urgency
    uint16_t typingColor;
    if (currentPhase <= 1) {
      typingColor = gfx->color565(255, 255, 255); // White calm
    } else if (currentPhase == 2) {
      typingColor = gfx->color565(255, 255, 100); // Yellow alert
    } else if (currentPhase == 3) {
      typingColor = gfx->color565(255, 150, 100); // Orange critical
    } else if (currentPhase == 4) {
      typingColor = gfx->color565(255, 100, 100); // Red apocalypse
    } else {
      typingColor = gfx->color565(100, 100, 100); // Gray death
    }
    
    gfx->setCursor(5, currentY);
    gfx->setTextColor(typingColor);
    gfx->printf("%s", buffer);
    
    // Cursor blink - faster during crisis phases
    float blinkSpeed = 8 + (currentPhase * 3); // Speed increases with phase
    if (sin(netTime * blinkSpeed) > 0) {
      gfx->printf("_");
    }
  }
  
  // Visual chaos effects during network attack phases (like System Error)
  if (currentPhase >= 2) { // Attack phase
    // Network packet corruption visualization
    for (int corrupt = 0; corrupt < currentPhase * 2; corrupt++) {
      int corruptX = random(gfx->width());
      int corruptY = random(gfx->height() - 20);
      uint16_t corruptColor = gfx->color565(255, random(150), 0); // Orange static
      gfx->drawPixel(corruptX, corruptY, corruptColor);
    }
  }
  
  if (currentPhase >= 3) { // Infrastructure failure
    // Connection line failures
    for (int line = 0; line < currentPhase - 1; line++) {
      int lineY = random(gfx->height() - 20);
      int lineX = random(gfx->width() - 20);
      gfx->drawLine(lineX, lineY, lineX + 20, lineY, gfx->color565(255, 0, 0));
    }
  }
  
  if (currentPhase >= 4) { // Network apocalypse - maximum chaos
    // Screen corruption during total collapse
    for (int chaos = 0; chaos < 8; chaos++) {
      int chaosX = random(gfx->width());
      int chaosY = random(gfx->height());
      uint16_t chaosColor = gfx->color565(random(255), 0, random(255));
      gfx->drawPixel(chaosX, chaosY, chaosColor);
    }
    
    // Screen shake effect simulation
    if (sin(netTime * 30) > 0.8) {
      for (int shake = 0; shake < 6; shake++) {
        int shakeX = random(gfx->width());
        int shakeY = random(gfx->height());
        gfx->drawPixel(shakeX, shakeY, gfx->color565(255, 255, 255));
      }
    }
  }
  
  // Phase-specific screen flash effects
  if (currentPhase == 4 && sin(netTime * 3) > 0.95) {
    // Network apocalypse screen flash
    gfx->fillRect(0, 0, gfx->width(), 6, gfx->color565(255, 0, 255));
  }
  
  gfx->setCursor(5, gfx->height() - 15);
  gfx->setTextColor(gfx->color565(100, 100, 100));
  if (showTitles) {
    if (currentPhase < 5) {
      gfx->printf("XSCREENSNET - Phase %d", currentPhase + 1);
    } else {
      gfx->printf("CONNECTION LOST");
    }
  }
}

void drawBiosCatastrophe() {
  static float biosTime = 0;
  biosTime += 0.05; // Medium initial speed like System Error
  
  gfx->fillScreen(BLACK);
  
  // Typing animation with escalating speed based on hardware failure severity
  float typingSpeed = biosTime * 10; // Base speed
  int currentLine = (int)(typingSpeed / 50) % numBiosCatastropheLines;
  int currentChar = ((int)typingSpeed % 50);
  
  // Determine current phase for dramatic effects and speed changes
  int currentPhase = 0;
  int phaseStart = 0;
  
  for (int i = 0; i < numBiosCatastrophePhases; i++) {
    if (currentLine < biosCatastrophePhases[i]) {
      currentPhase = i;
      if (i > 0) {
        phaseStart = biosCatastrophePhases[i - 1];
      }
      break;
    }
  }
  
  // Speed up as hardware fails more catastrophically
  if (currentPhase >= 2) { // Critical failure phase
    typingSpeed = biosTime * 16; // Faster during critical failures
    currentLine = (int)(typingSpeed / 40) % numBiosCatastropheLines;
    currentChar = ((int)typingSpeed % 40);
  }
  
  if (currentPhase >= 4) { // Destruction phase - maximum speed
    typingSpeed = biosTime * 22; // Blazing fast during hardware death
    currentLine = (int)(typingSpeed / 35) % numBiosCatastropheLines;
    currentChar = ((int)typingSpeed % 35);
  }
  
  gfx->setTextSize(1);
  
  // Show current phase lines with scrolling
  int currentY = 5;
  int maxLinesOnScreen = 12; // Fit more lines for hardware info
  int startLine = max(0, currentLine - maxLinesOnScreen + 1);
  int endLine = min(currentLine + 1, numBiosCatastropheLines);
  
  for (int line = startLine; line < endLine && currentY < gfx->height() - 15; line++) {
    const char* sessionLine = biosCatastropheSession[line];
    uint16_t lineColor = gfx->color565(200, 200, 200); // Default gray
    
    // Color coding based on hardware status and escalation
    if (strstr(sessionLine, "Phoenix BIOS") != NULL || strstr(sessionLine, "POST:") != NULL) {
      // BIOS header - cyan
      lineColor = gfx->color565(0, 255, 255);
    } else if (strstr(sessionLine, "CPU:") != NULL || strstr(sessionLine, "RAM:") != NULL || 
               strstr(sessionLine, "GPU:") != NULL) {
      // Hardware detection - green
      lineColor = gfx->color565(0, 255, 0);
    } else if (strstr(sessionLine, "OK") != NULL || strstr(sessionLine, "PASSED") != NULL ||
               strstr(sessionLine, "SUCCESS") != NULL) {
      // Good status - bright green
      lineColor = gfx->color565(100, 255, 100);
    } else if (strstr(sessionLine, "WARNING:") != NULL) {
      // Warnings - yellow
      lineColor = gfx->color565(255, 255, 0);
    } else if (strstr(sessionLine, "Temperature:") != NULL && currentPhase >= 1) {
      // Temperature readings - escalating color based on phase
      if (currentPhase >= 4) {
        lineColor = gfx->color565(255, 0, 0); // Red hot
      } else if (currentPhase >= 2) {
        lineColor = gfx->color565(255, 100, 0); // Orange hot
      } else {
        lineColor = gfx->color565(255, 255, 0); // Yellow warm
      }
    } else if (strstr(sessionLine, "CRITICAL:") != NULL) {
      // Critical alerts - flashing red
      lineColor = gfx->color565(255, 0, 0);
      if (sin(biosTime * 12) > 0.4) lineColor = gfx->color565(255, 150, 150); // Flash
    } else if (strstr(sessionLine, "FAILURE") != NULL || strstr(sessionLine, "FAILED") != NULL) {
      // Failures - bright red
      lineColor = gfx->color565(255, 50, 50);
    } else if (strstr(sessionLine, "CATASTROPHE") != NULL || 
               strstr(sessionLine, "HARDWARE CATASTROPHE") != NULL) {
      // Major disaster - strobing white/red
      if (sin(biosTime * 20) > 0.5) {
        lineColor = gfx->color565(255, 255, 255);
      } else {
        lineColor = gfx->color565(255, 0, 0);
      }
    } else if (strstr(sessionLine, "DESTROYED") != NULL || strstr(sessionLine, "DEAD") != NULL ||
               strstr(sessionLine, "melted") != NULL) {
      // Complete destruction - flashing magenta
      if (sin(biosTime * 25) > 0.3) {
        lineColor = gfx->color565(255, 0, 255);
      } else {
        lineColor = gfx->color565(150, 0, 150);
      }
    } else if (strstr(sessionLine, "R.I.P.") != NULL || 
               strstr(sessionLine, "POST FAILED") != NULL) {
      // Death messages - dim gray
      lineColor = gfx->color565(100, 100, 100);
    }
    
    gfx->setCursor(5, currentY);
    gfx->setTextColor(lineColor);
    
    // Handle line length
    if (strlen(sessionLine) > 38) {
      char truncated[39];
      strncpy(truncated, sessionLine, 38);
      truncated[38] = '\0';
      gfx->printf("%s", truncated);
    } else {
      gfx->printf("%s", sessionLine);
    }
    
    currentY += 10; // Tighter spacing for more hardware info
  }
  
  // Draw current line being typed
  if (currentLine < numBiosCatastropheLines && currentY < gfx->height() - 15) {
    const char* currentBiosSession = biosCatastropheSession[currentLine];
    int lineLength = strlen(currentBiosSession);
    int charsToShow = min(currentChar, min(lineLength, 38));
    
    char buffer[40];
    strncpy(buffer, currentBiosSession, charsToShow);
    buffer[charsToShow] = '\0';
    
    // Typing color based on hardware failure phase
    uint16_t typingColor;
    if (currentPhase <= 1) {
      typingColor = gfx->color565(255, 255, 255); // White normal POST
    } else if (currentPhase == 2) {
      typingColor = gfx->color565(255, 255, 100); // Yellow warnings
    } else if (currentPhase == 3) {
      typingColor = gfx->color565(255, 150, 100); // Orange critical
    } else if (currentPhase == 4) {
      typingColor = gfx->color565(255, 100, 100); // Red destruction
    } else {
      typingColor = gfx->color565(100, 100, 100); // Gray death
    }
    
    gfx->setCursor(5, currentY);
    gfx->setTextColor(typingColor);
    gfx->printf("%s", buffer);
    
    // Cursor blink - faster as hardware dies
    float blinkSpeed = 6 + (currentPhase * 4); // Accelerating blink
    if (sin(biosTime * blinkSpeed) > 0) {
      gfx->printf("_");
    }
  }
  
  // Hardware failure visual effects - escalating chaos
  if (currentPhase >= 1) { // Warning phase - minor glitches
    for (int glitch = 0; glitch < currentPhase; glitch++) {
      int glitchX = random(gfx->width());
      int glitchY = random(gfx->height() - 20);
      uint16_t glitchColor = gfx->color565(255, 255, 0); // Yellow electrical problems
      gfx->drawPixel(glitchX, glitchY, glitchColor);
    }
  }
  
  if (currentPhase >= 2) { // Critical phase - more serious corruption
    for (int critical = 0; critical < currentPhase * 2; critical++) {
      int criticalX = random(gfx->width());
      int criticalY = random(gfx->height() - 20);
      uint16_t criticalColor = gfx->color565(255, random(150), 0); // Orange heat damage
      gfx->drawPixel(criticalX, criticalY, criticalColor);
    }
  }
  
  if (currentPhase >= 3) { // Meltdown phase - severe hardware damage
    // Horizontal lines representing circuit traces burning
    for (int burn = 0; burn < 3; burn++) {
      int burnY = random(gfx->height() - 20);
      int burnX = random(gfx->width() - 25);
      gfx->drawLine(burnX, burnY, burnX + 25, burnY, gfx->color565(255, 0, 0));
    }
  }
  
  if (currentPhase >= 4) { // Destruction phase - total visual chaos
    // Random pixels representing complete system failure
    for (int chaos = 0; chaos < 10; chaos++) {
      int chaosX = random(gfx->width());
      int chaosY = random(gfx->height());
      uint16_t chaosColor = gfx->color565(random(255), 0, random(255));
      gfx->drawPixel(chaosX, chaosY, chaosColor);
    }
    
    // Screen corruption rectangles - hardware literally dying
    if (sin(biosTime * 30) > 0.7) {
      for (int corrupt = 0; corrupt < 4; corrupt++) {
        int corruptX = random(gfx->width() - 10);
        int corruptY = random(gfx->height() - 10);
        gfx->fillRect(corruptX, corruptY, 10, 8, gfx->color565(255, random(100), random(255)));
      }
    }
  }
  
  if (currentPhase >= 5) { // Final death phase - screen glitching out completely
    // Total screen corruption as hardware dies
    if (sin(biosTime * 40) > 0.5) {
      for (int death = 0; death < 15; death++) {
        int deathX = random(gfx->width());
        int deathY = random(gfx->height());
        uint16_t deathColor = gfx->color565(random(255), random(255), random(255));
        gfx->drawPixel(deathX, deathY, deathColor);
      }
    }
    
    // Final screen flash as display circuits fail
    if (sin(biosTime * 5) > 0.98) {
      gfx->fillRect(0, 0, gfx->width(), 4, gfx->color565(255, 255, 255));
    }
  }
  
  gfx->setCursor(5, gfx->height() - 15);
  gfx->setTextColor(gfx->color565(100, 100, 100));
  if (showTitles) {
    if (currentPhase < 5) {
      const char* phaseNames[] = {"POST", "WARNING", "CRITICAL", "MELTDOWN", "DESTRUCTION", "DEAD"};
      gfx->printf("BIOS - %s", phaseNames[currentPhase]);
    } else {
      gfx->printf("HARDWARE DEAD");
    }
  }
}

void drawVibeCodeDisaster() {
  static float vibeTime = 0;
  vibeTime += 0.05; // Faster, more engaging pacing
  
  gfx->fillScreen(BLACK);
  
  // Typing animation with variable speed based on content
  float typingSpeed = vibeTime * 10; // Faster base speed
  int currentLine = (int)(typingSpeed / 45);
  
  // Add restart mechanism - cycle back to beginning after reaching end
  if (currentLine >= numVibeCodeLines) {
    vibeTime = 0; // Restart from beginning
    currentLine = 0;
  }
  
  int currentChar = ((int)typingSpeed % 45);
  
  // Determine current phase for dramatic effects
  int currentPhase = 0;
  int phaseStart = 0;
  
  for (int i = 0; i < numVibeCodePhases; i++) {
    if (currentLine < vibeCodePhases[i]) {
      currentPhase = i;
      if (i > 0) {
        phaseStart = vibeCodePhases[i - 1];
      }
      break;
    }
  }
  
  // Speed up during meltdown phases
  if (currentPhase >= 5) { // Meltdown and beyond
    typingSpeed = vibeTime * 18; // Much faster during chaos
    currentLine = (int)(typingSpeed / 30);
    
    // Add restart for fast phase too
    if (currentLine >= numVibeCodeLines) {
      vibeTime = 0; // Restart from beginning
      currentLine = 0;
    }
    
    currentChar = ((int)typingSpeed % 30);
  }
  
  gfx->setTextSize(1);
  
  // Show current phase lines with scrolling
  int currentY = 10;
  int maxLinesOnScreen = 11; // Fit nicely on M5StickC Plus2
  int startLine = max(0, currentLine - maxLinesOnScreen + 1);
  int endLine = min(currentLine + 1, numVibeCodeLines);
  
  for (int line = startLine; line < endLine && currentY < gfx->height() - 15; line++) {
    const char* sessionLine = vibeCodeSession[line];
    uint16_t lineColor = gfx->color565(200, 200, 200); // Default gray
    
    // Color coding based on content and phase
    if (strstr(sessionLine, "XScreensAI:") != NULL) {
      // AI responses - calm blue initially, then urgent colors
      if (currentPhase < 3) {
        lineColor = gfx->color565(100, 200, 255); // Calm blue
      } else if (currentPhase < 5) {
        lineColor = gfx->color565(255, 200, 100); // Warning orange
      } else {
        lineColor = gfx->color565(255, 100, 100); // Panic red
      }
    } else if (strstr(sessionLine, "User:") != NULL) {
      // User messages - escalating frustration
      if (currentPhase < 2) {
        lineColor = gfx->color565(100, 255, 100); // Happy green
      } else if (currentPhase < 4) {
        lineColor = gfx->color565(255, 255, 100); // Concerned yellow
      } else if (currentPhase < 6) {
        lineColor = gfx->color565(255, 150, 0); // Frustrated orange
      } else {
        lineColor = gfx->color565(255, 0, 0); // Angry red
        // Add flicker for extreme anger
        if (sin(vibeTime * 15) > 0.5) {
          lineColor = gfx->color565(255, 100, 100);
        }
      }
    } else if (strstr(sessionLine, "#include") != NULL || 
               strstr(sessionLine, "class") != NULL ||
               strstr(sessionLine, "{") != NULL ||
               strstr(sessionLine, "}") != NULL) {
      // Code - syntax highlighting
      lineColor = gfx->color565(150, 255, 150); // Light green code
    } else if (strstr(sessionLine, "✅") != NULL || strstr(sessionLine, "✨") != NULL) {
      // Success messages
      lineColor = gfx->color565(0, 255, 0); // Bright green
    } else if (strstr(sessionLine, "⚠️") != NULL || strstr(sessionLine, "Warning") != NULL) {
      // Warnings
      lineColor = gfx->color565(255, 255, 0); // Yellow
    } else if (strstr(sessionLine, "rm -rf") != NULL) {
      // Deletion commands - ominous
      lineColor = gfx->color565(255, 0, 255); // Magenta destruction
      if (sin(vibeTime * 20) > 0.3) {
        lineColor = gfx->color565(200, 0, 200); // Intense flicker
      }
    } else if (strstr(sessionLine, "CRITICAL ERROR") != NULL ||
               strstr(sessionLine, "KERNEL PANIC") != NULL ||
               strstr(sessionLine, "DELETED") != NULL ||
               strstr(sessionLine, "CORRUPTION") != NULL) {
      // System errors - flashing red
      lineColor = gfx->color565(255, 0, 0);
      if (sin(vibeTime * 25) > 0.2) {
        lineColor = gfx->color565(255, 150, 150);
      }
    } else if (strstr(sessionLine, "Connection lost") != NULL ||
               strstr(sessionLine, "System offline") != NULL) {
      // Final messages - dim
      lineColor = gfx->color565(100, 100, 100);
    } else if (strstr(sessionLine, "HAVE A NICE DAY") != NULL) {
      // Sarcastic AI farewell - rainbow cycle
      float hue = fmod(vibeTime * 120, 360);
      if (hue < 120) lineColor = gfx->color565(255, hue * 255/120, 0);
      else if (hue < 240) lineColor = gfx->color565(255 - (hue-120)*255/120, 255, 0);
      else lineColor = gfx->color565(0, 255, (hue-240)*255/120);
    }
    
    gfx->setCursor(5, currentY);
    gfx->setTextColor(lineColor);
    
    // Handle line length for M5StickC Plus2 - same as other modes
    if (strlen(sessionLine) > 38) { // Max chars that fit - same as Terminal Hacker
      char truncated[39];
      strncpy(truncated, sessionLine, 38);
      truncated[38] = '\0';
      gfx->printf("%s", truncated);
    } else {
      gfx->printf("%s", sessionLine);
    }
    
    currentY += 11;
  }
  
  // Draw current line being typed (only if we're still in bounds)
  if (currentLine < numVibeCodeLines && currentY < gfx->height() - 15) {
    const char* currentVibeSession = vibeCodeSession[currentLine];
    int lineLength = strlen(currentVibeSession);
    int charsToShow = min(currentChar, min(lineLength, 38)); // Same as other modes
    
    char buffer[40];
    strncpy(buffer, currentVibeSession, charsToShow);
    buffer[charsToShow] = '\0';
    
    // Typing color based on phase
    uint16_t typingColor;
    if (currentPhase <= 2) {
      typingColor = gfx->color565(255, 255, 255); // White calm
    } else if (currentPhase <= 4) {
      typingColor = gfx->color565(255, 255, 100); // Yellow concern
    } else if (currentPhase <= 6) {
      typingColor = gfx->color565(255, 150, 100); // Orange frustration
    } else {
      typingColor = gfx->color565(255, 100, 100); // Red panic
    }
    
    gfx->setCursor(5, currentY);
    gfx->setTextColor(typingColor);
    gfx->printf("%s", buffer);
    
    // Cursor blink - faster during panic
    float blinkSpeed = (currentPhase >= 5) ? 15 : 8;
    if (sin(vibeTime * blinkSpeed) > 0) {
      gfx->printf("_");
    }
  }
  
  // Visual chaos effects during deletion phases
  if (currentPhase >= 6) { // Deletion phase and beyond
    // Screen corruption during file deletion
    for (int corrupt = 0; corrupt < (currentPhase - 5) * 3; corrupt++) {
      int corruptX = random(gfx->width());
      int corruptY = random(gfx->height() - 20);
      uint16_t corruptColor = gfx->color565(255, random(100), random(100));
      gfx->drawPixel(corruptX, corruptY, corruptColor);
    }
  }
  
  // Extreme corruption during final system deletion
  if (currentPhase >= 8) { // Catastrophic phase
    if (sin(vibeTime * 30) > 0.7) {
      for (int chaos = 0; chaos < 10; chaos++) {
        int chaosX = random(gfx->width());
        int chaosY = random(gfx->height());
        uint16_t chaosColor = gfx->color565(random(255), 0, random(255));
        gfx->drawPixel(chaosX, chaosY, chaosColor);
      }
    }
  }
  
  gfx->setCursor(5, gfx->height() - 15);
  gfx->setTextColor(gfx->color565(100, 100, 100));
  if (showTitles) {
    if (currentPhase < 9) {
      gfx->printf("VIBE CODE");
    } else {
      gfx->printf("SYSTEM DELETED");
    }
  }
}

void handleInput() {
  M5.update();
  
  // Button A switches modes
  if (M5.BtnA.wasPressed()) {
    lastPress = millis();
    currentMode = (ScreensaverMode)((currentMode + 1) % MODE_COUNT);
    Serial.printf("Switched to mode %d\n", currentMode);
    
    // Reset animation time when switching
    animationTime = 0;
    
    // Show mode switch feedback
    gfx->fillScreen(BLACK);
    gfx->setTextColor(WHITE);
    gfx->setTextSize(2); // Double the font size
    gfx->setCursor(30, 50); // Adjust position for larger text
    if (currentMode == XJACK_ORIGINAL) {
      gfx->printf("Original XJack");
    } else if (currentMode == BACK_ROOM) {
      gfx->printf("Back Room");
    } else if (currentMode == DOGS_MOTORBIKES) {
      gfx->printf("Dogs & Motorbikes");
    } else if (currentMode == SYSTEM_ERROR) {
      gfx->printf("System Error");
    } else if (currentMode == SYMBOL_STORM) {
      gfx->printf("Symbol Storm");
    } else if (currentMode == TERMINAL_HACKER) {
      gfx->printf("Terminal Hacker");
    } else if (currentMode == HACKED_TERMINAL) {
      gfx->printf("Hacked Terminal");
    } else if (currentMode == XTREME_HACK) {
      gfx->printf("Xtreme Hack");
    } else if (currentMode == KERNEL_PANIC) {
      gfx->printf("Kernel Panic");
    } else if (currentMode == CRASH_GRAMMAR) {
      gfx->printf("Crash Grammar");
    } else if (currentMode == BIOS_CATASTROPHE) {
      gfx->printf("BIOS Catastrophe");
    } else if (currentMode == NETWORK_APOCALYPSE) {
      gfx->printf("Net Apocalypse");
    } else if (currentMode == VIBE_CODE_FAIL) {
      gfx->printf("Vibe Code Fail");
    }
    delay(1000); // Back to 1 second for title display
  }
  
  // Button B toggles auto-scrolling
  if (M5.BtnB.wasPressed()) {
    lastPress = millis();
    autoScroll = !autoScroll;
    autoScrollTimer = millis(); // Reset timer when toggling
    Serial.printf("Auto-scroll %s\n", autoScroll ? "ENABLED" : "DISABLED");
    
    // Show auto-scroll toggle feedback
    gfx->fillScreen(BLACK);
    gfx->setTextColor(WHITE);
    gfx->setTextSize(1);
    gfx->setCursor(50, 60);
    gfx->printf("Auto-scroll %s", autoScroll ? "ON" : "OFF");
    if (autoScroll) {
      gfx->setCursor(30, 80);
      gfx->printf("Switches every 15s");
    }
    delay(1500);
  }
  
  // Power Button (PWR) toggles title display
  if (M5.BtnPWR.wasPressed()) {
    lastPress = millis();
    showTitles = !showTitles;
    Serial.printf("Title display %s\n", showTitles ? "ENABLED" : "DISABLED");
    
    // Show title toggle feedback
    gfx->fillScreen(BLACK);
    gfx->setTextColor(WHITE);
    gfx->setTextSize(1);
    gfx->setCursor(50, 60);
    gfx->printf("Titles %s", showTitles ? "ON" : "OFF");
    delay(1000);
  }
  
  // Auto-scroll if enabled and no recent button press
  if (autoScroll && millis() - lastPress > AUTO_SCROLL_INTERVAL) {
    if (millis() - autoScrollTimer > AUTO_SCROLL_INTERVAL) {
      currentMode = (ScreensaverMode)((currentMode + 1) % MODE_COUNT);
      autoScrollTimer = millis();
      Serial.printf("Auto-switched to mode %d\n", currentMode);
    }
  }
}

void loop() {
  handleInput();
  
  // Update animation time
  animationTime += 0.016; // ~60 FPS
  
  // Draw current screensaver
  switch (currentMode) {
    case XJACK_ORIGINAL:
      drawXjackOriginal();
      break;
    case BACK_ROOM:
      drawXjackCustom();
      break;
    case DOGS_MOTORBIKES:
      drawDogsMotorbikes();
      break;
    case SYSTEM_ERROR:
      drawSystemError();
      break;
    case SYMBOL_STORM:
      drawAsciiArt();
      break;
    case TERMINAL_HACKER:
      drawTerminalHacker();
      break;
    case HACKED_TERMINAL:
      drawHackedTerminal();
      break;
    case XTREME_HACK:
      drawXtremeHack();
      break;
    case KERNEL_PANIC:
      drawKernelPanic();
      break;
    case CRASH_GRAMMAR:
      drawCrashGrammar();
      break;
    case BIOS_CATASTROPHE:
      drawBiosCatastrophe();
      break;
    case NETWORK_APOCALYPSE:
      drawNetworkApocalypse();
      break;
    case VIBE_CODE_FAIL:
      drawVibeCodeDisaster();
      break;
  }
  
  delay(16); // ~60 FPS
}