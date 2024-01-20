#include <SDL2/SDL.h>
#include <iostream>
#include "minesweeper/game.h"

int main(int argc, char *argv[])
{

  SDL_Init(SDL_INIT_EVERYTHING);

  // 480x240 is the resolution of the VEX Brain
  // but we make it bigger so its more comfortable to debug
  SDL_Window *window = SDL_CreateWindow("Minesweeper", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 480 * 4, 240 * 4, SDL_WINDOW_SHOWN);
  SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
  SDL_RenderSetLogicalSize(renderer, 480, 240);

  MinesweeperGame game;
  game.init();

  uint32_t *pixels = new uint32_t[480 * 240];

  while (true)
  {
    SDL_Event e;
    while (SDL_PollEvent(&e))
    {
      if (e.type == SDL_QUIT)
      {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        delete[] pixels;
        return 0;
        break;
      }

      // keypresses
      if (e.type == SDL_KEYDOWN)
      {
        switch (e.key.keysym.sym)
        {
        case SDLK_UP:
          game.moveCursor(0, -1);
          break;
        case SDLK_DOWN:
          game.moveCursor(0, 1);
          break;
        case SDLK_LEFT:
          game.moveCursor(-1, 0);
          break;
        case SDLK_RIGHT:
          game.moveCursor(1, 0);
          break;
        case SDLK_SPACE:
          game.reveal();
          break;
        case SDLK_f:
          game.flag();
          break;
        case SDLK_r:
          game.reset();
          break;
        }
      }
    }

    // render the game
    game.render(pixels, 480, 240);

    // create a texture from the pixel array
    SDL_Texture *texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, 480, 240);
    SDL_UpdateTexture(texture, NULL, pixels, 480 * sizeof(Uint32));

    // render the texture to the screen
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);

    SDL_Delay(33);
  }

  return 0;
}
