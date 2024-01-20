#pragma once
#include "../spritelib/sprites.h"
#include <time.h>
#include <random>
#include <functional>

extern std::function<int()> randUInt8;

/** Sprite ID list (WIP)
 * 0-8: number of adjacent mines tile
 * 9: mine (unexploded)
 * 10: mine (exploded)
 * 11: flag (unexploded)
 * 12: flag (exploded)
 * 13: cursor
 * 14: unrevealed tile
 * ...
 */

class MinesweeperGame
{
public:
  MinesweeperGame();
  ~MinesweeperGame();

  int cursorX;
  int cursorY;

  void init();

  void moveCursor(int x, int y);
  void flag();
  void reveal();
  void reset();

  void render(uint32_t *pixels, uint16_t width, uint16_t height);

private:
  SpriteEngine displayEngine;
  Sprite *sprites;

  /**
   * 0: empty
   * 1-8: number of adjacent mines
   * 9: mine
   */
  uint8_t *board;

  /**
   * 1 bit: revealed
   * 1 bit: flagged
   * 6 bits: unused
   */
  uint8_t *boardState;
  int minesRemaining;
  time_t startTime;
  bool gameOver;
  bool initizalized;

  void loadSprites();

  void generateBoard();

  void revealTile(int x, int y);

  void revealAllMines();
};