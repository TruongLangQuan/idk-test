#include <M5StickCPlus2.h>
#include "UNIT_MiniJoyC.h"
#include "tet.h"
#include "wifi_beacon.h"
#include "wifi_scanner.h"
#include <IRsend.h>  // IR library for remote control
#include <WiFi.h>
#include <WiFiUdp.h>

#define Disp M5.Lcd
#define BITMAP M5.Lcd.drawBitmap(0,0,135,240,tet)
#define POS_X 0
#define POS_Y 1

// IR Remote setup - using built-in IR emitter on GPIO19
IRsend irsend(19);

// Function declarations
void blockDraw(int y, int x, int colorz);
void blockDrawCascade(int y, int x, int colorz); // CASCADE version with neon outline
void bloff(int y,int x, int brightz);
void clearLines();
void newPiece(boolean setPiece);
void wait();
void moveLeft();
void moveDown();
void moveRight();
void plummet();
void rotateLeft();
void rotateRight();
void pausegame();
void reset();
void showField(int startz, int finish,boolean showpiece);
boolean test(int y, int x);
void ButtonLoop(int _btn);
void scoreBoardWrite(int scorez);
int levelSelect();
void drawGhostPiece();
void clearGhostPiece();
void drawGameBorder(); // New function for drawing border once

// Enhanced Tetris feature declarations
void holdPiece();
void drawHoldPiece();
void drawNextPiece();
void softDrop();

// Network Battle feature declarations  
bool setupTetrisBattle();
void sendBattleMessage(uint8_t type, float data1 = 0, float data2 = 0);
void receiveBattleMessages();
void addGarbageRows(int rows);
void drawOpponentDanger(int dangerLevel);
void checkGameOver();
bool waitForBothPlayersReady();
void drawReadyScreen();

// CASCADE SYSTEM FUNCTIONS - V1.3
void processCascadePhysics();
void cascadeBlockDrop(int startY, int startX);
boolean canCascadeBlockFall(int y, int x);
void toggleCascadeMode();

// Add IR Remote function declarations
void irRemoteControl();
void sendUniversalPower();
int calculateDropDistance();
int check_Btn();
int check_Up();
int check_Right();
int check_Left();
int check_Down();

#define offsetx 14     //pixel offset X
#define offsety 20     //pixel offset Y
#define blockSize 11  //block size
#define fieldx 10     //how many blocks wide (Standard is 10)
#define fieldy 20     //how many blocks tall (Standard is 20)

// FIXED PIECE DATA - ALL PIECES NOW ROTATE CLOCKWISE CONSISTENTLY
// Format: piece[type][rotation][y/x][4_blocks] where rotation 0→1→2→3 = clockwise progression
int piece[7][4][2][4]={{{{0,1,0,1},{0,0,1,1}},{{0,1,0,1},{0,0,1,1}},{{0,1,0,1},{0,0,1,1}},{{0,1,0,1},{0,0,1,1}}},     // O-piece (square - same all rotations)
                        {{{0,-1,0,1},{0,0,1,1}},{{0,1,1,0},{0,0,-1,1}},{{0,-1,0,1},{0,0,1,1}},{{0,1,1,0},{0,0,-1,1}}},     // I-piece (line)  
                        {{{0,-1,0,1},{0,0,-1,-1}},{{0,0,1,1},{0,-1,0,1}},{{0,-1,0,1},{0,0,-1,-1}},{{0,0,1,1},{0,-1,0,1}}}, // S-piece (zigzag)
                        {{{1,0,-1,1},{0,0,0,-1}},{{0,-1,0,0},{0,0,1,2}},{{0,1,2,0},{0,0,0,1}},{{1,0,0,0},{1,1,0,-1}}},     // T-piece (T-shape)
                        {{{1,0,0,0},{-1,-1,0,1}},{{0,0,1,2},{-1,0,0,0}},{{-1,0,0,0},{1,1,0,-1}},{{1,1,0,-1},{1,0,0,0}}},   // L-piece (FIXED ORDER)
                        {{{1,0,-1,0},{0,0,0,1}},{{0,0,0,1},{-1,0,1,0}},{{1,0,-1,0},{0,0,0,-1}},{{0,0,0,-1},{-1,0,1,0}}},   // J-piece (FIXED ORDER)
                        {{{0,0,0,0},{-1,0,1,2}},{{-1,0,1,2},{0,0,0,0}},{{0,0,0,0},{-1,0,1,2}},{{-1,0,1,2},{0,0,0,0}}}};   // Z-piece (reverse zigzag)
// piece[block][rot][y/x][4 spots] - NOW ALL PIECES ROTATE CLOCKWISE WITH rot++

int score;
byte multiplier;
byte digit[10][7];
#define dLength 4
const int cLength = dLength*7;
int pc;
int rot;
int block;
int color;
int level;
byte music;
long unsigned speed;
long unsigned timez;
long unsigned downHold,leftHold,rightHold;
boolean collide;
boolean game;
boolean release;
int posX;
int posY;
int a, b, c, f, g, h,i,j,k;
int linePos[4];
  int linez;
    int blockposY;
    int blockposX;
  int lineCount;
  
int stage;

UNIT_JOYC Joystick;

int pins[8];
boolean buttons[8];
boolean hold[8];

byte field[fieldy][fieldx];
boolean cascadeField[fieldy][fieldx]; // Track which blocks are cascade pieces

// HOLD piece system variables
int heldPiece = -1;        // -1 = no piece held, 0-6 = piece type
boolean canUseHold = true; // Can only use HOLD once per piece
boolean holdUsed = false;  // Track if HOLD was used this piece

// CASCADE SYSTEM - V1.3
boolean isCascadePiece = false;  // Is current piece a cascade piece?
int cascadePieceCounter = 0;     // Counter for spawn ratio (1 cascade per 3 normal)
boolean heldPieceIsCascade = false; // Track if held piece is cascade
boolean cascadeEnabled = true;   // Toggle cascade mode on/off

// NEXT piece system variables
int nextPiece = -1;        // The next piece to spawn
boolean nextPieceIsCascade = false; // Is next piece cascade?
int lastDrawnNextPiece = -2; // Track what was last drawn to prevent unnecessary redraws

// Lock delay system variables
unsigned long lockStartTime = 0; // When piece first touched ground
boolean pieceOnGround = false;   // Is piece currently on ground
int lockResets = 0;             // Number of times lock delay was reset
const int MAX_LOCK_RESETS = 15; // Maximum lock resets before forced lock

// Network battle variables
const char* tetris_ssid = "M5Tetris_Battle";
const char* tetris_password = "tetris123";
const int tetris_port = 7777;

WiFiUDP battleUDP;
IPAddress remoteIP;
bool isHost = false;
bool isBattleMode = false;
bool isConnected = false;
unsigned long lastHeartbeat = 0;
int opponentDangerLevel = 0;
bool gameOver = false;

// Ready-up synchronization variables
bool localPlayerReady = false;
bool remotePlayerReady = false;
bool bothPlayersReady = false;
int selectedLevel = 0;
int remoteSelectedLevel = 0; // Track opponent's selected level

// Battle message structure
struct BattleMessage {
  uint8_t type; // 0=heartbeat, 1=line_clear_attack, 2=danger_level, 3=game_over, 4=ready_up, 5=start_game
  float data1;
  float data2;
  uint32_t timestamp;
};

// Variable lock delay based on level - Progressive for better playability
int getLockDelay() {
  if (level <= 5) return 350;      // Levels 0-5: 350ms (less delay for beginners)
  else if (level <= 9) return 450; // Levels 6-9: 450ms (moderate delay)
  else if (level <= 15) return 650; // Levels 10-15: 650ms (more time to think)
  else return 725;                 // Levels 16-19: 725ms (max time for impossible speeds)
}

void setup() {
  
  M5.begin();
  
  // Basic power management - disable deep sleep for better wake-up behavior  
  esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_ALL);
  
  // Initialize IR sender
  irsend.begin();
  
  posX=4;
  posY=4;

  pinMode(35,INPUT_PULLUP);
  pinMode(37,INPUT_PULLUP);
  pinMode(39,INPUT_PULLUP);

  Disp.setRotation(0);
  BITMAP;
  Disp.fillRect(0,0,135,18,BLACK);
  //M5.Lcd.setTextSize(2);
  //M5.Lcd.setFont(&fonts::FreeSerifBoldItalic9pt7b);
  //M5.Lcd.drawString("C+2",37,77);
  M5.Lcd.setTextSize(1);
  M5.Lcd.drawString("v.1.4.0",4,8,1);
  M5.Lcd.drawString((String)M5.Power.getBatteryLevel()+"%",107,8,1);
  M5.Lcd.drawString("Loading...",39,134,1);
  while (!(Joystick.begin(&Wire, JoyC_ADDR, 0, 26, 100000UL))) {
    delay(100);
    Serial.println("I2C Error!\r\n");
  }
  M5.Lcd.fillRect(39,134,100,10,BLACK);
  M5.Lcd.drawString(" Press M5 ",39,134,1);
  M5.Lcd.drawString("L+R:Beacon Up:Scanner",5,150,1);
  M5.Lcd.drawString("Down:Battle Mode",8,165,1);
  M5.Lcd.drawString("Game has Level Select!",8,180,1);
  
  // Add "CASCADE TETRIS BATTLE!" in bigger font above "Press M5"
  M5.Lcd.setTextColor(0x07FF); // Cyan color  
  M5.Lcd.setTextSize(1);        // Normal font size
  M5.Lcd.drawString("CASCADE", 25, 100, 1);
  M5.Lcd.drawString("TETRIS BATTLE!", 15, 115, 1);
  M5.Lcd.setTextSize(1);        // Reset to normal size
  while (digitalRead(37)==1) {
    // Check for WiFi beacon mode (both side buttons pressed)
    if(digitalRead(35)==0 && digitalRead(39)==0) {
      delay(500); // Hold time to confirm
      if(digitalRead(35)==0 && digitalRead(39)==0) {
        beaconSpam();
        // After WiFi beacon, redraw the screen
        Disp.fillScreen(BLACK);
        M5.Lcd.drawLine(0,16,135,16,WHITE);
        M5.Lcd.fillRect(39,134,100,10,BLACK);
        M5.Lcd.drawString(" Press M5 ",39,134,1);
        M5.Lcd.drawString("L+R:Beacon Up:Scanner",5,150,1);
        M5.Lcd.drawString("Game has Level Select!",8,165,1);
      }
    }
    
    // Check for WiFi scanner mode (Up on joystick)
    if(Joystick.getADCValue(1) > 2950) {
      delay(500); // Hold time to confirm
      if(Joystick.getADCValue(1) > 2950) {
        wifiScanner();
        // After WiFi scanner, redraw the screen
        Disp.fillScreen(BLACK);
        M5.Lcd.drawLine(0,16,135,16,WHITE);
        M5.Lcd.fillRect(39,134,100,10,BLACK);
        M5.Lcd.drawString(" Press M5 ",39,134,1);
        M5.Lcd.drawString("L+R:Beacon Down:IR Remote",2,150,1);
        M5.Lcd.drawString("Game has Level Select!",8,165,1);
        
        // Redraw "HAPPY HALLOWEEN!!" after WiFi scanner
        M5.Lcd.setTextColor(0xF800);
        M5.Lcd.setTextSize(2);
        M5.Lcd.drawString("HAPPY", 30, 100, 1);
        M5.Lcd.drawString("HALLOWEEN!!", 0, 118, 1);
        M5.Lcd.setTextSize(1);
      }
    }
    
    // Check for Battle Mode (Down on joystick) 
    if(Joystick.getADCValue(1) < 1600) {
      delay(500); // Hold time to confirm
      if(Joystick.getADCValue(1) < 1600) {
        if(setupTetrisBattle()) {
          // Battle mode setup successful, now wait for both players to ready up
          if(waitForBothPlayersReady()) {
            // Both players ready, continue to game
            break;
          }
        }
        // Battle setup failed, redraw screen
        Disp.fillScreen(BLACK);
        M5.Lcd.drawLine(0,16,135,16,WHITE);
        M5.Lcd.fillRect(39,134,100,10,BLACK);
        M5.Lcd.drawString(" Press M5 ",39,134,1);
        M5.Lcd.drawString("L+R:Beacon Up:Scanner",5,150,1);
        M5.Lcd.drawString("Down:Battle Mode",8,165,1);
        M5.Lcd.drawString("Game has Level Select!",8,180,1);
        
        // Redraw "CASCADE TETRIS BATTLE!" after battle setup
        M5.Lcd.setTextColor(0x07FF);
        M5.Lcd.setTextSize(1);
        M5.Lcd.drawString("CASCADE", 25, 100, 1);
        M5.Lcd.drawString("TETRIS BATTLE!", 15, 115, 1);
        M5.Lcd.setTextSize(1);
      }
    }
    delay(30);
  }
  delay(300);

  Disp.fillScreen(BLACK);
  M5.Lcd.drawLine(0,16,135,16,WHITE);
  
  // Draw Halloween skull at bottom of opening screen (simple pixel art)
  // Small skull pattern at bottom center (around x=67, y=190-210)
  int skullX = 50; // Center horizontally
  int skullY = 190; // Near bottom
  
  // Skull outline (white pixels)
  M5.Lcd.drawPixel(skullX+2, skullY, WHITE);
  M5.Lcd.drawPixel(skullX+3, skullY, WHITE);
  M5.Lcd.drawPixel(skullX+4, skullY, WHITE);
  M5.Lcd.drawPixel(skullX+1, skullY+1, WHITE);
  M5.Lcd.drawPixel(skullX+5, skullY+1, WHITE);
  M5.Lcd.drawPixel(skullX, skullY+2, WHITE);
  M5.Lcd.drawPixel(skullX+6, skullY+2, WHITE);
  M5.Lcd.drawPixel(skullX, skullY+3, WHITE);
  M5.Lcd.drawPixel(skullX+6, skullY+3, WHITE);
  M5.Lcd.drawPixel(skullX+1, skullY+4, WHITE);
  M5.Lcd.drawPixel(skullX+5, skullY+4, WHITE);
  M5.Lcd.drawPixel(skullX+2, skullY+5, WHITE);
  M5.Lcd.drawPixel(skullX+4, skullY+5, WHITE);
  M5.Lcd.drawPixel(skullX+3, skullY+6, WHITE);
  
  // Eye sockets (red pixels for spooky effect)
  M5.Lcd.drawPixel(skullX+1, skullY+2, RED);
  M5.Lcd.drawPixel(skullX+5, skullY+2, RED);
  
  // Nasal cavity 
  M5.Lcd.drawPixel(skullX+3, skullY+3, RED);
  
  // Teeth/jaw
  M5.Lcd.drawPixel(skullX+2, skullY+4, WHITE);
  M5.Lcd.drawPixel(skullX+4, skullY+4, WHITE);
  
  // Add "HAPPY HALLOWEEN" text in orange at center of screen
  M5.Lcd.setTextColor(0xF800); // Orange color
  M5.Lcd.setTextSize(1);
  M5.Lcd.drawString("HAPPY", 50, 100, 1);      // Center horizontal, middle vertical
  M5.Lcd.drawString("HALLOWEEN", 35, 115, 1);  // Center horizontal, below HAPPY
}

void loop(){
  int startingLevel;
  
  if(isBattleMode) {
    // In battle mode, BOTH players select their own levels for individual skill
    startingLevel = levelSelect(); 
    selectedLevel = startingLevel; // Store for ready-up display
    
    // NOW WAIT FOR BOTH PLAYERS TO BE READY BEFORE STARTING GAME
    if(!waitForBothPlayersReady()) {
      // If battle connection lost, return to setup
      return;
    }
    
  } else {
    // Normal mode - show level select screen
    startingLevel = levelSelect();
  }
  
  // Calculate proper speed for selected level - make it VERY obvious
  // Level 0 = 1000ms (slow), Level 5 = 300ms (fast), Level 9 = 100ms (insane), Level 19 = 30ms (lightning)
  if(startingLevel == 0) speed = 1000;
  else if(startingLevel == 1) speed = 800;
  else if(startingLevel == 2) speed = 600;
  else if(startingLevel == 3) speed = 450;
  else if(startingLevel == 4) speed = 350;
  else if(startingLevel == 5) speed = 300;
  else if(startingLevel == 6) speed = 250;
  else if(startingLevel == 7) speed = 200;
  else if(startingLevel == 8) speed = 150;
  else if(startingLevel == 9) speed = 100;
  else if(startingLevel == 10) speed = 85;
  else if(startingLevel == 11) speed = 75;
  else if(startingLevel == 12) speed = 65;
  else if(startingLevel == 13) speed = 55;
  else if(startingLevel == 14) speed = 50;
  else if(startingLevel == 15) speed = 45;
  else if(startingLevel == 16) speed = 40;
  else if(startingLevel == 17) speed = 35;
  else if(startingLevel == 18) speed = 32;
  else if(startingLevel == 19) speed = 30;
  else speed = 750; // fallback
  
  timez=millis();
  score=0;
  
  // In battle mode, each player uses their own selected level
  if(isBattleMode) {
    level = selectedLevel; // Each player uses their own chosen level
    startingLevel = selectedLevel;
  } else {
    level = startingLevel; // Normal mode
  }
  
  stage=startingLevel / 10; // Set proper stage based on level
  lineCount=0; // Reset line counter for proper speed progression
  
  // In battle mode, synchronized countdown before game starts
  if(isBattleMode) {
    M5.Display.fillScreen(TFT_BLACK);
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.setCursor(20, 80);
    M5.Display.print("Battle Level: ");
    M5.Display.println(selectedLevel);
    M5.Display.setCursor(20, 100);
    M5.Display.println("Starting in...");
    
    // Clean countdown - no fillRect mess
    delay(1000);
    M5.Display.setCursor(60, 130);
    M5.Display.setTextSize(4);
    M5.Display.setTextColor(TFT_YELLOW);
    M5.Display.println("3");
    delay(1000);
    M5.Display.setCursor(60, 130);
    M5.Display.setTextColor(TFT_BLACK);
    M5.Display.println("3"); // Clear previous
    M5.Display.setTextColor(TFT_YELLOW);
    M5.Display.println("2");
    delay(1000);
    M5.Display.setCursor(60, 130);
    M5.Display.setTextColor(TFT_BLACK);
    M5.Display.println("2"); // Clear previous  
    M5.Display.setTextColor(TFT_YELLOW);
    M5.Display.println("1");
    delay(1000);
    M5.Display.setCursor(40, 130);
    M5.Display.setTextColor(TFT_BLACK);
    M5.Display.println("1"); // Clear previous
    M5.Display.setTextColor(TFT_GREEN);
    M5.Display.println("GO!");
    delay(500);
    M5.Display.setTextSize(1);
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.fillScreen(TFT_BLACK); // Clean slate for game
  }
  
  // Initialize HOLD and NEXT piece system
  heldPiece = -1;
  canUseHold = true;
  holdUsed = false;
  nextPiece = random(7);
  lastDrawnNextPiece = -2;
  
  // Initialize lock delay system
  lockStartTime = 0;
  pieceOnGround = false;
  lockResets = 0;
  
  newPiece(0);
  game=1;
  showField(0,fieldy,1);
  
  // Draw the game border ONCE at start - won't be redrawn
  drawGameBorder();
  
  // Draw initial HOLD and NEXT displays
  drawHoldPiece();
  drawNextPiece();
  while(game){
    // Handle network battle communication
    if(isBattleMode) {
      receiveBattleMessages();
      
      // Send periodic heartbeat  
      if(millis() - lastHeartbeat > 1000) {
        sendBattleMessage(0); // heartbeat
        lastHeartbeat = millis();
      }
      
      // Check for game over in battle mode
      checkGameOver();
    }
    
    collide=1;
    multiplier=1;
    scoreBoardWrite(score);
    
    // Update HOLD and NEXT displays if needed
    drawHoldPiece();
    if(nextPiece != lastDrawnNextPiece) {
      drawNextPiece();
      lastDrawnNextPiece = nextPiece;
    }
    
    // Draw opponent danger level in battle mode
    if(isBattleMode && opponentDangerLevel > 0) {
      drawOpponentDanger(opponentDangerLevel);
    }
      while(collide){
        wait();
        if(collide) moveDown();
      }
    if(game!=0)
    newPiece(1);
  }
  for(a=0;a<fieldy;a++){
    for(b=0;b<fieldx;b++){
      field[a][b]=0;
      cascadeField[a][b]=false; // CASCADE SYSTEM: Initialize cascade field
    }
  }
  delay(1500);
  M5.Lcd.drawString("            ",36,135,1); //Erase Reset msg
  M5.Lcd.drawString("Sco.:00     ",10,4,1);
}

//==============================JoyC==============================
int check_Btn(){
  if (Joystick.getButtonStatus()==0){
    return 0;
  }
  return 1;
}
int check_Up(){
  if (Joystick.getADCValue(POS_Y)>2950){
    return 0;
  }
  return 1;
}
int check_Right(){
  if ((Joystick.getADCValue(POS_X)>2950)){
    return 0;
  }
  return 1;
}
int check_Left(){
  if ((Joystick.getADCValue(POS_X)<1350)){
    return 0;
  }
  return 1;
}
int check_Down() {
  if ((Joystick.getADCValue(POS_Y)<1600)){
    return 0;
  }
  return 1;
}
//================================================================

void blockDraw(int y, int x, int colorz){
  // Standard Tetris colors
  int red=0;
  int gre=0;
  int blu=0;
  
  // Map colors to standard Tetris piece colors
  switch(colorz % 7) {
    case 0: // I-piece - Cyan
      red = 0; gre = 31; blu = 31;
      break;
    case 1: // O-piece - Yellow  
      red = 31; gre = 31; blu = 0;
      break;
    case 2: // T-piece - Purple
      red = 31; gre = 0; blu = 31;
      break;
    case 3: // S-piece - Green
      red = 0; gre = 31; blu = 0;
      break;
    case 4: // Z-piece - Red
      red = 31; gre = 0; blu = 0;
      break;
    case 5: // J-piece - Blue
      red = 0; gre = 0; blu = 31;
      break;
    case 6: // L-piece - Orange
      red = 31; gre = 15; blu = 0;
      break;
    default:
      red = 15; gre = 15; blu = 15; // Gray fallback
      break;
  }
  
  int returnz=(red*2048)+(gre*64)+blu;
  int posx=(x*blockSize)+offsetx;
  int posy=(y*blockSize)+offsety;
  Disp.drawRect(posx,posy,blockSize-1,blockSize-1,returnz);
  red/=2;
  gre/=2;
  blu/=2;
  returnz=(red*2048)+(gre*64)+blu;
  Disp.fillRect(posx+1,posy+1,blockSize-3,blockSize-3,returnz);
  Disp.drawPixel(posx,posy,WHITE);
  Disp.drawPixel(posx+1,posy+1,48599);
  Disp.drawPixel(posx+2,posy+1,48599);
  Disp.drawPixel(posx+1,posy+2,48599);
}

void blockDrawCascade(int y, int x, int colorz){
  // CASCADE PIECE - UNMISTAKABLY DIFFERENT with X marker
  
  int posx=(x*blockSize)+offsetx;
  int posy=(y*blockSize)+offsety;
  
  // BRIGHT CYAN OUTLINE (unique color not used in normal tetris)
  Disp.drawRect(posx,posy,blockSize-1,blockSize-1, 0x07FF); // Bright cyan outline
  
  // BLACK INTERIOR (high contrast)
  Disp.fillRect(posx+1,posy+1,blockSize-3,blockSize-3, 0x0000); // Black fill
  
  // BRIGHT YELLOW X MARKER in center (VERY OBVIOUS!)
  int centerX = posx + blockSize/2;
  int centerY = posy + blockSize/2;
  
  // Draw X with bright yellow pixels
  Disp.drawPixel(centerX-1, centerY-1, 0xFFE0); // Top-left
  Disp.drawPixel(centerX, centerY, 0xFFE0);     // Center
  Disp.drawPixel(centerX+1, centerY+1, 0xFFE0); // Bottom-right
  Disp.drawPixel(centerX+1, centerY-1, 0xFFE0); // Top-right  
  Disp.drawPixel(centerX-1, centerY+1, 0xFFE0); // Bottom-left
  
  // Additional X outline for visibility
  Disp.drawPixel(centerX-2, centerY-2, 0xFFE0);
  Disp.drawPixel(centerX+2, centerY+2, 0xFFE0);
  Disp.drawPixel(centerX+2, centerY-2, 0xFFE0);
  Disp.drawPixel(centerX-2, centerY+2, 0xFFE0);
  
  // Corner accent dots in cyan
  Disp.drawPixel(posx+1, posy+1, 0x07FF);
  Disp.drawPixel(posx+blockSize-2, posy+1, 0x07FF);
  Disp.drawPixel(posx+1, posy+blockSize-2, 0x07FF);
  Disp.drawPixel(posx+blockSize-2, posy+blockSize-2, 0x07FF);
}

void bloff(int y,int x, int brightz){
  Disp.fillRect((x*blockSize)+offsetx,(y*blockSize)+offsety,blockSize-1,blockSize-1,2113*brightz);
}

void clearLines(){
  linez=0;
  for(a=0;a<fieldy;a++){
    byte counter=0;
    for(b=0;b<fieldx;b++){
      counter+=field[a][b]>0;
    }
    if(counter==fieldx){
      linePos[linez]=a;
      linez++;
    }
  }
  if(linez==1){
    level++;
    score+=multiplier;
    for(a=0;a<fieldx;a++){
      for(b=31;b>=0;b-=1){
        bloff(linePos[0],a,b);
        delay(1);
      }
    }
    delay(200);
  }
  
  if(linez==2){
    level+=2;
    score+=3*multiplier;
    for(a=0;a<fieldx;a++){
      bloff(linePos[0],a,31);
      bloff(linePos[1],a,31);
      delay(50);
    }
    for(a=0;a<50;a++){
      bloff(linePos[random(2)],random(fieldx),0);
      delay(15);
    }
  }
  
  if(linez==3){
    level+=3;
    score+=8*multiplier;
    for(a=0;a<fieldx;a++){
      for(b=0;b<3;b++){
        blockDraw(linePos[b],a,(10*b));
      }
      delay(50);
    }
    for(a=0;a<fieldx;a++){
      for(b=0;b<3;b++){
        bloff(linePos[b],a,0);
      }
      delay(50);
    }
  }
  
  if(linez==4){
    level+=4;
    score+=20*multiplier;
    for(a=0;a<192;a+=1){
      for(b=linePos[0];b<linePos[3]+1;b++){
        for(c=0;c<fieldx;c++){
          blockDraw(b,c,(3*c)+(b*8)+a);
        }
      }
      delay(10);
    }
  }
  for(a=0;a<linez;a++){  //clear
    for(c=0;c<fieldx;c++){
      field[linePos[a]][c]=0;
      cascadeField[linePos[a]][c]=false; // CASCADE SYSTEM: Clear cascade data
    }
    for(b=linePos[a];b>0;b--){
      for(c=0;c<fieldx;c++){
        field[b][c]=field[b-1][c];
        cascadeField[b][c]=cascadeField[b-1][c]; // CASCADE SYSTEM: Move cascade data down
      }
    }
  }
  lineCount+=linez;
  
  // Send attack in battle mode
  if(isBattleMode && linez > 0) {
    int attackRows = 0;
    if(linez == 2) attackRows = 1;      // Double = 1 garbage row
    else if(linez == 3) attackRows = 2; // Triple = 2 garbage rows  
    else if(linez == 4) attackRows = 4; // Tetris = 4 garbage rows
    
    if(attackRows > 0) {
      sendBattleMessage(1, attackRows); // Send line clear attack
      
      // Visual feedback for sending attack
      M5.Lcd.fillRect(100, 200, 30, 10, TFT_RED);
      M5.Lcd.setTextColor(TFT_WHITE);  
      M5.Lcd.drawString("ATK!", 105, 200);
      delay(500); // Show attack indicator
      M5.Lcd.fillRect(100, 200, 30, 10, TFT_BLACK); // Clear
    }
  }
  
  if(lineCount>=10){
    lineCount-=10;
    speed=((speed*4)/5); // 20% faster (0.8x multiplier)
    if(speed < 80) speed = 80; // Minimum speed cap
  }
  showField(0,fieldy,0);
  // Simplified level progression - just every 10 levels increase stage and speed
  if(level > 0 && level % 10 == 0 && level > stage * 10) {
    stage++;
    speed=((speed*4)/5); // 20% faster 
    if(speed < 80) speed = 80; // Minimum speed cap
  }
  for(int a=0;a<linez;a++){
    for(int b=0;b<fieldx;b++){
      field[a][b]=0;
    }
  }
  timez=millis();
}
void newPiece(boolean setPiece){
  if(setPiece){
      for(a=0;a<4;a++){
        int blockY = posY+piece[block][rot][0][a];
        int blockX = posX+piece[block][rot][1][a];
        field[blockY][blockX] = color;
        cascadeField[blockY][blockX] = isCascadePiece; // Mark if this block is from cascade piece
      }
    clearLines();
    
    // CASCADE PHYSICS - After piece is set, check for cascade effects
    if(isCascadePiece) {
      processCascadePhysics();
    }
  }
  
  posY=1;
  posX=4;
  rot=0;
  
  // Use next piece system with CASCADE logic
  if(nextPiece == -1) {
    // First piece - generate randomly
    block = random(7);
    nextPiece = random(7);
    
    // Determine cascade status - 1 cascade per 3 normal pieces (randomized)
    cascadePieceCounter++;
    if(cascadeEnabled && cascadePieceCounter >= 2 && random(100) < 33) { // 33% chance after 2 pieces (1 in 3)
      isCascadePiece = true;
      cascadePieceCounter = 0;
    } else {
      isCascadePiece = false;
    }
    
    // Generate next piece cascade status
    if(cascadeEnabled && random(100) < 33) { // 33% chance for next piece to be cascade (1 in 3)
      nextPieceIsCascade = true;
    } else {
      nextPieceIsCascade = false;
    }
    
  } else {
    // Use the next piece and generate a new next piece
    block = nextPiece;
    isCascadePiece = nextPieceIsCascade;
    nextPiece = random(7); // Generate new next piece
    
    // Generate next piece cascade status
    cascadePieceCounter++;
    if(cascadeEnabled && cascadePieceCounter >= 2 && random(100) < 33) { // 33% chance after 2 pieces (1 in 3)
      nextPieceIsCascade = true;
      cascadePieceCounter = 0;
    } else {
      nextPieceIsCascade = false;
    }
  }
  
  // Set colors - CASCADE pieces get NEON GREEN outline
  if(isCascadePiece) {
    color = 32; // Neon green for cascade pieces
  } else {
    color = random(96) + 1; // Normal random colors
  }
  
  // Reset HOLD for new piece
  canUseHold = true;
  holdUsed = false;
  
  // Reset lock delay system
  lockStartTime = 0;
  pieceOnGround = false;
  lockResets = 0;
  
  showField(0,20,1);
  if(game!=0){
    game=test(0,0);    
  }
}

void wait(){
  timez+=speed;
  while(millis()<(timez+speed)){
    // CRITICAL: Process battle messages during wait loop!
    if(isBattleMode) {
      receiveBattleMessages();
    }
    
    if(millis()>(leftHold+150)){hold[0]=0;}
    if(millis()>(downHold+150)){hold[1]=0;}
    if(millis()>(rightHold+150)){hold[2]=0;}
    for(int d=0;d<8;d++){
      buttons[d]=0;
      ButtonLoop(d);
      if(!pins[d]&&!hold[d]){
        buttons[d]=1;
        hold[d]=1;
        delay(25);
      }
      if(pins[d]&&hold[d]){
        hold[d]=0;
        delay(25);
      }
    }
    if(buttons[0]){moveLeft();leftHold=millis();}
    if(buttons[1]){moveDown();downHold=millis();}
    if(buttons[2]){moveRight();rightHold=millis();}
    if(buttons[3]){rotateRight();}
    if(buttons[4]){holdPiece();} // BtnA = HOLD piece (FIXED)
    if(buttons[5]){plummet();}
    if(buttons[6]){pausegame();} // Power button = PAUSE game (FIXED)
    if(buttons[7]){toggleCascadeMode();} // BtnB = Toggle CASCADE mode
  }
}

void moveLeft(){
  int oldX = posX;
  posX-=test(0,-1);
  
  // Reset lock delay if piece moved while on ground
  if(pieceOnGround && posX != oldX && lockResets < MAX_LOCK_RESETS) {
    lockStartTime = millis();
    lockResets++;
  }
  
  showField(0,fieldy,1); // Full redraw to clear ghost trails
}

void moveDown(){
  if(test(1,0)){
    posY++;
    showField(0,fieldy,1); // Full redraw to clear ghost trails
    
    // Reset lock delay if piece was on ground but now can move
    if(pieceOnGround) {
      pieceOnGround = false;
      lockStartTime = 0;
      lockResets = 0;
    }
  } else{
    // Piece can't move down - implement lock delay
    if(!pieceOnGround) {
      // First time touching ground
      pieceOnGround = true;
      lockStartTime = millis();
    } else {
      // Already on ground, check if lock delay expired
      if(millis() - lockStartTime >= getLockDelay() || lockResets >= MAX_LOCK_RESETS) {
        collide=0; // Lock the piece
      }
    }
  }
  timez=millis();
}

void moveRight(){
  int oldX = posX;
  posX+=(test(0,1));
  
  // Reset lock delay if piece moved while on ground
  if(pieceOnGround && posX != oldX && lockResets < MAX_LOCK_RESETS) {
    lockStartTime = millis();
    lockResets++;
  }
  
  showField(0,fieldy,1); // Full redraw to clear ghost trails
}

void plummet(){
  multiplier=2;posY;
  while(collide){
    moveDown();
    delay(12);
  }
  timez-=speed;
}

void rotateLeft(){
  // ELIMINATED - All rotation is now clockwise only!
  // Just call rotateRight() instead
  rotateRight();
}

void rotateRight(){
  int oldRot = rot;
  rot++;
  rot%=4;
  if(!test(0,0)){
    rot+=4;
    rot--;
    rot%=4;
  } else {
    // Reset lock delay if piece rotated while on ground
    if(pieceOnGround && oldRot != rot && lockResets < MAX_LOCK_RESETS) {
      lockStartTime = millis();
      lockResets++;
    }
  }
  showField(0,fieldy,1); // Full redraw to clear ghost trails
}

void pausegame(){
  delay(200);
  M5.Lcd.drawString("(P)",61,4,1);
  while (digitalRead(37)==1) {
  Serial.println("Game paused :'v");
  delay(100);
  }
  M5.Lcd.drawString("   ",61,4,1);
  M5.Lcd.drawString("Sco.:0"+String(score),10,4,1);
}

void reset(){
  game = 0;
  collide = 0;
  M5.Lcd.drawString("Resetting...",36,135,1);
}

void showField(int startz, int finish,boolean showpiece){
  startz=max(0,startz);
  finish=min(fieldy,finish);
  
  // First, redraw the background/field to clear any ghost pieces
  for(f=startz;f<finish;f++){
    for(g=0;g<fieldx;g++){
      int colorz=field[f][g];
      if(colorz){
        // CASCADE SYSTEM: Use special drawing for cascade pieces
        if(cascadeField[f][g]) {
          blockDrawCascade(f,g,colorz);
        } else {
          blockDraw(f,g,colorz);
        }
      }
      if(!colorz){bloff(f,g,0);}
    }
  }
  
  if(showpiece){
    // Draw ghost piece first (so it appears behind the actual piece)
    drawGhostPiece();
    
    // Then draw the actual piece
    for(f=0;f<4;f++){ // grid max
      blockposY=posY+piece[block][rot][0][f];
      blockposX=posX+piece[block][rot][1][f];

      // CASCADE SYSTEM: Use special drawing for falling cascade pieces
      if(isCascadePiece) {
        blockDrawCascade(blockposY,blockposX,color);
      } else {
        blockDraw(blockposY,blockposX,color);
      }
    }
  }
  
  // Don't redraw border here - it should only be drawn once at game start
}

boolean test(int y, int x){
  boolean hitz=0;
  int blockposY[4];
  int blockposX[4];
  
  for(a=0;a<4;a++){ // grid max
    blockposY[a]=posY+y+piece[block][rot][0][a];
    if(blockposY[a]<0){hitz=1;}
    if(blockposY[a]>(fieldy-1)){hitz=1;}
    blockposX[a]=posX+x+piece[block][rot][1][a];
    if(blockposX[a]<0){hitz=1;}
    if(blockposX[a]>(fieldx-1)){hitz=1;}
  }
  
  for(a=0;a<4;a++){ // collision
    int collide=field[blockposY[a]][blockposX[a]];
    if(collide>0){hitz=1;}
  }
  return(!hitz);
}

void ButtonLoop(int _btn){
  switch (_btn) {
  case 0:
    pins[0]=check_Left();
  break; 
  case 1:
    pins[1]=check_Down();
  break;
  case 2:
    pins[2]=check_Right();
  break;
  case 3:
    pins[3]=check_Btn();
  break;
  case 4:
    pins[4]=digitalRead(35);
  break;
  case 5:
    pins[5]=check_Up();
  break;
  case 6:
    pins[6]=digitalRead(37);
  break;
  case 7:
    pins[7]=digitalRead(39);
  break;
  default:
  break;
  }
}

void scoreBoardWrite(int scorez){
  //M5.Lcd.drawLine(0,16,135,16,WHITE);
  M5.Lcd.drawString("Sco.:0"+String(score),10,4,1);
  M5.Lcd.drawString("Level:"+String(level),84,4,1);
}

void drawGameBorder() {
  // Draw smart blue border lines with small corner gaps - ONCE only
  uint16_t borderColor = 0x001F; // Pure blue
  
  // Left border line - full height
  M5.Lcd.drawFastVLine(offsetx - 1, offsety, fieldy * blockSize, borderColor);
  
  // Right border line - full height  
  M5.Lcd.drawFastVLine(offsetx + (fieldx * blockSize), offsety, fieldy * blockSize, borderColor);
  
  // Top border line - with small corner gaps for HOLD and NEXT
  // Left segment: from left border to HOLD gap
  M5.Lcd.drawFastHLine(offsetx - 1, offsety - 1, 35 - (offsetx - 1), borderColor);
  
  // Middle segment: between HOLD and NEXT gaps  
  M5.Lcd.drawFastHLine(35, offsety - 1, 100 - 35, borderColor);
  
  // Right segment: from NEXT gap to right border - FIX: stay within screen bounds
  int rightStart = 100 + 35; // After NEXT area
  int rightEnd = offsetx + (fieldx * blockSize) + 1; // Right border position
  if(rightStart < rightEnd && rightEnd <= 135) { // Make sure we don't go off screen
    M5.Lcd.drawFastHLine(rightStart, offsety - 1, rightEnd - rightStart, borderColor);
  }
  
  // Bottom border line - full width
  M5.Lcd.drawFastHLine(offsetx - 1, offsety + (fieldy * blockSize), (fieldx * blockSize) + 2, borderColor);
}

int levelSelect() {
  int selectedLevel = 0;
  bool levelSelected = false;
  unsigned long lastInput = 0;
  
  // Clear screen and show level selection
  Disp.fillScreen(BLACK);
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.setTextSize(1);
  
  // Header
  M5.Lcd.drawString("TETRIS Level Select", 12, 20, 1);
  M5.Lcd.drawLine(0, 35, 135, 35, WHITE);
  
  // Instructions
  M5.Lcd.drawString("L/R: Change Level", 15, 200, 1);
  M5.Lcd.drawString("M5: Start Game", 25, 215, 1);
  
  while(!levelSelected) {
    // Display current level
    M5.Lcd.fillRect(0, 50, 135, 140, BLACK); // Clear middle area
    
    // Large level number
    M5.Lcd.setTextSize(3);
    M5.Lcd.setTextColor(GREEN);
    String levelText = "LEVEL " + String(selectedLevel);
    M5.Lcd.drawString(levelText, 15, 80, 1);
    
    // Speed info - show the ACTUAL speed values we're using
    M5.Lcd.setTextSize(1);
    M5.Lcd.setTextColor(YELLOW);
    long actualSpeed;
    if(selectedLevel == 0) actualSpeed = 1000;
    else if(selectedLevel == 1) actualSpeed = 800;
    else if(selectedLevel == 2) actualSpeed = 600;
    else if(selectedLevel == 3) actualSpeed = 450;
    else if(selectedLevel == 4) actualSpeed = 350;
    else if(selectedLevel == 5) actualSpeed = 300;
    else if(selectedLevel == 6) actualSpeed = 250;
    else if(selectedLevel == 7) actualSpeed = 200;
    else if(selectedLevel == 8) actualSpeed = 150;
    else if(selectedLevel == 9) actualSpeed = 100;
    else if(selectedLevel == 10) actualSpeed = 85;
    else if(selectedLevel == 11) actualSpeed = 75;
    else if(selectedLevel == 12) actualSpeed = 65;
    else if(selectedLevel == 13) actualSpeed = 55;
    else if(selectedLevel == 14) actualSpeed = 50;
    else if(selectedLevel == 15) actualSpeed = 45;
    else if(selectedLevel == 16) actualSpeed = 40;
    else if(selectedLevel == 17) actualSpeed = 35;
    else if(selectedLevel == 18) actualSpeed = 32;
    else if(selectedLevel == 19) actualSpeed = 30;
    else actualSpeed = 750;
    
    M5.Lcd.drawString("Speed: " + String(actualSpeed) + "ms", 25, 120, 1);
    
    // Difficulty description - updated for new speed ranges
    M5.Lcd.setTextColor(WHITE);
    String difficulty;
    if(selectedLevel <= 1) difficulty = "Beginner";
    else if(selectedLevel <= 3) difficulty = "Intermediate"; 
    else if(selectedLevel <= 6) difficulty = "Advanced";
    else if(selectedLevel <= 9) difficulty = "Expert";
    else if(selectedLevel <= 14) difficulty = "INSANE";
    else if(selectedLevel <= 17) difficulty = "LIGHTNING";
    else difficulty = "IMPOSSIBLE";
    M5.Lcd.drawString("Difficulty: " + difficulty, 20, 140, 1);
    
    // Visual level bars - two rows for levels 0-19
    M5.Lcd.setTextColor(CYAN);
    // First row: levels 0-9
    for(int i = 0; i < 10; i++) {
      if(i <= selectedLevel) {
        M5.Lcd.fillRect(5 + (i * 12), 160, 10, 12, GREEN);
      } else {
        M5.Lcd.drawRect(5 + (i * 12), 160, 10, 12, DARKGREY);
      }
    }
    // Second row: levels 10-19  
    for(int i = 10; i < 20; i++) {
      if(i <= selectedLevel) {
        M5.Lcd.fillRect(5 + ((i-10) * 12), 175, 10, 12, YELLOW);
      } else {
        M5.Lcd.drawRect(5 + ((i-10) * 12), 175, 10, 12, DARKGREY);
      }
    }
    
    // Handle input with debouncing
    if(millis() - lastInput > 150) {
      // Left button - decrease level
      if(digitalRead(35) == 0) {
        if(selectedLevel > 0) selectedLevel--;
        lastInput = millis();
      }
      
      // Right button - increase level  
      if(digitalRead(39) == 0) {
        if(selectedLevel < 19) selectedLevel++;
        lastInput = millis();
      }
      
      // M5 button - confirm selection
      if(digitalRead(37) == 0) {
        levelSelected = true;
        delay(200); // Prevent accidental double-press
      }
    }
    
    delay(50);
  }
  
  // Clear screen before returning
  Disp.fillScreen(BLACK);
  M5.Lcd.drawLine(0,16,135,16,WHITE);
  
  return selectedLevel;
}

int calculateDropDistance() {
  // Calculate how far the current piece can drop
  int dropDistance = 0;
  int originalPosY = posY;
  
  // Keep testing downward positions until we hit something
  for(int testY = posY + 1; testY < fieldy; testY++) {
    boolean canDrop = true;
    
    // Test all 4 blocks of the piece at this position
    for(int i = 0; i < 4; i++) {
      int blockY = testY + piece[block][rot][0][i];
      int blockX = posX + piece[block][rot][1][i];
      
      // Check bounds and collisions
      if(blockY >= fieldy || blockX < 0 || blockX >= fieldx || 
         (blockY >= 0 && field[blockY][blockX] > 0)) {
        canDrop = false;
        break;
      }
    }
    
    if(canDrop) {
      dropDistance = testY - originalPosY;
    } else {
      break;
    }
  }
  
  return dropDistance;
}

void drawGhostPiece() {
  int dropDistance = calculateDropDistance();
  
  if(dropDistance > 0) {
    int ghostY = posY + dropDistance;
    
    // Draw ghost piece blocks as dim gray outlines
    for(int i = 0; i < 4; i++) {
      int blockY = ghostY + piece[block][rot][0][i];
      int blockX = posX + piece[block][rot][1][i];
      
      // Only draw if within bounds and not overlapping current piece
      if(blockY >= 0 && blockY < fieldy && blockX >= 0 && blockX < fieldx) {
        // Don't draw ghost where the actual piece is
        boolean isCurrentPiece = false;
        for(int j = 0; j < 4; j++) {
          if(blockY == posY + piece[block][rot][0][j] && 
             blockX == posX + piece[block][rot][1][j]) {
            isCurrentPiece = true;
            break;
          }
        }
        
        if(!isCurrentPiece && field[blockY][blockX] == 0) {
          // Draw ghost piece as a dim gray outline
          int posx = (blockX * blockSize) + offsetx;
          int posy = (blockY * blockSize) + offsety;
          M5.Lcd.drawRect(posx, posy, blockSize-1, blockSize-1, DARKGREY);
        }
      }
    }
  }
}

void clearGhostPiece() {
  // This function is now handled by showField redrawing the background
  // No separate clearing needed since we redraw empty spaces each time
}

// ========== IR REMOTE CONTROL FUNCTIONS ==========

void irRemoteControl() {
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.setTextSize(1);
  M5.Lcd.drawString("🎃 Halloween Remote 📺", 5, 40);
  M5.Lcd.drawString("Sending Universal", 20, 80);
  M5.Lcd.drawString("TV POWER codes...", 20, 100);
  
  // Send all universal power codes
  sendUniversalPower();
  
  M5.Lcd.drawString("Power codes sent!", 20, 140);
  M5.Lcd.drawString("Returning to game...", 15, 160);
  
  delay(2000); // Show message for 2 seconds
}

void sendUniversalPower() {
  // Comprehensive TV power codes from multiple protocols and brands
  // Progress indicator
  int totalCodes = 50;
  int currentCode = 0;
  
  M5.Lcd.fillRect(20, 120, 100, 10, BLACK);
  M5.Lcd.drawString("Sending code: 1/50", 20, 120);
  
  // Sony TV Power codes (multiple variants)
  irsend.sendSony(0xA90, 12); currentCode++; delay(100);
  irsend.sendSony(0x290, 12); currentCode++; delay(100);
  irsend.sendSony(0x750, 12); currentCode++; delay(100);
  
  // Samsung TV Power codes
  irsend.sendSAMSUNG(0xE0E040BF); currentCode++; delay(100);
  irsend.sendSAMSUNG(0xE0E09966); currentCode++; delay(100);
  
  // LG TV Power codes  
  irsend.sendLG(0x20DF10EF); currentCode++; delay(100);
  irsend.sendLG(0x20DF0FF0); currentCode++; delay(100);
  
  // Update progress
  M5.Lcd.fillRect(20, 120, 100, 10, BLACK);
  M5.Lcd.drawString("Sending code: 7/50", 20, 120);
  
  // Panasonic TV Power codes
  irsend.sendPanasonic(0x4004, 0x100BCBD); currentCode++; delay(100);
  irsend.sendPanasonic(0x4004, 0x1000BCD); currentCode++; delay(100);
  
  // NEC Protocol variants (many TVs)
  irsend.sendNEC(0xFF02FD); currentCode++; delay(100);  // Generic
  irsend.sendNEC(0x20DF10EF); currentCode++; delay(100); // LG variant
  irsend.sendNEC(0x04FB08F7); currentCode++; delay(100); // Sharp
  irsend.sendNEC(0x1CE318E7); currentCode++; delay(100); // Toshiba
  irsend.sendNEC(0x40BF00FF); currentCode++; delay(100); // Philips
  
  // Update progress  
  M5.Lcd.fillRect(20, 120, 100, 10, BLACK);
  M5.Lcd.drawString("Sending code: 13/50", 20, 120);
  
  // RC5 Protocol (European TVs)
  irsend.sendRC5(0x0C0C); currentCode++; delay(100); // Philips
  irsend.sendRC5(0x100C); currentCode++; delay(100); // Thomson
  irsend.sendRC5(0x180C); currentCode++; delay(100); // Grundig
  
  // RC6 Protocol 
  irsend.sendRC6(0x800F040C, 20); currentCode++; delay(100);
  
  // JVC Power codes
  irsend.sendJVC(0xC5E8, 16); currentCode++; delay(100);
  irsend.sendJVC(0xC538, 16); currentCode++; delay(100);
  
  // Update progress
  M5.Lcd.fillRect(20, 120, 100, 10, BLACK); 
  M5.Lcd.drawString("Sending code: 19/50", 20, 120);
  
  // DISH Network codes
  irsend.sendDISH(0x775A85, 16); currentCode++; delay(100);
  irsend.sendDISH(0x775A05, 16); currentCode++; delay(100);
  
  // Sharp Power codes
  irsend.sendSharpRaw(0x41A2, 15); currentCode++; delay(100);
  irsend.sendSharpRaw(0x45A8, 15); currentCode++; delay(100);
  
  // Mitsubishi codes
  irsend.sendMitsubishi(0xE25D); currentCode++; delay(100);
  
  // Update progress
  M5.Lcd.fillRect(20, 120, 100, 10, BLACK);
  M5.Lcd.drawString("Sending code: 25/50", 20, 120);
  
  // Additional Samsung variants (using standard SAMSUNG function)
  irsend.sendSAMSUNG(0x400462001); currentCode++; delay(100);
  irsend.sendSAMSUNG(0x400460001); currentCode++; delay(100);
  
  // More NEC variants for different brands
  irsend.sendNEC(0x00FF807F); currentCode++; delay(100); // Emerson
  irsend.sendNEC(0x807F40BF); currentCode++; delay(100); // RCA
  irsend.sendNEC(0x827D42BD); currentCode++; delay(100); // Zenith
  irsend.sendNEC(0x20DF708F); currentCode++; delay(100); // LG variant 2
  irsend.sendNEC(0x55AA1AE5); currentCode++; delay(100); // Vizio
  irsend.sendNEC(0x10EF20DF); currentCode++; delay(100); // Sanyo
  
  // Update progress
  M5.Lcd.fillRect(20, 120, 100, 10, BLACK);
  M5.Lcd.drawString("Sending code: 32/50", 20, 120);
  
  // Pioneer codes
  irsend.sendPioneer(0xA55A38C7, 32); currentCode++; delay(100);
  
  // Whynter codes
  irsend.sendWhynter(0x87654321, 32); currentCode++; delay(100);
  
  // More RC5 variants
  irsend.sendRC5(0x140C); currentCode++; delay(100); // Bang & Olufsen
  irsend.sendRC5(0x1C0C); currentCode++; delay(100); // Loewe
  
  // Additional Panasonic variants
  irsend.sendPanasonic(0x4004, 0x1004BCD); currentCode++; delay(100);
  irsend.sendPanasonic(0x4004, 0x1008BCD); currentCode++; delay(100);
  
  // Update progress
  M5.Lcd.fillRect(20, 120, 100, 10, BLACK);
  M5.Lcd.drawString("Sending code: 38/50", 20, 120);
  
  // Coolix (many Chinese brands)
  irsend.sendCOOLIX(0xB24D7F); currentCode++; delay(100);
  
  // Denon codes  
  irsend.sendDenon(0x2A4C028D, 32); currentCode++; delay(100);
  
  // More NEC codes for different manufacturers
  irsend.sendNEC(0x08F708F7); currentCode++; delay(100); // Hitachi
  irsend.sendNEC(0x42BD827D); currentCode++; delay(100); // Admiral
  
  // Additional Sony variants
  irsend.sendSony(0xB90, 12); currentCode++; delay(100);
  irsend.sendSony(0xF90, 12); currentCode++; delay(100);
  irsend.sendSony(0x190, 12); currentCode++; delay(100);
  
  // Update progress
  M5.Lcd.fillRect(20, 120, 100, 10, BLACK);
  M5.Lcd.drawString("Sending code: 45/50", 20, 120);
  
  // Final batch - more NEC variants
  irsend.sendNEC(0x02FD807F); currentCode++; delay(100);
  irsend.sendNEC(0x827D708F); currentCode++; delay(100);
  irsend.sendNEC(0x10EF708F); currentCode++; delay(100);
  irsend.sendNEC(0xFF00BF40); currentCode++; delay(100);
  irsend.sendNEC(0x7F80FF00); currentCode++; delay(100);
  
  // Final progress update
  M5.Lcd.fillRect(20, 120, 100, 10, BLACK);
  M5.Lcd.drawString("All 50 codes sent!", 20, 120);
}

// ==============================ENHANCED FEATURES==============================

// HOLD piece functionality
void holdPiece() {
  // Can only use HOLD once per piece and only if not already used
  if (!canUseHold || holdUsed) return;
  
  if (heldPiece == -1) {
    // First time using HOLD - store current piece and use next piece
    heldPiece = block;
    
    // Use next piece as current piece
    block = nextPiece;
    nextPiece = random(7); // Generate new next piece
    color = random(96) + 1;
    posY = 1;
    posX = 4;
    rot = 0;
  } else {
    // Swap current piece with held piece
    int tempPiece = block;
    block = heldPiece;
    heldPiece = tempPiece;
    
    // Reset position for swapped piece
    posY = 1;
    posX = 4;
    rot = 0;
    // Keep current color scheme
  }
  
  // Mark HOLD as used for this piece cycle
  holdUsed = true;
  canUseHold = false;
  
  // Reset lock delay system
  lockStartTime = 0;
  pieceOnGround = false;
  lockResets = 0;
  
  // Redraw field with new piece
  showField(0, fieldy, 1);
  
  // Draw the held piece and next piece displays
  drawHoldPiece();
  drawNextPiece();
  lastDrawnNextPiece = nextPiece; // Update tracking
}

void drawHoldPiece() {
  // Clear hold area (top left corner) - smaller area
  M5.Lcd.fillRect(0, 18, 35, 45, BLACK);
  
  if (heldPiece == -1) {
    // No piece held yet - just show empty area
    return;
  }
  
  // Draw held piece preview (small version)
  int previewX = 8;  // X offset for piece preview
  int previewY = 25; // Y offset for piece preview
  int previewSize = 4; // Smaller blocks for preview
  
  // Get piece shape (using rotation 0)
  for(int i = 0; i < 4; i++) {
    int blockX = previewX + (piece[heldPiece][0][1][i] * previewSize);
    int blockY = previewY + (piece[heldPiece][0][0][i] * previewSize);
    
    // Draw small colored block for held piece
    int heldColor = (heldPiece % 7); // Use piece-specific color
    uint16_t color16;
    switch(heldColor) {
      case 0: color16 = 0x07FF; break; // Cyan
      case 1: color16 = 0x001F; break; // Blue  
      case 2: color16 = 0xFC00; break; // Orange
      case 3: color16 = 0xFFE0; break; // Yellow
      case 4: color16 = 0x07E0; break; // Green
      case 5: color16 = 0x8010; break; // Purple
      case 6: color16 = 0xF800; break; // Red
      default: color16 = 0x07FF; break;
    }
    
    M5.Lcd.fillRect(blockX, blockY, previewSize-1, previewSize-1, color16);
    M5.Lcd.drawRect(blockX, blockY, previewSize-1, previewSize-1, WHITE);
  }
}

void drawNextPiece() {
  // Clear next area (top right corner) - smaller area
  M5.Lcd.fillRect(100, 18, 35, 45, BLACK);
  
  if (nextPiece == -1) {
    // No next piece generated yet - just show empty area
    return;
  }
  
  // Draw next piece preview (small version)
  int previewX = 108;  // X offset for piece preview
  int previewY = 25;  // Y offset for piece preview
  int previewSize = 4; // Smaller blocks for preview
  
  // Get piece shape (using rotation 0)
  for(int i = 0; i < 4; i++) {
    int blockX = previewX + (piece[nextPiece][0][1][i] * previewSize);
    int blockY = previewY + (piece[nextPiece][0][0][i] * previewSize);
    
    // Draw small colored block for next piece
    int nextColor = (nextPiece % 7); // Use piece-specific color
    uint16_t color16;
    switch(nextColor) {
      case 0: color16 = 0x07FF; break; // Cyan (I-piece)
      case 1: color16 = 0x001F; break; // Blue (J-piece)
      case 2: color16 = 0xFC00; break; // Orange (L-piece)
      case 3: color16 = 0xFFE0; break; // Yellow (O-piece)
      case 4: color16 = 0x07E0; break; // Green (S-piece)
      case 5: color16 = 0x8010; break; // Purple (T-piece)
      case 6: color16 = 0xF800; break; // Red (Z-piece)
      default: color16 = 0x07FF; break;
    }
    
    M5.Lcd.fillRect(blockX, blockY, previewSize-1, previewSize-1, color16);
    M5.Lcd.drawRect(blockX, blockY, previewSize-1, previewSize-1, WHITE);
  }
}

void softDrop() {
  // Soft drop gives small score bonus
  score += 1;
  moveDown();
}

// ==============================NETWORK BATTLE==============================

bool setupTetrisBattle() {
  M5.Display.fillScreen(TFT_BLACK);
  M5.Display.setTextColor(TFT_WHITE);
  M5.Display.setCursor(10, 10);
  
  // Clean WiFi state first
  WiFi.disconnect(true);
  WiFi.mode(WIFI_STA);
  delay(500);
  
  // Try to connect as client first
  M5.Display.println("Searching for host...");
  WiFi.begin(tetris_ssid, tetris_password);
  
  unsigned long startTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startTime < 8000) {
    delay(500);
    M5.Display.print(".");
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    // Connected as client
    isHost = false;
    M5.Display.println("\nConnected as CLIENT!");
    M5.Display.print("Host IP: ");
    M5.Display.println(WiFi.gatewayIP());
    remoteIP = WiFi.gatewayIP();
    
    battleUDP.begin(tetris_port);
    isBattleMode = true;
    isConnected = true;
    
    // Send immediate heartbeat to register with host
    delay(1000);
    sendBattleMessage(0); // Send heartbeat to establish connection
    
    delay(2000);
    return true;
  } else {
    // Failed to connect, try to become host
    M5.Display.println("\nNo host found, becoming HOST...");
    
    WiFi.mode(WIFI_AP);
    bool apStarted = WiFi.softAP(tetris_ssid, tetris_password);
    
    if (apStarted) {
      isHost = true;
      M5.Display.println("HOST mode started!");
      M5.Display.print("SSID: ");
      M5.Display.println(tetris_ssid);
      M5.Display.print("IP: ");
      M5.Display.println(WiFi.softAPIP());
      M5.Display.println("Waiting for client...");
      
      battleUDP.begin(tetris_port);
      isBattleMode = true;
      
      // Wait for client to connect
      unsigned long waitStart = millis();
      unsigned long lastHostHeartbeat = 0;
      
      while (!isConnected && millis() - waitStart < 30000) {
        receiveBattleMessages(); // This will set isConnected when client sends heartbeat
        
        // Send periodic heartbeats as host to help client find us
        if(millis() - lastHostHeartbeat > 1000) {
          sendBattleMessage(0); // Send heartbeat
          lastHostHeartbeat = millis();
        }
        
        delay(100);
      }
      
      if (isConnected) {
        M5.Display.println("Client connected!");
        delay(2000);
        return true;
      } else {
        M5.Display.println("No client connected, timeout");
        WiFi.mode(WIFI_OFF);
        isBattleMode = false;
        delay(3000);
        return false;
      }
    } else {
      M5.Display.println("Failed to start AP");
      delay(3000);
      return false;
    }
  }
}

void sendBattleMessage(uint8_t type, float data1, float data2) {
  if (!isBattleMode || !isConnected) return;
  
  BattleMessage msg;
  msg.type = type;
  msg.data1 = data1;
  msg.data2 = data2;
  msg.timestamp = millis();
  
  battleUDP.beginPacket(remoteIP, tetris_port);
  battleUDP.write((uint8_t*)&msg, sizeof(msg));
  battleUDP.endPacket();
}

void receiveBattleMessages() {
  int packetSize = battleUDP.parsePacket();
  if (packetSize == sizeof(BattleMessage)) {
    BattleMessage msg;
    battleUDP.read((uint8_t*)&msg, sizeof(msg));
    
    if (!isConnected) {
      // First message received - establish connection
      isConnected = true;
      remoteIP = battleUDP.remoteIP();
    }
    
    switch (msg.type) {
      case 0: // Heartbeat
        // Update connection status
        lastHeartbeat = millis();
        break;
        
      case 1: // Line clear attack - receive garbage rows
        addGarbageRows((int)msg.data1);
        
        // Visual feedback for receiving attack  
        M5.Display.fillRect(5, 200, 30, 10, TFT_YELLOW);
        M5.Display.setTextColor(TFT_BLACK);
        M5.Display.drawString("HIT!", 8, 200);
        delay(300); // Show hit indicator
        M5.Display.fillRect(5, 200, 30, 10, TFT_BLACK); // Clear
        break;
        
      case 2: // Opponent danger level
        opponentDangerLevel = (int)msg.data1;
        break;
        
      case 3: // Game over
        M5.Display.fillRect(30, 100, 80, 40, TFT_BLACK);
        M5.Display.setCursor(40, 110);
        M5.Display.setTextColor(TFT_GREEN);
        M5.Display.println("YOU WIN!");
        delay(3000);
        game = 0; // End game
        break;
        
      case 4: // Ready up message
        remotePlayerReady = true;
        remoteSelectedLevel = (int)msg.data1; // Get opponent's selected level
        bothPlayersReady = (localPlayerReady && remotePlayerReady);
        break;
        
      case 5: // Start game message
        bothPlayersReady = true;
        break;
    }
  }
}

void addGarbageRows(int rows) {
  // Add garbage rows to bottom and shift everything up
  for(int r = 0; r < rows; r++) {
    // Shift all rows up
    for(int y = 0; y < fieldy - 1; y++) {
      for(int x = 0; x < fieldx; x++) {
        field[y][x] = field[y + 1][x];
      }
    }
    
    // Add new garbage row at bottom
    int gapPosition = random(fieldx); // Random gap in garbage row
    for(int x = 0; x < fieldx; x++) {
      if(x == gapPosition) {
        field[fieldy - 1][x] = 0; // Gap
      } else {
        field[fieldy - 1][x] = 50; // Garbage block
      }
    }
  }
  
  showField(0, fieldy, 1); // Redraw field
}

void drawOpponentDanger(int dangerLevel) {
  // Draw danger meter for opponent on right side
  int meterHeight = (dangerLevel * 80) / 100;
  
  // Clear previous meter
  M5.Lcd.fillRect(130, 80, 5, 80, BLACK);
  
  // Draw current danger level
  if(meterHeight > 0) {
    uint16_t dangerColor = TFT_GREEN;
    if(dangerLevel > 60) dangerColor = TFT_RED;
    else if(dangerLevel > 30) dangerColor = TFT_YELLOW;
    
    M5.Lcd.fillRect(130, 160 - meterHeight, 5, meterHeight, dangerColor);
  }
}

void checkGameOver() {
  // Check if top rows have blocks (game over condition)
  bool hasBlocksInTopRows = false;
  for(int y = 0; y < 3; y++) {
    for(int x = 0; x < fieldx; x++) {
      if(field[y][x] > 0) {
        hasBlocksInTopRows = true;
        break;
      }
    }
    if(hasBlocksInTopRows) break;
  }
  
  if(hasBlocksInTopRows) {
    // Send game over message to opponent
    sendBattleMessage(3); // Game over - opponent wins
    
    M5.Display.fillRect(30, 100, 80, 40, TFT_BLACK);
    M5.Display.setCursor(40, 110);
    M5.Display.setTextColor(TFT_RED);
    M5.Display.println("YOU LOSE!");
    delay(3000);
    game = 0; // End game
  } else {
    // Calculate and send current danger level
    int dangerLevel = 0;
    for(int y = fieldy - 10; y < fieldy; y++) { // Check bottom 10 rows
      for(int x = 0; x < fieldx; x++) {
        if(field[y][x] > 0) {
          dangerLevel += (fieldy - y) * 2; // Higher penalty for blocks higher up
        }
      }
    }
    
    dangerLevel = min(dangerLevel, 100); // Cap at 100%
    sendBattleMessage(2, dangerLevel); // Send danger level
  }
}

bool waitForBothPlayersReady() {
  M5.Display.fillScreen(TFT_BLACK);
  M5.Display.setTextColor(TFT_WHITE);
  M5.Display.setCursor(10, 10);
  M5.Display.println("BATTLE MODE CONNECTED!");
  M5.Display.setCursor(10, 30);
  if(isHost) {
    M5.Display.println("You are: HOST");
  } else {
    M5.Display.println("You are: CLIENT");
  }
  
  M5.Display.setCursor(10, 50);
  M5.Display.print("Your Level: ");
  M5.Display.println(selectedLevel);
  M5.Display.setCursor(10, 70);
  if(isHost) {
    M5.Display.println("HOST: Press M5 when ready");
  } else {
    M5.Display.println("CLIENT: Press M5 when ready");
  }
  
  // Ready-up loop - BOTH players can ready up with their chosen levels
  localPlayerReady = false;
  remotePlayerReady = false;
  bothPlayersReady = false;
  
  bool lastButtonState = true; // Track button state for proper detection
  
  while(!bothPlayersReady) {
    receiveBattleMessages();
    
    // Both players can ready up
    if(!localPlayerReady) {
      // Ready up with M5 button - use button state change detection
      bool currentButtonState = (digitalRead(37) == 0);
      if(!currentButtonState && lastButtonState) { // Button released (was pressed)
        localPlayerReady = true;
        sendBattleMessage(4, selectedLevel); // Send ready message with YOUR chosen level
        delay(100); // Small delay to prevent double-trigger
      }
      lastButtonState = currentButtonState;
      
      // Update ready display
      M5.Display.fillRect(10, 90, 120, 60, TFT_BLACK);
      M5.Display.setCursor(10, 90);
      if(localPlayerReady) {
        M5.Display.setTextColor(TFT_GREEN);
        M5.Display.print("You: READY! Lv");
        M5.Display.println(selectedLevel);
        M5.Display.setTextColor(TFT_WHITE);
      } else {
        M5.Display.println("Press M5 to ready up");
      }
      
      // Show opponent status
      M5.Display.setCursor(10, 110);
      if(remotePlayerReady) {
        M5.Display.setTextColor(TFT_GREEN);
        M5.Display.print("Opp: READY! Lv");
        M5.Display.println(remoteSelectedLevel);
        M5.Display.setTextColor(TFT_WHITE);
      } else {
        M5.Display.setTextColor(TFT_YELLOW);
        M5.Display.println("Opp: Waiting...");
        M5.Display.setTextColor(TFT_WHITE);
      }
      
      // Show battle info
      if(localPlayerReady && remotePlayerReady) {
        M5.Display.setCursor(10, 130);
        M5.Display.println("MIXED SKILL BATTLE!");
      }
    }
    
    // Check if both players are ready
    if(localPlayerReady && remotePlayerReady) {
      // Send start game message (host initiates)
      if(isHost) {
        sendBattleMessage(5); // Start game
      }
      bothPlayersReady = true;
      
      M5.Display.fillRect(10, 150, 120, 20, TFT_BLACK);
      M5.Display.setCursor(10, 150);
      M5.Display.setTextColor(TFT_CYAN);
      M5.Display.println("STARTING BATTLE!");
      M5.Display.setTextColor(TFT_WHITE);
      delay(500);
      delay(2000);
    }
    
    delay(50); // Small delay to prevent overwhelming
  }
  
  return true;
}

void toggleCascadeMode() {
  cascadeEnabled = !cascadeEnabled;
  
  // Visual feedback
  M5.Lcd.fillRect(10, 200, 115, 20, TFT_BLACK); // Clear area
  M5.Lcd.setTextColor(cascadeEnabled ? TFT_CYAN : TFT_RED);
  M5.Lcd.setCursor(10, 200);
  if(cascadeEnabled) {
    M5.Lcd.println("CASCADE: ON");
  } else {
    M5.Lcd.println("CASCADE: OFF");
  }
  M5.Lcd.setTextColor(TFT_WHITE); // Reset color
  delay(1000); // Show message for 1 second
  M5.Lcd.fillRect(10, 200, 115, 20, TFT_BLACK); // Clear message
}

// ========== CASCADE SYSTEM V1.3 ==========

void processCascadePhysics() {
  // Process cascade physics for all cascade blocks on the field
  // Start from bottom and work up to avoid processing same blocks multiple times
  
  boolean hadMovement = true;
  int iterations = 0;
  
  while(hadMovement && iterations < 20) { // Prevent infinite loops
    hadMovement = false;
    iterations++;
    
    // Scan from bottom to top
    for(int y = fieldy-2; y >= 0; y--) {
      for(int x = 0; x < fieldx; x++) {
        // If this is a cascade block and it can fall
        if(cascadeField[y][x] && field[y][x] != 0 && canCascadeBlockFall(y, x)) {
          cascadeBlockDrop(y, x);
          hadMovement = true;
          delay(50); // Visual cascade effect
        }
      }
    }
    
    if(hadMovement) {
      showField(0, fieldy, 0); // Refresh display during cascade
    }
  }
}

void cascadeBlockDrop(int startY, int startX) {
  // Drop a single cascade block as far as it can go
  
  int currentY = startY;
  int color = field[startY][startX];
  boolean isCascade = cascadeField[startY][startX];
  
  // Clear the original position
  field[startY][startX] = 0;
  cascadeField[startY][startX] = false;
  
  // Find how far it can drop
  while(currentY + 1 < fieldy && field[currentY + 1][startX] == 0) {
    currentY++;
  }
  
  // Place block at final position
  field[currentY][startX] = color;
  cascadeField[currentY][startX] = isCascade;
}

boolean canCascadeBlockFall(int y, int x) {
  // Check if a cascade block can fall (has empty space below)
  
  // Can't fall if at bottom
  if(y >= fieldy - 1) return false;
  
  // Can't fall if there's something below
  if(field[y + 1][x] != 0) return false;
  
  return true;
}