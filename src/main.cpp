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

  // add a sprite to the engine
  SpriteEntry *redSq = engine.addSprite(loadSprite("assets/compiled/redsquare.bin"), 30, 10, 1);
  SpriteEntry *bluSq = engine.addSprite(loadSprite("assets/compiled/bluesquare.bin"), 160, 60, 2);
  SpriteEntry *greSq = engine.addSprite(loadSprite("assets/compiled/greensquare.bin"), 110, 60, 3);

  int pos = 10;
  bool toRight = true;

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

    if (toRight)
    {
      pos += 2;
      if (pos > 300)
      {
        toRight = false;
      }
    }
    else
    {
      pos -= 2;
      if (pos < 10)
      {
        toRight = true;
      }
    }

    engine.moveSprite(redSq, pos, 10);

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
