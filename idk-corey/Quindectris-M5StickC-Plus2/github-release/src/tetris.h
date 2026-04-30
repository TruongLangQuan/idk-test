// tetris.h - ThreeTris game for M5StickC Plus2 (3-block pieces!)
#ifndef TETRIS_H
#define TETRIS_H

#include "config.h"

#define BLOCK_SIZE 10
#define FIELD_WIDTH 10
#define FIELD_HEIGHT 23
#define OFFSET_X 5
#define OFFSET_Y 10

class TetrisGame {
private:
  uint8_t field[FIELD_HEIGHT][FIELD_WIDTH];
  int currentPiece;
  int currentRot;
  int posX, posY;
  int score;
  int level;
  unsigned long lastDropTime;
  unsigned long lockDelayStart;
  bool lockDelayActive;
  int dropSpeed;
  bool gameOver;
  bool needsRedraw;  // Flag to force full redraw
  
  // Modern features
  int heldPiece;
  int nextPiece;
  bool canHold;
  int linesCleared;
  
  int pieces[8][4][2][3];  // 8 pieces: I, L, J, Single, Double, Diag2, Diag3, T-minus
  uint16_t pieceColors[8];
  
  bool test(int y, int x, int piece, int rot);
  void placePiece();
  void clearLines();
  void newPiece(bool setPiece);
  void drawGhostPiece();
  void drawHoldPiece();
  void drawNextPiece();
  void drawMiniPiece(int pieceType, int x, int y, int scale);
  int calculateDropDistance();
  void holdPiece();
  
public:
  void init();
  void update();
  void draw();
  void handleInput();
  bool isGameOver() { return gameOver; }
  int getScore() { return score; }
  const char* getName() { return "THREETRIS"; }
};

extern TetrisGame tetrisGame;

#endif
