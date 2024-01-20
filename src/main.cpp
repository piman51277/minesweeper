#include <SDL2/SDL.h>
#include <iostream>
#include "spritelib/sprites.h"

int main(int argc, char *argv[])
{

  SDL_Init(SDL_INIT_EVERYTHING);

  // 480x240 is the resolution of the VEX Brain
  // but we make it bigger so its more comfortable to debug
  SDL_Window *window = SDL_CreateWindow("Minesweeper", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 480 * 4, 240 * 4, SDL_WINDOW_SHOWN);
  SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
  SDL_RenderSetLogicalSize(renderer, 480, 240);

  // create a pixel array
  Uint32 *pixels = new Uint32[480 * 240];

  // set all pixels to white
  for (int i = 0; i < 480 * 240; i++)
  {
    pixels[i] = 0xFFFFFFFF;
  }

  SpriteEngine engine;

  Sprite tile1 = loadSprite("assets/compiled/tile1.sprite");
  Sprite tile2 = loadSprite("assets/compiled/tile2.sprite");
  Sprite tile3 = loadSprite("assets/compiled/tile3.sprite");
  Sprite tile4 = loadSprite("assets/compiled/tile4.sprite");
  Sprite tile5 = loadSprite("assets/compiled/tile5.sprite");
  Sprite tile6 = loadSprite("assets/compiled/tile6.sprite");
  Sprite tile7 = loadSprite("assets/compiled/tile7.sprite");
  Sprite tile8 = loadSprite("assets/compiled/tile8.sprite");
  Sprite flag = loadSprite("assets/compiled/flag.sprite");
  Sprite mine = loadSprite("assets/compiled/mine.sprite");
  Sprite unchecked = loadSprite("assets/compiled/unchecked.sprite");
  Sprite mine_exploded = loadSprite("assets/compiled/mine-exploded.sprite");

  // add a sprite to the engine
  engine.addSprite(&tile1, 0, 0, 1);
  engine.addSprite(&tile2, 16, 0, 1);
  engine.addSprite(&tile3, 32, 0, 1);
  engine.addSprite(&tile4, 48, 0, 1);
  engine.addSprite(&tile5, 64, 0, 1);
  engine.addSprite(&tile6, 80, 0, 1);
  engine.addSprite(&tile7, 96, 0, 1);
  engine.addSprite(&tile8, 112, 0, 1);
  engine.addSprite(&flag, 128, 0, 1);
  engine.addSprite(&mine, 144, 0, 1);
  engine.addSprite(&unchecked, 160, 0, 1);
  engine.addSprite(&mine_exploded, 176, 0, 1);
  while (true)
  {
    SDL_Event e;
    if (SDL_PollEvent(&e))
    {
      if (e.type == SDL_QUIT)
      {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        break;
      }
    }

    // create a texture from the pixel array
    engine.renderSprites(pixels, 480, 240);
    SDL_Texture *texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, 480, 240);
    SDL_UpdateTexture(texture, NULL, pixels, 480 * sizeof(Uint32));

    // render the texture to the screen
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);

    SDL_Delay(20);
  }

  return 0;
}
