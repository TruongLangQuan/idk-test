// dodge.h - Tetris Dodge game for M5StickC Plus2
#ifndef DODGE_H
#define DODGE_H

#include "config.h"

struct FallingPiece {
  int x, y;
  int type;  // 0-6 for tetromino, 7 for wall, 8+ for rectangles
  int rotation;
  bool active;
  float speed;
  int width, height;  // For wall/rectangle obstacles
};

class DodgeGame {
private:
  // Player (T piece)
  int playerX, playerY;
  int playerSize;
  int playerRotation;
  
  // Falling pieces
  static const int MAX_PIECES = 8;
  FallingPiece pieces[MAX_PIECES];
  
  int score;
  int level;
  unsigned long lastSpawnTime;
  unsigned long spawnInterval;
  unsigned long gameStartTime;
  int currentWave;
  bool gameOver;
  
  void spawnPiece();
  void updatePieces();
  void updateWave();
  void drawPiece(int x, int y, int type, int rotation, uint16_t color);
  void drawObstacle(int x, int y, int width, int height, uint16_t color);
  void drawLetter(int x, int y, int letterIdx, uint16_t color);
  void drawShape(int x, int y, int shapeIdx, uint16_t color);
  void drawPlayer();
  bool checkCollision(int px, int py, int pieceIdx);
  void clearPiece(int x, int y, int type, int rotation);
  void clearObstacle(int x, int y, int width, int height);
  void clearLetter(int x, int y, int letterIdx);
  void clearShape(int x, int y, int shapeIdx);
  
public:
  void init();
  void update();
  void draw();
  void handleInput();
  bool isGameOver() { return gameOver; }
  int getScore() { return score; }
  const char* getName() { return "DODGE"; }
};

extern DodgeGame dodgeGame;

#endif
