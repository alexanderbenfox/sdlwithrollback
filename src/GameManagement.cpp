#include "../include/GameManagement.h"
#include "../include/Timer.h"

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
  Timer clock;
  //start the timer
  clock.Start();

  //initialize the functions
  auto input = std::bind(&GameManager::UpdateInput, this);
  auto update = std::bind(&GameManager::Update, this, std::placeholders::_1);
  auto draw = std::bind(&GameManager::Draw, this);

  for (;;)
  {
    //get new key states
    if (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        return;
      }
    }
    clock.Update(input, update, draw);
  }
}

void GameManager::Update(int deltaTime_ms)
{
}

void GameManager::UpdateInput()
{
  //update keys pressed here

  //then process the keys pressed depending on the state
}

void GameManager::Draw()
{
  //clear last frame graphics
  SDL_RenderClear(_renderer);

  // resource manager draw here?

  //present this frame
  SDL_RenderPresent(_renderer);
}