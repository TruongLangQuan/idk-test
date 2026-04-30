#include <M5StickCPlus2.h>
#include "UNIT_MiniJoyC.h"
#include "tet.h"
#include "wifi_beacon.h"
#include "wifi_scanner.h"
#include <IRsend.h>  // IR library for remote control

#define Disp M5.Lcd
#define BITMAP M5.Lcd.drawBitmap(0,0,135,240,tet)
#define POS_X 0
#define POS_Y 1

// IR Remote setup - using built-in IR emitter on GPIO19
IRsend irsend(19);

// Function declarations
void blockDraw(int y, int x, int colorz);
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
void holdPiece();           // NEW: Hold piece function
void drawHoldPiece();       // NEW: Draw hold piece preview
void drawNextPiece();       // NEW: Draw next piece preview
void drawMiniPiece(int pieceType, int x, int y, int scale); // NEW: Draw mini piece helper

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

int piece[7][4][2][4]={
  // O piece - square (doesn't rotate) - KEEP SAME
  {{{0,1,0,1},{0,0,1,1}},{{0,1,0,1},{0,0,1,1}},{{0,1,0,1},{0,0,1,1}},{{0,1,0,1},{0,0,1,1}}},
  
  // I piece - line - KEEP SAME
  {{{0,0,0,0},{-1,0,1,2}},{{-1,0,1,2},{0,0,0,0}},{{0,0,0,0},{-1,0,1,2}},{{-1,0,1,2},{0,0,0,0}}},
  
  // T piece - REVERSE ROTATION ORDER (was wrong)
  {{{0,0,0,1},{-1,0,1,0}},{{1,0,-1,0},{0,0,0,-1}},{{0,0,0,-1},{-1,0,1,0}},{{1,0,-1,0},{0,0,0,1}}},
  
  // S piece - KEEP SAME  
  {{{0,-1,0,1},{0,0,1,1}},{{0,1,1,0},{0,0,-1,1}},{{0,-1,0,1},{0,0,1,1}},{{0,1,1,0},{0,0,-1,1}}},
  
  // Z piece - KEEP SAME
  {{{0,-1,0,1},{0,0,-1,-1}},{{0,0,1,1},{0,-1,0,1}},{{0,-1,0,1},{0,0,-1,-1}},{{0,0,1,1},{0,-1,0,1}}},
  
  // J piece - KEEP SAME (works correctly)
  {{{1,0,-1,1},{0,0,0,-1}},{{0,-1,0,0},{0,0,1,2}},{{0,1,2,0},{0,0,0,1}},{{1,0,0,0},{1,1,0,-1}}},
  
  // L piece - REVERSE ROTATION ORDER (was wrong)
  {{{0,0,1,2},{-1,0,0,0}},{{-1,0,0,0},{1,1,0,-1}},{{1,1,0,-1},{1,0,0,0}},{{1,0,0,0},{-1,-1,0,1}}}
};
// piece[block][rot][y/x][4 spots]

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
long unsigned lockDelayStart; // Lock delay timer
boolean lockDelayActive = false; // Is lock delay currently active
const long lockDelayTime = 750; // 750ms lock delay for better control

// MODERN TETRIS FEATURES - Hold & Next Piece
int heldPiece = -1;        // -1 = no held piece, 0-6 = piece type
int nextPiece;             // Pre-generated next piece (0-6)
boolean canHold = true;    // One hold per piece drop (resets when piece locks)

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
  M5.Lcd.drawString("v.1.5.0",4,8,1);
  M5.Lcd.drawString((String)M5.Power.getBatteryLevel()+"%",107,8,1);
  M5.Lcd.drawString("Loading...",39,134,1);
  while (!(Joystick.begin(&Wire, JoyC_ADDR, 0, 26, 100000UL))) {
    delay(100);
    Serial.println("I2C Error!\r\n");
  }
  M5.Lcd.fillRect(39,134,100,10,BLACK);
  M5.Lcd.drawString(" Press M5 ",39,134,1);
  M5.Lcd.drawString("L+R:Beacon Up:Scanner",5,150,1);
  M5.Lcd.drawString("Game has Level Select!",8,165,1);
  
  // Add "TETRIS_PRO!" in bigger, bolder font above "Press M5"
  M5.Lcd.setTextColor(0xF800); // Red color  
  M5.Lcd.setTextSize(2);        // Bigger font size
  M5.Lcd.drawString("TETRIS_PRO!", 5, 118, 1);  // All on one line with regular Rs
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
        
        // Redraw "TETRIS_PRO!" after WiFi scanner
        M5.Lcd.setTextColor(0xF800);
        M5.Lcd.setTextSize(2);
        M5.Lcd.drawString("TETRIS_PRO!", 5, 118, 1);
        M5.Lcd.setTextSize(1);
      }
    }
    
    // Check for IR Remote mode (Down on joystick) 
    if(Joystick.getADCValue(1) < 1600) {
      delay(500); // Hold time to confirm
      if(Joystick.getADCValue(1) < 1600) {
        irRemoteControl();
        // After IR remote, redraw the screen
        Disp.fillScreen(BLACK);
        M5.Lcd.drawLine(0,16,135,16,WHITE);
        M5.Lcd.fillRect(39,134,100,10,BLACK);
        M5.Lcd.drawString(" Press M5 ",39,134,1);
        M5.Lcd.drawString("L+R:Beacon Down:IR Remote",2,150,1);
        M5.Lcd.drawString("Game has Level Select!",8,165,1);
        
        // Redraw "TETRIS_PRO!" after IR remote
        M5.Lcd.setTextColor(0xF800);
        M5.Lcd.setTextSize(2);
        M5.Lcd.drawString("TETRIS_PRO!", 5, 118, 1);
        M5.Lcd.setTextSize(1);
      }
    }
    delay(30);
  }
  delay(300);

  Disp.fillScreen(BLACK);
  M5.Lcd.drawLine(0,16,135,16,WHITE);
  
  // Draw Halloween colored borders on the sides of the game area
  // Left border (x=0 to x=13) with Halloween color pattern
  for(int x = 0; x <= 13; x++) {
    for(int y = 20; y < 240; y += 4) { // Every 4 pixels for pattern
      int colorPattern = ((x + y/4) % 7); // Create color pattern
      uint16_t borderColor;
      switch(colorPattern) {
        case 0: borderColor = 0xF800; break; // Red/Orange
        case 1: borderColor = 0x7800; break; // Dark Orange  
        case 2: borderColor = 0xF81F; break; // Magenta/Purple
        case 3: borderColor = 0x07E0; break; // Green
        case 4: borderColor = 0xFFE0; break; // Yellow
        case 5: borderColor = 0x8010; break; // Dark Purple
        case 6: borderColor = 0xFC00; break; // Orange-Red
        default: borderColor = 0xF800; break;
      }
      M5.Lcd.drawPixel(x, y, borderColor);
      M5.Lcd.drawPixel(x, y+1, borderColor);
      M5.Lcd.drawPixel(x, y+2, borderColor);
    }
  }
  
  // Right border (x=124 to x=135) with Halloween color pattern
  for(int x = 124; x < 135; x++) {
    for(int y = 20; y < 240; y += 4) { // Every 4 pixels for pattern
      int colorPattern = ((x + y/4) % 7); // Create color pattern
      uint16_t borderColor;
      switch(colorPattern) {
        case 0: borderColor = 0xF800; break; // Red/Orange
        case 1: borderColor = 0x7800; break; // Dark Orange  
        case 2: borderColor = 0xF81F; break; // Magenta/Purple
        case 3: borderColor = 0x07E0; break; // Green
        case 4: borderColor = 0xFFE0; break; // Yellow
        case 5: borderColor = 0x8010; break; // Dark Purple
        case 6: borderColor = 0xFC00; break; // Orange-Red
        default: borderColor = 0xF800; break;
      }
      M5.Lcd.drawPixel(x, y, borderColor);
      M5.Lcd.drawPixel(x, y+1, borderColor);
      M5.Lcd.drawPixel(x, y+2, borderColor);
    }
  }
  
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
  
  // Add "FAST TETRIS!" text in red at center of screen
  M5.Lcd.setTextColor(0xF800); // Red color
  M5.Lcd.setTextSize(1);
  M5.Lcd.drawString("FAST", 55, 100, 1);      // Center horizontal, middle vertical
  M5.Lcd.drawString("TETRIS!", 40, 115, 1);  // Center horizontal, below FAST
}

void loop(){
  // Show level select screen
  int startingLevel = levelSelect();
  
  // Calculate FAST TETRIS speed - much faster drop speeds + INSANE levels 10-20!
  // Level 0-9: Fast to Lightning, Level 10-20: UNREAL speeds!
  if(startingLevel == 0) speed = 150;
  else if(startingLevel == 1) speed = 130;
  else if(startingLevel == 2) speed = 110;
  else if(startingLevel == 3) speed = 90;
  else if(startingLevel == 4) speed = 70;
  else if(startingLevel == 5) speed = 60;
  else if(startingLevel == 6) speed = 50;
  else if(startingLevel == 7) speed = 40;
  else if(startingLevel == 8) speed = 30;
  else if(startingLevel == 9) speed = 20;
  // INSANE LEVELS 10-20: UNREAL SPEEDS!
  else if(startingLevel == 10) speed = 15;
  else if(startingLevel == 11) speed = 12;
  else if(startingLevel == 12) speed = 10;
  else if(startingLevel == 13) speed = 8;
  else if(startingLevel == 14) speed = 6;
  else if(startingLevel == 15) speed = 5;
  else if(startingLevel == 16) speed = 4;
  else if(startingLevel == 17) speed = 3;
  else if(startingLevel == 18) speed = 2;
  else if(startingLevel == 19) speed = 1;  // FRAMES! PURE INSANITY!
  else if(startingLevel == 20) speed = 1;  // MAX INSANITY
  else speed = 150; // fallback
  
  timez=millis();
  score=0;
  level=startingLevel; // Start at selected level
  stage=startingLevel / 10; // Set proper stage based on level
  lineCount=0; // Reset line counter for proper speed progression
  
  // Initialize modern Tetris features
  heldPiece = -1;           // No held piece at start
  nextPiece = random(7);    // Generate first next piece
  canHold = true;           // Allow holding
  
  newPiece(0);
  game=1;
  showField(0,fieldy,1);
  
  // Draw initial hold and next piece areas
  drawHoldPiece();
  drawNextPiece();
  
  while(game){
    collide=1;
    multiplier=1;
    // Removed scoreBoardWrite - only show score at game end
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
    }
  }
  delay(1500);
  
  // Show final score at game end
  scoreBoardWrite(score);
  delay(3000);  // Show score for 3 seconds
  
  M5.Lcd.drawString("            ",36,135,1); //Erase Reset msg
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
  // Dynamic colors that change each level for variety!
  int red=0;
  int gre=0;
  int blu=0;
  
  // Use level to shift color palette - each level gets different colors!
  int levelOffset = (level % 10); // Cycle through 10 different palettes
  int techColor = ((colorz + levelOffset) % 8); // 8 tech colors + level offset
  
  switch(techColor) {
    case 0: // Electric Blue (varies with level)
      red = 0 + (levelOffset * 2); gre = 15 + levelOffset; blu = 31;
      break;
    case 1: // Matrix Green (varies with level)  
      red = 0; gre = 31; blu = 8 + (levelOffset * 2);
      break;
    case 2: // Cyber Red (varies with level)
      red = 31; gre = 0 + levelOffset; blu = 8 + levelOffset;
      break;
    case 3: // Neon Cyan (varies with level)
      red = 0 + levelOffset; gre = 31; blu = 31;
      break;
    case 4: // Electric Purple (varies with level)
      red = 20 + levelOffset; gre = 0; blu = 31;
      break;
    case 5: // Laser Yellow (varies with level)
      red = 31; gre = 31; blu = 0 + (levelOffset * 3);
      break;
    case 6: // Tech Magenta (varies with level)
      red = 31; gre = 0 + (levelOffset * 2); blu = 31;
      break;
    case 7: // Circuit White (varies with level)
      red = 25 + levelOffset; gre = 25 + levelOffset; blu = 31;
      break;
    default:
      red = 0 + levelOffset; gre = 31; blu = 15 + levelOffset; // Fallback
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
    }
    for(b=linePos[a];b>0;b--){
      for(c=0;c<fieldx;c++){
        field[b][c]=field[b-1][c];
      }
    }
  }
  lineCount+=linez;
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
        field[posY+piece[block][rot][0][a]][posX+piece[block][rot][1][a]]=color;
      }
    clearLines();
  }
  
  // Modern Tetris piece generation
  if(setPiece) {
    canHold = true;  // Reset hold ability when piece locks
  }
  
  posY=1;
  posX=4;
  rot=0;
  
  // Use next piece, then generate new next piece
  if(nextPiece >= 0) {
    block = nextPiece;
    nextPiece = random(7);  // Generate new next piece
  } else {
    block = random(7);      // Fallback for first piece
    nextPiece = random(7);
  }
  
  color=random(96)+1;
  
  // Reset lock delay for new piece
  lockDelayActive = false;
  
  showField(0,20,1);
  drawNextPiece();    // Update next piece display
  drawHoldPiece();    // Update hold piece display
  
  if(game!=0){
    game=test(0,0);    
  }
}

void wait(){
  timez+=speed;
  while(millis()<(timez+speed)){
    if(millis()>(leftHold+75)){hold[0]=0;}  // Faster JoyC response!
    if(millis()>(downHold+75)){hold[1]=0;}  // 75ms instead of 150ms
    if(millis()>(rightHold+75)){hold[2]=0;}  // Much more responsive
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
    if(buttons[3]){rotateRight();}     // JOYSTICK BUTTON = ROTATE RIGHT
    if(buttons[4]){rotateLeft();}      // L BUTTON = ROTATE LEFT  
    if(buttons[5]){plummet();}         // UP JOYSTICK = HARD DROP
    if(buttons[6]){                    // M5 BUTTON = HOLD PIECE
      holdPiece(); // Simple direct call - no hold array complications
    }
    if(buttons[7]){pausegame();}       // R BUTTON = PAUSE
  }
}

void moveLeft(){
  if(test(0,-1)) {
    posX--;
    // Reset lock delay timer on successful movement
    if(lockDelayActive) {
      lockDelayStart = millis();
    }
  }
  showField(0,fieldy,1); // Full redraw to clear ghost trails
}

void moveDown(){
  if(test(1,0)){
    posY++;
    showField(0,fieldy,1); // Full redraw to clear ghost trails
    // Reset lock delay if piece moved down successfully
    lockDelayActive = false;
  } else{
    // Piece can't move down - start lock delay
    if(!lockDelayActive) {
      lockDelayActive = true;
      lockDelayStart = millis();
    }
    
    // Check if 500ms lock delay has expired
    if(millis() - lockDelayStart >= lockDelayTime) {
      collide=0; // Lock the piece
      lockDelayActive = false;
    }
    // Otherwise piece stays in position during lock delay
  }
  timez=millis();
}

void moveRight(){
  if(test(0,1)) {
    posX++;
    // Reset lock delay timer on successful movement
    if(lockDelayActive) {
      lockDelayStart = millis();
    }
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
  rot+=4;
  rot--;
  rot%=4;
  if(!test(0,0)){
    rot++;
    rot%=4;
  } else {
    // Reset lock delay timer on successful rotation
    if(lockDelayActive) {
      lockDelayStart = millis();
    }
  }
  showField(0,fieldy,1); // Full redraw to clear ghost trails
}

void rotateRight(){
  rot++;
  rot%=4;
  if(!test(0,0)){
    rot+=4;
    rot--;
    rot%=4;
  } else {
    // Reset lock delay timer on successful rotation
    if(lockDelayActive) {
      lockDelayStart = millis();
    }
  }
  showField(0,fieldy,1); // Full redraw to clear ghost trails
}

void pausegame(){
  delay(200);
  M5.Lcd.drawString("(PAUSED)",45,100,1);
  while (digitalRead(39)==1) {  // Wait for R button release (pause button)
  Serial.println("Game paused");
  delay(100);
  }
  M5.Lcd.fillRect(45,100,50,10,BLACK); // Clear pause text
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
      if(colorz){blockDraw(f,g,colorz);}
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

      blockDraw(blockposY,blockposX,color);
    }
  }
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
    pins[0]=check_Left();        // LEFT
  break; 
  case 1:
    pins[1]=check_Down();        // DOWN
  break;
  case 2:
    pins[2]=check_Right();       // RIGHT
  break;
  case 3:
    pins[3]=check_Btn();         // JOYSTICK BUTTON
  break;
  case 4:
    pins[4]=digitalRead(35);     // L BUTTON  
  break;
  case 5:
    pins[5]=check_Up();          // UP JOYSTICK 
  break;
  case 6:
    pins[6]=digitalRead(37);     // M5 BUTTON
  break;
  case 7:
    pins[7]=digitalRead(39);     // R BUTTON
  break;
  default:
  break;
  }
}

void scoreBoardWrite(int scorez){
  // Only called at game end - show final score and level
  M5.Lcd.drawString("GAME OVER!",30,100,1);
  M5.Lcd.drawString("Final Score: "+String(score),10,120,1);
  M5.Lcd.drawString("Level: "+String(level),10,140,1);
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
    
    // Speed info - show the ACTUAL FAST speed values we're using
    M5.Lcd.setTextSize(1);
    M5.Lcd.setTextColor(YELLOW);
    long actualSpeed;
    if(selectedLevel == 0) actualSpeed = 150;
    else if(selectedLevel == 1) actualSpeed = 130;
    else if(selectedLevel == 2) actualSpeed = 110;
    else if(selectedLevel == 3) actualSpeed = 90;
    else if(selectedLevel == 4) actualSpeed = 70;
    else if(selectedLevel == 5) actualSpeed = 60;
    else if(selectedLevel == 6) actualSpeed = 50;
    else if(selectedLevel == 7) actualSpeed = 40;
    else if(selectedLevel == 8) actualSpeed = 30;
    else if(selectedLevel == 9) actualSpeed = 20;
    else if(selectedLevel == 10) actualSpeed = 15;
    else if(selectedLevel == 11) actualSpeed = 12;
    else if(selectedLevel == 12) actualSpeed = 10;
    else if(selectedLevel == 13) actualSpeed = 8;
    else if(selectedLevel == 14) actualSpeed = 6;
    else if(selectedLevel == 15) actualSpeed = 5;
    else if(selectedLevel == 16) actualSpeed = 4;
    else if(selectedLevel == 17) actualSpeed = 3;
    else if(selectedLevel == 18) actualSpeed = 2;
    else if(selectedLevel >= 19) actualSpeed = 1;
    else actualSpeed = 150;
    
    M5.Lcd.drawString("Speed: " + String(actualSpeed) + "ms", 25, 120, 1);
    
    // Difficulty description - updated for FAST Tetris speeds 0-20
    M5.Lcd.setTextColor(WHITE);
    String difficulty;
    if(selectedLevel <= 2) difficulty = "Fast";
    else if(selectedLevel <= 5) difficulty = "Very Fast"; 
    else if(selectedLevel <= 8) difficulty = "Lightning";
    else if(selectedLevel <= 12) difficulty = "Ludicrous";
    else if(selectedLevel <= 16) difficulty = "UNREAL";
    else difficulty = "MATRIX MODE";
    M5.Lcd.drawString("Difficulty: " + difficulty, 20, 140, 1);
    
    // Visual level bars - show 0-20 with scrolling display
    M5.Lcd.setTextColor(CYAN);
    int startLevel = max(0, selectedLevel - 5);  // Show 6 levels centered on selection
    int endLevel = min(20, startLevel + 5);
    
    for(int i = 0; i <= 5 && (startLevel + i) <= 20; i++) {
      int level = startLevel + i;
      uint16_t barColor = (level == selectedLevel) ? RED : GREEN;
      if(level <= selectedLevel) {
        M5.Lcd.fillRect(10 + (i * 20), 160, 15, 15, barColor);
      } else {
        M5.Lcd.drawRect(10 + (i * 20), 160, 15, 15, DARKGREY);
      }
      // Draw level number
      M5.Lcd.setTextColor(WHITE);
      M5.Lcd.drawString(String(level), 12 + (i * 20), 178, 1);
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
        if(selectedLevel < 20) selectedLevel++;
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
  
  // Redraw cool tech colored borders that got cleared
  for(int x = 0; x <= 13; x++) {
    for(int y = 20; y < 240; y += 3) {
      int colorPattern = ((x + y/3) % 6);
      uint16_t borderColor;
      switch(colorPattern) {
        case 0: borderColor = 0x001F; break; case 1: borderColor = 0x07FF; break;
        case 2: borderColor = 0xF81F; break; case 3: borderColor = 0x07E0; break;
        case 4: borderColor = 0xFFE0; break; case 5: borderColor = 0xF800; break;
        default: borderColor = 0x001F; break;
      }
      M5.Lcd.drawPixel(x, y, borderColor);
      M5.Lcd.drawPixel(x, y+1, borderColor);
    }
  }
  
  for(int x = 124; x < 135; x++) {
    for(int y = 20; y < 240; y += 3) {
      int colorPattern = ((x + y/3) % 6);
      uint16_t borderColor;
      switch(colorPattern) {
        case 0: borderColor = 0x001F; break; case 1: borderColor = 0x07FF; break;
        case 2: borderColor = 0xF81F; break; case 3: borderColor = 0x07E0; break;
        case 4: borderColor = 0xFFE0; break; case 5: borderColor = 0xF800; break;
        default: borderColor = 0x001F; break;
      }
      M5.Lcd.drawPixel(x, y, borderColor);
      M5.Lcd.drawPixel(x, y+1, borderColor);
    }
  }
  
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

// ========== MODERN TETRIS FEATURES ==========

// HOLD PIECE Implementation
void holdPiece() {
  if(!canHold) return;  // Can only hold once per piece
  
  if(heldPiece == -1) {
    // No held piece - store current piece and get new one
    heldPiece = block;
    canHold = false;
    newPiece(false);  // Get new piece without placing current
  } else {
    // Swap current piece with held piece
    int tempPiece = block;
    block = heldPiece;
    heldPiece = tempPiece;
    
    // Reset piece position and rotation
    posY = 1;
    posX = 4;
    rot = 0;
    canHold = false;
  }
  
  drawHoldPiece();  // Update hold display
  showField(0, fieldy, 1);  // Refresh field
}

void drawMiniPiece(int pieceType, int x, int y, int scale) {
  if(pieceType < 0 || pieceType > 6) return;
  
  // Draw mini piece using piece data
  for(int i = 0; i < 4; i++) {
    int px = x + (piece[pieceType][0][1][i] * scale);
    int py = y + (piece[pieceType][0][0][i] * scale);
    
    // Use level-based colors like main pieces
    int levelOffset = (level % 10);
    int techColor = ((pieceType + levelOffset) % 8);
    uint16_t pieceColor;
    
    switch(techColor) {
      case 0: pieceColor = 0x001F; break; // Electric Blue
      case 1: pieceColor = 0x07E0; break; // Matrix Green  
      case 2: pieceColor = 0xF800; break; // Cyber Red
      case 3: pieceColor = 0x07FF; break; // Neon Cyan
      case 4: pieceColor = 0xF81F; break; // Electric Purple
      case 5: pieceColor = 0xFFE0; break; // Laser Yellow
      case 6: pieceColor = 0xF81F; break; // Tech Magenta
      case 7: pieceColor = 0xFFFF; break; // Circuit White
      default: pieceColor = 0x07E0; break;
    }
    
    M5.Lcd.fillRect(px, py, scale-1, scale-1, pieceColor);
  }
}

void drawHoldPiece() {
  // Clear hold area (top-left corner)
  M5.Lcd.fillRect(2, 2, 28, 28, BLACK);
  M5.Lcd.drawRect(1, 1, 30, 30, WHITE);
  
  // Draw "HOLD" label
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.setTextSize(1);
  M5.Lcd.drawString("HOLD", 4, 32, 1);
  
  if(heldPiece >= 0) {
    drawMiniPiece(heldPiece, 8, 8, 5);  // 5x5 pixel mini blocks in TOP-LEFT
  }
}

void drawNextPiece() {
  // Clear next area (top-right corner)  
  M5.Lcd.fillRect(106, 2, 28, 28, BLACK);
  M5.Lcd.drawRect(105, 1, 30, 30, WHITE);
  
  // Draw "NEXT" label
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.setTextSize(1);
  M5.Lcd.drawString("NEXT", 108, 32, 1);
  
  if(nextPiece >= 0) {
    drawMiniPiece(nextPiece, 112, 8, 5);  // 5x5 pixel mini blocks in TOP-RIGHT
  }
}

// ========== IR REMOTE CONTROL FUNCTIONS ==========

void irRemoteControl() {
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.setTextSize(1);
  M5.Lcd.drawString("⚡ FAST Tetris Remote 🎮", 5, 40);
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