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

int piece[7][4][2][4]={{{{0,1,0,1},{0,0,1,1}},{{0,1,0,1},{0,0,1,1}},{{0,1,0,1},{0,0,1,1}},{{0,1,0,1},{0,0,1,1}}},
                        {{{0,-1,0,1},{0,0,1,1}},{{0,1,1,0},{0,0,-1,1}},{{0,-1,0,1},{0,0,1,1}},{{0,1,1,0},{0,0,-1,1}}},
                        {{{0,-1,0,1},{0,0,-1,-1}},{{0,0,1,1},{0,-1,0,1}},{{0,-1,0,1},{0,0,-1,-1}},{{0,0,1,1},{0,-1,0,1}}},
                        {{{1,0,-1,1},{0,0,0,-1}},{{0,-1,0,0},{0,0,1,2}},{{0,1,2,0},{0,0,0,1}},{{1,0,0,0},{1,1,0,-1}}},
                        {{{1,0,0,0},{-1,-1,0,1}},{{1,1,0,-1},{1,0,0,0}},{{-1,0,0,0},{1,1,0,-1}},{{0,0,1,2},{-1,0,0,0}}},
                        {{{1,0,-1,0},{0,0,0,1}},{{0,0,0,-1},{-1,0,1,0}},{{1,0,-1,0},{0,0,0,-1}},{{0,0,0,1},{-1,0,1,0}}},
                        {{{0,0,0,0},{-1,0,1,2}},{{-1,0,1,2},{0,0,0,0}},{{0,0,0,0},{-1,0,1,2}},{{-1,0,1,2},{0,0,0,0}}}};
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
  
  // Add "HAPPY HALLOWEEN" in bigger font above "Press M5"
  M5.Lcd.setTextColor(0xF800); // Orange color  
  M5.Lcd.setTextSize(2);        // Bigger font size
  M5.Lcd.drawString("HAPPY", 30, 100, 1);
  M5.Lcd.drawString("HALLOWEEN!!", 0, 118, 1);
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
        
        // Redraw "HAPPY HALLOWEEN!!" after IR remote
        M5.Lcd.setTextColor(0xF800);
        M5.Lcd.setTextSize(2);
        M5.Lcd.drawString("HAPPY", 30, 100, 1);
        M5.Lcd.drawString("HALLOWEEN!!", 0, 118, 1);
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
  
  // Add "HAPPY HALLOWEEN" text in orange at center of screen
  M5.Lcd.setTextColor(0xF800); // Orange color
  M5.Lcd.setTextSize(1);
  M5.Lcd.drawString("HAPPY", 50, 100, 1);      // Center horizontal, middle vertical
  M5.Lcd.drawString("HALLOWEEN", 35, 115, 1);  // Center horizontal, below HAPPY
}

void loop(){
  // Show level select screen
  int startingLevel = levelSelect();
  
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
    if(game!=0)
    newPiece(1);
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
  // Halloween color scheme
  int red=0;
  int gre=0;
  int blu=0;
  
  // Map colors to Halloween theme
  int halloweenColor = (colorz % 7); // 7 Halloween colors
  
  switch(halloweenColor) {
    case 0: // Pumpkin Orange
      red = 31; gre = 15; blu = 0;
      break;
    case 1: // Deep Purple (Witch)
      red = 15; gre = 0; blu = 31;
      break;
    case 2: // Blood Red
      red = 31; gre = 0; blu = 0;
      break;
    case 3: // Toxic Green
      red = 0; gre = 31; blu = 0;
      break;
    case 4: // Ghostly White
      red = 28; gre = 28; blu = 28;
      break;
    case 5: // Midnight Black
      red = 5; gre = 5; blu = 5;
      break;
    case 6: // Golden Yellow
      red = 31; gre = 25; blu = 0;
      break;
    default:
      red = 31; gre = 15; blu = 0; // Default to orange
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
  posY=1;
  posX=4;
  rot=0;
  block=random(7);
  color=random(96)+1;
  showField(0,20,1);
  if(game!=0){
    game=test(0,0);    
  }
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
    if(buttons[4]){rotateLeft();}
    if(buttons[5]){plummet();}
    if(buttons[6]){pausegame();}
    if(buttons[7]){reset();}
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
  //M5.Lcd.drawLine(0,16,135,16,WHITE);
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
    else actualSpeed = 750;
    
    M5.Lcd.drawString("Speed: " + String(actualSpeed) + "ms", 25, 120, 1);
    
    // Difficulty description - updated for new speed ranges
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
  
  // Redraw Halloween colored borders that got cleared
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