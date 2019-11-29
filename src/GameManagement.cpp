#include "../include/GameManagement.h"

const int ScreenWidth = 600;
const int ScreenHeight = 400;

const char* Title = "Game Title";

void ResourceManager::Initialize()
{
  char* basePath = SDL_GetBasePath();
  if (basePath)
    _dataPath = std::string(basePath);
  else _dataPath = "./";
}

void ResourceManager::LoadFile(const std::string& file)
{

}

GameManager::~GameManager()
{
  TTF_Quit();
}

void GameManager::Initialize()
{
  SDL_Init(SDL_INIT_EVERYTHING);
  TTF_Init();

  SDL_CreateWindowAndRenderer(ScreenWidth, ScreenHeight, 0, &_window, &_renderer);
  SDL_SetWindowTitle(_window, Title);
}

void GameManager::Destroy()
{
  SDL_DestroyWindow(_window);
  SDL_DestroyRenderer(_renderer);
}

void GameManager::BeginGameLoop()
{
  SDL_Event event;
  int lastUpdateTimeMS = SDL_GetTicks();

  for (;;)
  {
    //get new key states
    if (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        return;
      }
    }

    const int currTime = SDL_GetTicks();
    int elapsedTimeMS = lastUpdateTimeMS - SDL_GetTicks();
    // do update logic here
    Update(elapsedTimeMS);
    //
    lastUpdateTimeMS = currTime;
    // do draw logic here (after all logic is complete)
    Draw();
  }
}

void GameManager::Update(int deltaTime_ms)
{

}

void GameManager::Draw()
{
  //clear last frame graphics
  SDL_RenderClear(_renderer);

  // resource manager draw here?

  //present this frame
  SDL_RenderPresent(_renderer);
}