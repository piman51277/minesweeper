#include "game.h"

std::function<int()> randUint8 = std::bind(std::uniform_int_distribution<int>(0, 255), std::default_random_engine());

MinesweeperGame::MinesweeperGame()
{
  this->sprites = new Sprite[30];
  this->board = new uint8_t[30 * 13];
  this->boardState = new uint8_t[30 * 13];
  this->boardPoolSize = 0;
  this->boardPool = nullptr;
  this->hasFirstMove = false;
  this->initizalized = false;
  this->gameState = 0;
  this->minesRemaining = 80;
  this->startTime = time(NULL);
  this->endTime = time(NULL);
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
  delete[] this->boardPool;
  delete[] this->board;
  delete[] this->boardState;
}

void MinesweeperGame::init()
{
  this->loadSprites();
  this->loadBoardPool();
  this->generateFakeBoard();
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
  if (this->gameState != 0)
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

  if (this->hasWon())
  {
    this->revealAllMines();
    this->gameState = 1;
    this->endTime = time(NULL);
  }
}

void MinesweeperGame::reveal()
{
  // first move is always safe
  if (!this->hasFirstMove)
  {
    this->hasFirstMove = true;
    this->generateBoard(this->cursorX, this->cursorY);
  }

  if (this->gameState != 0)
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
    this->gameState = 2;
    this->endTime = time(NULL);
    return;
  }

  this->revealTile(this->cursorX, this->cursorY);

  if (this->hasWon())
  {
    this->revealAllMines();
    this->gameState = 1;
    this->endTime = time(NULL);
  }
}

void MinesweeperGame::reset()
{
  this->minesRemaining = 80;
  this->startTime = time(NULL);
  this->endTime = time(NULL);
  this->gameState = 0;
  this->hasFirstMove = false;
  this->generateFakeBoard();
  std::fill(this->boardState, this->boardState + 30 * 13, 0);
}

void MinesweeperGame::render(uint32_t *pixels, uint16_t width, uint16_t height)
{
  if (!this->initizalized)
    return;

  // TODO: We could do this the smart way, simply shuffling sprites around
  //       instead of re-rendering everything every frame.

  this->displayEngine.clearSprites();

  // topbar

  // background
  for (int x = 0; x <= 480 - 32; x += 32)
  {
    this->displayEngine.addSprite(&this->sprites[15], x, 0, 1);
  }

  // smiley
  int smileyID = 16;
  if (this->gameState == 1)
  {
    smileyID = 18;
  }
  else if (this->gameState == 2)
  {
    smileyID = 17;
  }
  this->displayEngine.addSprite(&this->sprites[smileyID], 480 / 2 - 16, 0, 2);

  // timer
  time_t currentTime = time(NULL);
  int timeElapsed = this->gameState != 0 ? this->endTime - this->startTime : currentTime - this->startTime;

  int time100 = timeElapsed / 100;
  int time10 = (timeElapsed / 10) % 10;
  int time1 = timeElapsed % 10;

  this->displayEngine.addSprite(&this->sprites[20 + time100], 480 - 48, 0, 2);
  this->displayEngine.addSprite(&this->sprites[20 + time10], 480 - 32, 0, 2);
  this->displayEngine.addSprite(&this->sprites[20 + time1], 480 - 16, 0, 2);

  // mines remaining
  int mines10 = (this->minesRemaining / 10) % 10;
  int mines1 = this->minesRemaining % 10;

  // since we actually only have 80 mines, this one is special
  int mines100 = 19;
  if (this->minesRemaining < 0)
  {
    mines100 = 30;
    mines10 = (-this->minesRemaining / 10) % 10;
    mines1 = -this->minesRemaining % 10;
  }

  this->displayEngine.addSprite(&this->sprites[mines100], 0, 0, 2);
  this->displayEngine.addSprite(&this->sprites[20 + mines10], 16, 0, 2);
  this->displayEngine.addSprite(&this->sprites[20 + mines1], 32, 0, 2);

  // the grid
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
          if (this->gameState == 2 && !flagged)
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
        if (this->gameState != 0 && isMine)
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
  this->sprites[15] = loadSprite("assets/compiled/top-tile.sprite");
  this->sprites[16] = loadSprite("assets/compiled/state-normal.sprite");
  this->sprites[17] = loadSprite("assets/compiled/state-loss.sprite");
  this->sprites[18] = loadSprite("assets/compiled/state-victory.sprite");
  this->sprites[19] = loadSprite("assets/compiled/7seg-empty.sprite");
  this->sprites[20] = loadSprite("assets/compiled/7seg-0.sprite");
  this->sprites[21] = loadSprite("assets/compiled/7seg-1.sprite");
  this->sprites[22] = loadSprite("assets/compiled/7seg-2.sprite");
  this->sprites[23] = loadSprite("assets/compiled/7seg-3.sprite");
  this->sprites[24] = loadSprite("assets/compiled/7seg-4.sprite");
  this->sprites[25] = loadSprite("assets/compiled/7seg-5.sprite");
  this->sprites[26] = loadSprite("assets/compiled/7seg-6.sprite");
  this->sprites[27] = loadSprite("assets/compiled/7seg-7.sprite");
  this->sprites[28] = loadSprite("assets/compiled/7seg-8.sprite");
  this->sprites[29] = loadSprite("assets/compiled/7seg-9.sprite");
  this->sprites[30] = loadSprite("assets/compiled/7seg-neg.sprite");
}

void MinesweeperGame::loadBoardPool()
{
  std::fstream file("assets/compiled/boards.bin");

  if (!file.good())
  {
    std::cout << "File not found: assets/compiled/boards.bin" << std::endl;

    // this will cause a crash, but it's better than silently failing
    return;
  }

  file.read((char *)&this->boardPoolSize, 2);
  this->boardPool = new uint8_t[this->boardPoolSize * (15 * 13)];
  file.read((char *)this->boardPool, this->boardPoolSize * 15 * 13);
  file.close();
}

void MinesweeperGame::generateFakeBoard()
{
  // until the user makes the first move, the board is not generated
  // this just makes a placeholder board
  // this is so the first move is always safe

  for (int i = 0; i < 30 * 13; i++)
  {
    this->board[i] = 0;
  }
}

void MinesweeperGame::generateBoard(int firstX, int firstY)
{
  static int nextBoard = 0;

  // this finds a board where the first move is safe
  int moveIndex = firstX + firstY * 30;

  while (true)
  {
    // create a pointer to the start of the board
    // its 2 tiles per byte, so its (30 / 2) * 13
    uint8_t *boardEntry = this->boardPool + nextBoard * 15 * 13;

    // look at the specific nibble containing the first move
    uint8_t nibble = boardEntry[moveIndex / 2];

    if ((moveIndex % 2) == 0)
    {
      nibble = nibble >> 4;
    }

    // if it is 2, it's ok
    if (nibble & 0x02)
    {
      // unpack the board
      for (int i = 0; i < 15 * 13; i++)
      {
        uint8_t chunk = boardEntry[i];
        bool isFirstMine = chunk & 0x10;
        bool isSecondMine = chunk & 0x01;

        if (isFirstMine)
          this->board[i * 2] = 9;
        else
          this->board[i * 2] = 0;

        if (isSecondMine)
          this->board[i * 2 + 1] = 9;
        else
          this->board[i * 2 + 1] = 0;
      }

      // fill in the numbered tiles
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
              if (x + dx >= 0 && x + dx < 30 && y + dy >= 0 && y + dy < 13)
              {
                if (this->board[x + dx + (y + dy) * 30] == 9)
                {
                  count++;
                }
              }
            }
          }
          this->board[x + y * 30] = count;
        }
      }

      break;
    }

    nextBoard = (nextBoard + 1) % this->boardPoolSize;
  }

  // force the next board to be different
  nextBoard = (nextBoard + 1) % this->boardPoolSize;
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

bool MinesweeperGame::hasWon()
{
  if (this->minesRemaining != 0)
    return false;

  // All tiles must be either revealed or flagged
  for (int x = 0; x < 30; x++)
  {
    for (int y = 0; y < 13; y++)
    {
      uint8_t state = this->boardState[x + y * 30];
      bool revealed = state & 0b10000000;
      bool flagged = state & 0b01000000;

      if (this->board[x + y * 30] == 9)
      {
        if (!flagged)
          return false;
      }
      else
      {
        if (!revealed)
          return false;
      }
    }
  }

  return true;
}