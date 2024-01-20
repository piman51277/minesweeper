#pragma once
#include <iostream>
#include <fstream>
#include <cinttypes>
#include <string>
#include <vector>
#include <algorithm>

struct Sprite
{
  uint16_t width;
  uint16_t height;
  uint32_t *pixels;
};

struct SpriteEntry
{
  Sprite *sprite;
  uint16_t x;
  uint16_t y;
  // Higher z index means the sprite is rendered on top of other sprites
  // 0 is do not render
  uint16_t z_index;
};

struct ARGB
{
  float a;
  uint8_t r;
  uint8_t g;
  uint8_t b;
};

/**
 * Loads a sprite from a file
 * @param path The path to the sprite file
 * @return The sprite
 */
Sprite loadSprite(std::string path);

/**
 * Deconstructs a uint32_t into an ARGB struct
 * @param pixel The pixel to deconstruct
 * @return The ARGB struct
 */
ARGB parseARGB8888(uint32_t pixel);

/**
 * Constructs a uint32_t from an ARGB struct
 * @param argb The ARGB struct
 * @return The uint32_t
 */
uint32_t constructARGB8888(ARGB argb);

/**
 * Applies the composition formula to two ARGB structs
 * @param a The first ARGB struct
 * @param b The second ARGB struct
 * @return The composited ARGB struct
 */
ARGB composePair(ARGB a, ARGB b);

// composes pixels, respecting alpha
uint32_t composePixels(uint32_t *pixels, uint8_t pixelCount);

class SpriteEngine
{
public:
  SpriteEngine();
  ~SpriteEngine();

  std::vector<SpriteEntry *> sprites;

  /**
   * Adds a sprite to the sprite engine
   * @param sprite The sprite to add
   * @param x The x position of the sprite
   * @param y The y position of the sprite
   * @param [z_index] The z index of the sprite
   */
  SpriteEntry *addSprite(Sprite *sprite, uint16_t x, uint16_t y, uint16_t z_index);
  SpriteEntry *addSprite(Sprite *sprite, uint16_t x, uint16_t y);

  /**
   * Moves a sprite to a new position
   * @param sprite The sprite to move
   * @param x The new x position of the sprite
   * @param y The new y position of the sprite
   * @param [z_index] The new z index of the sprite
   */
  void moveSprite(SpriteEntry *sprite, uint16_t x, uint16_t y, uint16_t z_index);
  void moveSprite(SpriteEntry *sprite, uint16_t x, uint16_t y);

  /**
   * Removes a sprite from the sprite engine. This is O(n) so it's not recommended to call this every frame.
   * Setting the z index to 0 is a better option.
   * WARNING: This does not free the SpriteEntry. You must free it yourself.
   * @param sprite The sprite to remove
   */
  void removeSprite(SpriteEntry *sprite);

  /**
   * Removes all sprites from the sprite engine
   */
  void clearSprites();

  /**
   * Renders all sprites to a pixel array
   * @param pixels The pixel array to render to
   * @param width The width of the pixel array
   * @param height The height of the pixel array
   */
  void renderSprites(uint32_t *pixels, uint16_t width, uint16_t height);

private:
  uint16_t next_z_index;

  /**
   * Reshuffles the sprites vector so that the sprites
   * are sorted by z index high-to-low
   */
  void reshuffleSprites();
};