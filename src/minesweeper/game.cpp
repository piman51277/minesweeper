#include "game.h"

std::function<int()> randUint8 = std::bind(std::uniform_int_distribution<int>(0, 255), std::default_random_engine());

MinesweeperGame::MinesweeperGame()
{
  this->sprites = new Sprite[30];
  this->board = new uint8_t[30 * 13];
  this->boardState = new uint8_t[30 * 13];
  this->initizalized = false;
  this->gameOver = false;
  this->minesRemaining = 80;
  this->startTime = time(NULL);
  this->cursorX = 0;
  this->cursorY = 0;
}

MinesweeperGame::~MinesweeperGame()
{
  for (int i = 0; i < 30; i++)
  {
    delete[] this->sprites[i].pixels;
  }
  delete[] this->sprites;

  delete[] this->board;
  delete[] this->boardState;
}

void MinesweeperGame::init()
{
  this->loadSprites();
  this->generateBoard();
  std::fill(this->boardState, this->boardState + 30 * 13, 0);
  this->initizalized = true;
}

void MinesweeperGame::moveCursor(int x, int y)
{
  // clamp cursor to (0, 0) - (29, 12)
  this->cursorX += x;
  this->cursorY += y;
  if (this->cursorX < 0)
    this->cursorX = 0;
  if (this->cursorX > 29)
    this->cursorX = 29;
  if (this->cursorY < 0)
    this->cursorY = 0;
  if (this->cursorY > 12)
    this->cursorY = 12;
}

void MinesweeperGame::flag()
{
  if (this->gameOver)
    return;

  uint8_t state = this->boardState[this->cursorX + this->cursorY * 30];

  // revealed
  if (state & 0b10000000)
    return;

  // toggle flag
  if (state & 0b01000000)
  {
    this->boardState[this->cursorX + this->cursorY * 30] &= 0b00000000;
    this->minesRemaining++;
  }
  else
  {
    this->boardState[this->cursorX + this->cursorY * 30] |= 0b01000000;
    this->minesRemaining--;
  }
}

void MinesweeperGame::reveal()
{
  if (this->gameOver)
    return;

  uint8_t state = this->boardState[this->cursorX + this->cursorY * 30];

  // revealed
  if (state & 0b10000000)
    return;

  // flagged
  if (state & 0b01000000)
    return;

  // mine
  if (this->board[this->cursorX + this->cursorY * 30] == 9)
  {
    this->revealAllMines();
    this->gameOver = true;
    return;
  }

  this->revealTile(this->cursorX, this->cursorY);
}

void MinesweeperGame::reset()
{
  this->minesRemaining = 80;
  this->startTime = time(NULL);
  this->gameOver = false;
  this->generateBoard();
  std::fill(this->boardState, this->boardState + 30 * 13, 0);
}

void MinesweeperGame::render(uint32_t *pixels, uint16_t width, uint16_t height)
{
  if (!this->initizalized)
    return;

  // TODO: We could do this the smart way, simply shuffling sprites around
  //       instead of re-rendering everything every frame.

  this->displayEngine.clearSprites();

  // add all the required sprites
  for (int x = 0; x < 30; x++)
  {
    for (int y = 0; y < 13; y++)
    {
      uint8_t state = this->boardState[x + y * 30];
      bool revealed = state & 0b10000000;
      bool flagged = state & 0b01000000;

      uint8_t tile = this->board[x + y * 30];
      bool isMine = tile == 9;

      int baseSpriteID = 0;
      if (revealed)
      {
        if (isMine)
        {
          // has everything exploded yet?
          if (this->gameOver && !flagged)
          {
            baseSpriteID = 10;
          }
          else
          {
            // technically, this should never happen, but just in case
            baseSpriteID = 9;
          }
        }
        else
        {
          baseSpriteID = tile;
        }
      }
      else
      {
        baseSpriteID = 14;
      }

      int overlaySpriteID = -1;
      if (flagged)
      {
        // has everything exploded yet?
        if (this->gameOver && isMine)
        {
          overlaySpriteID = 12;
        }
        else
        {
          overlaySpriteID = 11;
        }
      }
      int xPixel = x * 16;
      int yPixel = y * 16 + 32;

      this->displayEngine.addSprite(&this->sprites[baseSpriteID], xPixel, yPixel, 1);
      if (overlaySpriteID != -1)
      {
        this->displayEngine.addSprite(&this->sprites[overlaySpriteID], xPixel, yPixel, 2);
      }
    }
  }

  // add the cursor
  this->displayEngine.addSprite(&this->sprites[13], this->cursorX * 16, this->cursorY * 16 + 32, 3);

  this->displayEngine.renderSprites(pixels, width, height);
}

void MinesweeperGame::loadSprites()
{
  this->sprites[0] = loadSprite("assets/compiled/tile0.sprite");
  this->sprites[1] = loadSprite("assets/compiled/tile1.sprite");
  this->sprites[2] = loadSprite("assets/compiled/tile2.sprite");
  this->sprites[3] = loadSprite("assets/compiled/tile3.sprite");
  this->sprites[4] = loadSprite("assets/compiled/tile4.sprite");
  this->sprites[5] = loadSprite("assets/compiled/tile5.sprite");
  this->sprites[6] = loadSprite("assets/compiled/tile6.sprite");
  this->sprites[7] = loadSprite("assets/compiled/tile7.sprite");
  this->sprites[8] = loadSprite("assets/compiled/tile8.sprite");
  this->sprites[9] = loadSprite("assets/compiled/mine.sprite");
  this->sprites[10] = loadSprite("assets/compiled/mine-exploded.sprite");
  this->sprites[11] = loadSprite("assets/compiled/flag.sprite");
  this->sprites[12] = loadSprite("assets/compiled/defused.sprite");
  this->sprites[13] = loadSprite("assets/compiled/cursor.sprite");
  this->sprites[14] = loadSprite("assets/compiled/unchecked.sprite");
}

void MinesweeperGame::generateBoard()
{
  // TODO: Only load in solveable boards
  //  generate the board
  for (int i = 0; i < 30 * 13; i++)
  {
    this->board[i] = 0;
  }

  for (int i = 0; i < 90; i++)
  {
    int x = randUint8() % 30;
    int y = randUint8() % 13;
    this->board[x + y * 30] = 9;
  }

  for (int x = 0; x < 30; x++)
  {
    for (int y = 0; y < 13; y++)
    {
      if (this->board[x + y * 30] == 9)
        continue;

      int count = 0;
      for (int dx = -1; dx <= 1; dx++)
      {
        for (int dy = -1; dy <= 1; dy++)
        {
          if (x + dx < 0 || x + dx >= 30 || y + dy < 0 || y + dy >= 13)
            continue;

          if (this->board[(x + dx) + (y + dy) * 30] == 9)
            count++;
        }
      }

      this->board[x + y * 30] = count;
    }
  }
}

void MinesweeperGame::revealTile(int x, int y)
{
  uint8_t state = this->boardState[x + y * 30];
  bool revealed = state & 0b10000000;
  bool flagged = state & 0b01000000;

  // revealed
  if (revealed)
    return;

  // flagged
  if (flagged)
    return;

  // mine
  if (this->board[x + y * 30] == 9)
    return;

  this->boardState[x + y * 30] |= 0b10000000;

  // if the tile is empty, reveal all adjacent tiles
  if (this->board[x + y * 30] == 0)
  {
    for (int dx = -1; dx <= 1; dx++)
    {
      for (int dy = -1; dy <= 1; dy++)
      {
        if (x + dx < 0 || x + dx >= 30 || y + dy < 0 || y + dy >= 13)
          continue;

        this->revealTile(x + dx, y + dy);
      }
    }
  }
}

void MinesweeperGame::revealAllMines()
{
  this->gameOver = true;
  for (int x = 0; x < 30; x++)
  {
    for (int y = 0; y < 13; y++)
    {
      if (this->board[x + y * 30] == 9)
      {
        this->boardState[x + y * 30] |= 0b10000000;
      }
    }
  }
}