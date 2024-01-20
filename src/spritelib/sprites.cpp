#include "sprites.h"

Sprite loadSprite(std::string path)
{
  std::fstream file(path);

  if (!file.good())
  {
    std::cout << "File not found: " << path << std::endl;
    return Sprite();
  }

  uint8_t paletteSize;
  file.read((char *)&paletteSize, 1);

  uint16_t width;
  file.read((char *)&width, 2);

  uint16_t height;
  file.read((char *)&height, 2);

  uint32_t *palette = new uint32_t[paletteSize];
  file.read((char *)palette, paletteSize * 4);

  uint8_t *pixels = new uint8_t[width * height];
  file.read((char *)pixels, width * height);

  file.close();

  uint32_t *pixelColors = new uint32_t[width * height];
  for (int i = 0; i < width * height; i++)
  {
    pixelColors[i] = palette[pixels[i]];
  }

  delete[] palette;

  Sprite sprite = {width, height, pixelColors};
  return sprite;
}

ARGB parseARGB8888(uint32_t pixel)
{
  ARGB argb;
  argb.a = ((pixel >> 24) & 0xFF) / 255.0f;
  argb.r = (pixel >> 16) & 0xFF;
  argb.g = (pixel >> 8) & 0xFF;
  argb.b = pixel & 0xFF;
  return argb;
}

uint32_t constructARGB8888(ARGB argb)
{
  uint32_t pixel = 0;
  pixel |= ((uint32_t)(argb.a * 255)) << 24;
  pixel |= ((uint32_t)argb.r) << 16;
  pixel |= ((uint32_t)argb.g) << 8;
  pixel |= ((uint32_t)argb.b);
  return pixel;
}

ARGB composePair(ARGB a, ARGB b)
{
  ARGB result;
  result.a = a.a + b.a * (1 - a.a);
  result.r = (a.r * a.a + b.r * b.a * (1 - a.a)) / result.a;
  result.g = (a.g * a.a + b.g * b.a * (1 - a.a)) / result.a;
  result.b = (a.b * a.a + b.b * b.a * (1 - a.a)) / result.a;
  return result;
}

uint32_t composePixels(uint32_t *pixels, uint8_t pixelCount)
{
  // if pixelCount is 1
  if (pixelCount == 1)
  {
    // if it is opaque just return it
    if ((pixels[0] >> 24) == 0xFF)
    {
      return pixels[0];
    }
    // otherwise put solid white behind it
    else
    {
      ARGB color = composePair(parseARGB8888(pixels[0]), parseARGB8888(0xFFFFFFFF));
      return constructARGB8888(color);
    }
  }

  ARGB working = parseARGB8888(pixels[pixelCount - 1]);
  // set the alpha to 1
  working.a = 1;

  for (int i = pixelCount - 2; i >= 0; i--)
  {
    working = composePair(parseARGB8888(pixels[i]), working);
  }

  return constructARGB8888(working);
}

SpriteEngine::SpriteEngine()
{
  this->sprites = std::vector<SpriteEntry *>();
  this->next_z_index = 1;
}

SpriteEngine::~SpriteEngine()
{
  for (SpriteEntry *sprite : this->sprites)
  {
    delete sprite;
  }
}

SpriteEntry *SpriteEngine::addSprite(Sprite *sprite, uint16_t x, uint16_t y, uint16_t z_index)
{
  SpriteEntry *spriteEntry = new SpriteEntry();
  spriteEntry->sprite = sprite;
  spriteEntry->x = x;
  spriteEntry->y = y;
  spriteEntry->z_index = z_index;
  this->sprites.push_back(spriteEntry);
  this->reshuffleSprites();
  return spriteEntry;
}

SpriteEntry *SpriteEngine::addSprite(Sprite *sprite, uint16_t x, uint16_t y)
{
  return this->addSprite(sprite, x, y, this->next_z_index++);
}

void SpriteEngine::moveSprite(SpriteEntry *sprite, uint16_t x, uint16_t y, uint16_t z_index)
{
  // TODO: it may be nice to verify if the entry is in the list
  // It will be slow, but it will be nice to have

  sprite->x = x;
  sprite->y = y;
  sprite->z_index = z_index;
  this->reshuffleSprites();
}

void SpriteEngine::moveSprite(SpriteEntry *sprite, uint16_t x, uint16_t y)
{
  this->moveSprite(sprite, x, y, sprite->z_index);
}

void SpriteEngine::removeSprite(SpriteEntry *sprite)
{
  auto itr = std::find(this->sprites.begin(), this->sprites.end(), sprite);
  if (itr != this->sprites.end())
  {
    this->sprites.erase(itr);
  }

  delete sprite;
}

void SpriteEngine::renderSprites(uint32_t *pixels, uint16_t width, uint16_t height)
{

  uint32_t *pixelLayerBuffer = new uint32_t[10];
  for (int x = 0; x < width; x++)
  {
    for (int y = 0; y < height; y++)
    {
      uint16_t lastZ = 65535;
      int layerCount = 0;
      for (SpriteEntry *sprite : this->sprites)
      {
        // optimization -> This assumes that there are many sprites with the same z_index
        // that aren't overlapping.
        if (sprite->z_index >= lastZ || layerCount >= 10 || sprite->z_index == 0)
        {
          break;
        }

        if (sprite->x <= x && sprite->y <= y && sprite->x + sprite->sprite->width > x && sprite->y + sprite->sprite->height > y)
        {
          uint32_t pixel = sprite->sprite->pixels[(y - sprite->y) * sprite->sprite->width + (x - sprite->x)];
          pixelLayerBuffer[layerCount++] = pixel;
          // is this opaque?
          if ((pixel >> 24) == 0xFF)
          {
            break;
          }

          lastZ = sprite->z_index;
        }
      }

      if (layerCount > 0)
      {
        pixels[y * width + x] = composePixels(pixelLayerBuffer, layerCount);
      }
      else
      {
        pixels[y * width + x] = 0xFFFFFFFF;
      }
    }
  }

  delete[] pixelLayerBuffer;
}

void SpriteEngine::reshuffleSprites()
{
  std::sort(this->sprites.begin(), this->sprites.end(), [](SpriteEntry *a, SpriteEntry *b)
            { return a->z_index > b->z_index; });
}