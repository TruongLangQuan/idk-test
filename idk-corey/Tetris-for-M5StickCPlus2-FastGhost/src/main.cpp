#include <M5StickCPlus2.h>
#include "tet.h"
#include "wifi_beacon.h"
#include "wifi_scanner.h"
#include <IRsend.h>  // IR library for remote control

#define PIN_UP 32
#define PIN_DOWN 33
#define PIN_LEFT 25
#define PIN_RIGHT 26
#define PIN_CENTER 0

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

// JoyC removed

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
  
  // Initialize 5-way switch pins
  pinMode(PIN_UP, INPUT_PULLUP);
  pinMode(PIN_DOWN, INPUT_PULLUP);
  pinMode(PIN_LEFT, INPUT_PULLUP);
  pinMode(PIN_RIGHT, INPUT_PULLUP);
  pinMode(PIN_CENTER, INPUT_PULLUP);

  Disp.setRotation(0);
  BITMAP;
  Disp.fillRect(0,0,135,18,BLACK);
  M5.Lcd.setTextSize(1);
  M5.Lcd.drawString("v.1.5.0",4,8,1);
  M5.Lcd.drawString((String)M5.Power.getBatteryLevel()+"%",107,8,1);
  M5.Lcd.drawString("Loading...",39,134,1);
  
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
    
    // Check for WiFi scanner mode (Up on switch)
    if(digitalRead(PIN_UP) == LOW) {
      delay(500); // Hold time to confirm
      if(digitalRead(PIN_UP) == LOW) {
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
    
    // Check for IR Remote mode (Down on switch) 
    if(digitalRead(PIN_DOWN) == LOW) {
      delay(500); // Hold time to confirm
      if(digitalRead(PIN_DOWN) == LOW) {
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
  
  // Initialize next piece
  nextPiece = random(7);
}

void loop(){
  // Show level select screen
  int startingLevel = levelSelect();
  
  // Reset modern features
  heldPiece = -1;
  canHold = true;
  
  // Calculate proper speed for selected level - make it VERY obvious
  // Level 0 = 1000ms (slow), Level 5 = 300ms (fast), Level 9 = 100ms (insane)
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
  else speed = 750; // fallback
  
  timez=millis();
  score=0;
  level=startingLevel; // Start at selected level
  stage=startingLevel / 10; // Set proper stage based on level
  lineCount=0; // Reset line counter for proper speed progression
  newPiece(0);
  game=1;
  showField(0,fieldy,1);
  while(game){
    collide=1;
    multiplier=1;
    scoreBoardWrite(score);
      while(collide){
        wait();
        if(collide) moveDown();
      }
    if(game!=0) {
      newPiece(1);
      canHold = true; // Reset hold capability when piece locks
    }
  }
  for(a=0;a<fieldy;a++){
    for(b=0;b<fieldx;b++){
      field[a][b]=0;
    }
  }
  delay(1500);
  M5.Lcd.drawString("            ",36,135,1); //Erase Reset msg
  M5.Lcd.drawString("Sco.:00     ",10,4,1);
}

//==============================JoyC==============================
int check_Btn(){
  return digitalRead(PIN_CENTER) == LOW ? 0 : 1;
}
int check_Up(){
  return digitalRead(PIN_UP) == LOW ? 0 : 1;
}
int check_Right(){
  return digitalRead(PIN_RIGHT) == LOW ? 0 : 1;
}
int check_Left(){
  return digitalRead(PIN_LEFT) == LOW ? 0 : 1;
}
int check_Down() {
  return digitalRead(PIN_DOWN) == LOW ? 0 : 1;
}
//================================================================

void blockDraw(int y, int x, int colorz){
  // Modern colors
  int red=0;
  int gre=0;
  int blu=0;
  
  // Map colors 
  int tetColor = (colorz % 7); 
  
  switch(tetColor) {
    case 0: // Cyan (I)
      red = 0; gre = 31; blu = 31;
      break;
    case 1: // Yellow (O)
      red = 31; gre = 31; blu = 0;
      break;
    case 2: // Purple (T)
      red = 15; gre = 0; blu = 31;
      break;
    case 3: // Green (S)
      red = 0; gre = 31; blu = 0;
      break;
    case 4: // Red (Z)
      red = 31; gre = 0; blu = 0;
      break;
    case 5: // Blue (J)
      red = 0; gre = 0; blu = 31;
      break;
    case 6: // Orange (L)
      red = 31; gre = 15; blu = 0;
      break;
    default:
      red = 31; gre = 15; blu = 0; 
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
  
  // Use pre-generated next piece
  block = nextPiece;
  nextPiece = random(7); // Generate piece for next turn
  
  posY=1;
  posX=4;
  rot=0;
  color=block+1; // Fixed colors for pieces
  
  // Redraw previews
  drawNextPiece();
  drawHoldPiece();
  
  showField(0,20,1);
  if(game!=0){
    game=test(0,0);    
  }
}

void holdPiece() {
  if (!canHold) return; // Only one hold per drop
  
  int temp = heldPiece;
  heldPiece = block;
  
  if (temp == -1) {
    // First time holding, spawn new piece
    block = nextPiece;
    nextPiece = random(7);
  } else {
    // Swap with previously held piece
    block = temp;
  }
  
  posY = 1;
  posX = 4;
  rot = 0;
  color = block + 1;
  canHold = false; // Disable holding until next piece locks
  
  // Clear the field (hide current piece at old position)
  showField(0, fieldy, 0);
  
  // Redraw previews
  drawNextPiece();
  drawHoldPiece();
  
  // Redraw field with new piece
  showField(0, fieldy, 1);
}

void drawMiniPiece(int pieceType, int x, int y, int scale) {
  if (pieceType == -1) return;
  
  uint16_t colors[] = {0x07FF, 0xFFE0, 0xF81F, 0x07E0, 0xF800, 0x001F, 0xFD20};
  uint16_t c = colors[pieceType % 7];
  
  for (int i = 0; i < 4; i++) {
    int px = x + piece[pieceType][0][1][i] * scale;
    int py = y + piece[pieceType][0][0][i] * scale;
    M5.Lcd.fillRect(px, py, scale - 1, scale - 1, c);
  }
}

void drawHoldPiece() {
  // Clear hold area
  M5.Lcd.fillRect(1, 40, 12, 30, BLACK);
  M5.Lcd.drawRect(0, 38, 14, 34, WHITE);
  M5.Lcd.setTextSize(1);
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.setCursor(1, 30);
  M5.Lcd.print("H");
  
  if (heldPiece != -1) {
    drawMiniPiece(heldPiece, 3, 48, 4);
  }
}

void drawNextPiece() {
  // Clear next area
  M5.Lcd.fillRect(123, 40, 12, 30, BLACK);
  M5.Lcd.drawRect(121, 38, 14, 34, WHITE);
  M5.Lcd.setTextSize(1);
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.setCursor(123, 30);
  M5.Lcd.print("N");
  
  drawMiniPiece(nextPiece, 123, 48, 4);
}

void wait(){
  timez+=speed;
  while(millis()<(timez+speed)){
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
    if(buttons[4]){holdPiece();} // Btn 4 (GPIO 35) = Hold
    if(buttons[5]){rotateLeft();} // Joystick Up = Rotate Left
    if(buttons[6]){pausegame();}
    if(buttons[7]){plummet();} // Btn 7 (GPIO 39) = Hard Drop
  }
}

void moveLeft(){
  posX-=test(0,-1);
  showField(0,fieldy,1); // Full redraw to clear ghost trails
}

void moveDown(){
  if(test(1,0)){
    posY++;
    showField(0,fieldy,1); // Full redraw to clear ghost trails
  } else{
    collide=0;
  }
  timez=millis();
}

void moveRight(){
  posX+=(test(0,1));
  showField(0,fieldy,1); // Full redraw to clear ghost trails
}

void plummet(){
  multiplier=2;
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
  M5.Lcd.drawString("Sco.:0"+String(score),10,4,1);
  M5.Lcd.drawString("Level:"+String(level),84,4,1);
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
    
    // Speed info
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
    else actualSpeed = 750;
    
    M5.Lcd.drawString("Speed: " + String(actualSpeed) + "ms", 25, 120, 1);
    
    // Difficulty description
    M5.Lcd.setTextColor(WHITE);
    String difficulty;
    if(selectedLevel <= 1) difficulty = "Beginner";
    else if(selectedLevel <= 3) difficulty = "Intermediate"; 
    else if(selectedLevel <= 6) difficulty = "Advanced";
    else if(selectedLevel <= 8) difficulty = "Expert";
    else difficulty = "INSANE";
    M5.Lcd.drawString("Difficulty: " + difficulty, 20, 140, 1);
    
    // Visual level bars
    M5.Lcd.setTextColor(CYAN);
    for(int i = 0; i <= selectedLevel && i < 10; i++) {
      M5.Lcd.fillRect(10 + (i * 11), 160, 8, 20, GREEN);
    }
    for(int i = selectedLevel + 1; i < 10; i++) {
      M5.Lcd.drawRect(10 + (i * 11), 160, 8, 20, DARKGREY);
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
        if(selectedLevel < 9) selectedLevel++;
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
  
  // Redraw borders
  for(int x = 0; x <= 13; x++) {
    for(int y = 20; y < 240; y += 4) {
      int colorPattern = ((x + y/4) % 7);
      uint16_t borderColor;
      switch(colorPattern) {
        case 0: borderColor = 0xF800; break; case 1: borderColor = 0x7800; break;
        case 2: borderColor = 0xF81F; break; case 3: borderColor = 0x07E0; break;
        case 4: borderColor = 0xFFE0; break; case 5: borderColor = 0x8010; break;
        case 6: borderColor = 0xFC00; break; default: borderColor = 0xF800; break;
      }
      M5.Lcd.drawPixel(x, y, borderColor);
      M5.Lcd.drawPixel(x, y+1, borderColor);
      M5.Lcd.drawPixel(x, y+2, borderColor);
    }
  }
  
  for(int x = 124; x < 135; x++) {
    for(int y = 20; y < 240; y += 4) {
      int colorPattern = ((x + y/4) % 7);
      uint16_t borderColor;
      switch(colorPattern) {
        case 0: borderColor = 0xF800; break; case 1: borderColor = 0x7800; break;
        case 2: borderColor = 0xF81F; break; case 3: borderColor = 0x07E0; break;
        case 4: borderColor = 0xFFE0; break; case 5: borderColor = 0x8010; break;
        case 6: borderColor = 0xFC00; break; default: borderColor = 0xF800; break;
      }
      M5.Lcd.drawPixel(x, y, borderColor);
      M5.Lcd.drawPixel(x, y+1, borderColor);
      M5.Lcd.drawPixel(x, y+2, borderColor);
    }
  }
  
  return selectedLevel;
}

int calculateDropDistance() {
  int dropDistance = 0;
  int originalPosY = posY;
  
  for(int testY = posY + 1; testY < fieldy; testY++) {
    boolean canDrop = true;
    for(int i = 0; i < 4; i++) {
      int blockY = testY + piece[block][rot][0][i];
      int blockX = posX + piece[block][rot][1][i];
      if(blockY >= fieldy || blockX < 0 || blockX >= fieldx || 
         (blockY >= 0 && field[blockY][blockX] > 0)) {
        canDrop = false;
        break;
      }
    }
    if(canDrop) dropDistance = testY - originalPosY;
    else break;
  }
  return dropDistance;
}

void drawGhostPiece() {
  int dropDistance = calculateDropDistance();
  if(dropDistance > 0) {
    int ghostY = posY + dropDistance;
    for(int i = 0; i < 4; i++) {
      int blockY = ghostY + piece[block][rot][0][i];
      int blockX = posX + piece[block][rot][1][i];
      if(blockY >= 0 && blockY < fieldy && blockX >= 0 && blockX < fieldx) {
        boolean isCurrentPiece = false;
        for(int j = 0; j < 4; j++) {
          if(blockY == posY + piece[block][rot][0][j] && blockX == posX + piece[block][rot][1][j]) {
            isCurrentPiece = true; break;
          }
        }
        if(!isCurrentPiece && field[blockY][blockX] == 0) {
          int posx = (blockX * blockSize) + offsetx;
          int posy = (blockY * blockSize) + offsety;
          M5.Lcd.drawRect(posx, posy, blockSize-1, blockSize-1, DARKGREY);
        }
      }
    }
  }
}

void clearGhostPiece() {}

// ========== IR REMOTE CONTROL FUNCTIONS ==========
void irRemoteControl() {
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.drawString("🎃 Halloween Remote 📺", 5, 40);
  sendUniversalPower();
  delay(2000);
}

void sendUniversalPower() {
  irsend.sendSony(0xA90, 12); delay(100);
  irsend.sendSAMSUNG(0xE0E040BF); delay(100);
  irsend.sendLG(0x20DF10EF); delay(100);
  irsend.sendNEC(0xFF02FD); delay(100);
}
