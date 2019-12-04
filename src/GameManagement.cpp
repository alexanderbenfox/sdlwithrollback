#include "GameManagement.h"
#include "Timer.h"
#include "Sprite.h"
#include <iostream>

const int ScreenWidth = 600;
const int ScreenHeight = 400;

const char* Title = "Game Title";

void ResourceManager::Initialize()
{
  char* basePath = SDL_GetBasePath();
  if (basePath)
    _resourcePath = std::string(basePath) + "..\\..\\..\\resources\\";
  else _resourcePath = "./";
}

Texture& ResourceManager::GetTexture(const std::string& file)
{
  if (_loadedTextures.find(file) == _loadedTextures.end())
  {
    _loadedTextures.insert(std::make_pair(file, Texture(_resourcePath + file)));
  }
  _loadedTextures[file].Load();
  return _loadedTextures[file];
}

ResourceManager::BlitOperation* ResourceManager::RegisterBlitOp()
{
  _registeredSprites.push_back(BlitOperation());
  return &_registeredSprites.back();
}

void ResourceManager::BlitSprites()
{
  auto blit = [](BlitOperation* operation)
  {
    GameManager::Get().GetMainCamera()->ConvScreenSpace(operation);
    if (GameManager::Get().GetMainCamera()->EntityInDisplay(operation))
    {
      int w, h;
      auto srcTexture = operation->_textureResource->Get();
      float rotation = 0;
      //if(destRect->x >= _camera.x && destRect->x <=)
      try
      {
        if (SDL_QueryTexture(operation->_textureResource->Get(), NULL, NULL, &w, &h) == 0)
          SDL_RenderCopyEx(GameManager::Get().GetRenderer(), srcTexture, 
            &operation->_textureRect, &operation->_displayRect, rotation, nullptr, operation->_flip);
      }
      catch (std::exception &e)
      {
        std::cout << "I guess this texture isn't valid??" << "\nCaught exception: " << e.what() << "\n";
      }
    }
  };

  for (auto& sprite : _registeredSprites)
  {
    blit(&sprite);
  }
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
  
  //create game entities

  _gameEntities.push_back(Entity());
  Entity& camera = _gameEntities.back();
  camera.AddComponent<Camera>();
  camera.GetComponent<Camera>()->Init(ScreenWidth, ScreenHeight);
  _mainCamera = camera.GetComponent<Camera>();

  _gameEntities.push_back(Entity());
  Entity& sprite = _gameEntities.back();
  sprite.AddComponent<Sprite>();
  sprite.GetComponent<Sprite>()->Init("spritesheets\\idle.png");


}

void GameManager::Destroy()
{
  SDL_DestroyRenderer(_renderer);
  SDL_DestroyWindow(_window);

  _renderer = nullptr;
  _window = nullptr;

  SDL_Quit();
}

void GameManager::BeginGameLoop()
{
  SDL_Event event;
  Timer clock;
  //start the timer
  clock.Start();

  //initialize the functions
  FrameFunction input = std::bind(&GameManager::UpdateInput, this);
  UpdateFunction update = std::bind(&GameManager::Update, this, std::placeholders::_1);
  FrameFunction draw = std::bind(&GameManager::Draw, this);

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

void GameManager::Update(float deltaTime)
{
  for (auto& entity : _gameEntities)
    entity.Update(deltaTime);

  for (auto& entity : _gameEntities)
    entity.PushToRenderer();
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
  ResourceManager::Get().BlitSprites();

  //present this frame
  SDL_RenderPresent(_renderer);
}
