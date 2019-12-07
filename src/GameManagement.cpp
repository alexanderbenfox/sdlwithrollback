#include "GameManagement.h"
#include "Timer.h"
#include "Sprite.h"
#include "GameActor.h"

#include "Input.h"
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

Vector2<int> ResourceManager::GetTextureWidthAndHeight(const std::string& file)
{
  int width;
  int height;
  SDL_QueryTexture(GetTexture(file).Get(), nullptr, nullptr, &width, &height);
  return Vector2<int>(width, height);
}

void ResourceManager::RegisterBlitOp()
{
  _registeredSprites.push_back(BlitOperation());
  _registeredSprites.back().valid = false;
}

void ResourceManager::BlitSprites()
{
  auto blit = [](BlitOperation* operation)
  {
    if (!operation->valid) return;

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
    // deregister sprite from list
    //sprite.valid = false;
  }

  //reset available ops for next frame
  opIndex = 0;
}

GameManager::~GameManager() {}

void GameManager::Initialize()
{
  SDL_Init(SDL_INIT_EVERYTHING);
  TTF_Init();

  SDL_CreateWindowAndRenderer(ScreenWidth, ScreenHeight, 0, &_window, &_renderer);
  SDL_SetWindowTitle(_window, Title);
  
  //create game entities

  _gameEntities.push_back(std::make_shared<Entity>());
  auto camera = _gameEntities.back();
  camera->AddComponent<Camera>();
  camera->GetComponent<Camera>()->Init(ScreenWidth, ScreenHeight);
  _mainCamera = camera->GetComponent<Camera>();


  Vector2<int> textureSize = ResourceManager::Get().GetTextureWidthAndHeight("spritesheets\\ryu.png");

  _gameEntities.push_back(std::make_shared<Entity>());
  auto sprite = _gameEntities.back();
  sprite->AddComponent<Sprite>();
  sprite->GetComponent<Sprite>()->Init("spritesheets\\ryu.png");
  sprite->AddComponent<Physics>();
  sprite->AddComponent<GameActor>();
  sprite->AddComponent<RectCollider>();
  sprite->GetComponent<RectCollider>()->Init(Vector2<float>(0.0f, 0.0f), Vector2<float>(static_cast<float>(textureSize.x), static_cast<float>(textureSize.y)));
  sprite->GetComponent<RectCollider>()->SetStatic(false);
  _player = sprite->GetComponent<GameActor>();

  _gameEntities.push_back(std::make_shared<Entity>());
  auto staticBoy = _gameEntities.back();
  staticBoy->AddComponent<Sprite>();
  staticBoy->GetComponent<Sprite>()->Init("spritesheets\\ryu.png");
  staticBoy->AddComponent<RectCollider>();
  float startPosX = staticBoy->transform.position.x = 200.0f;
  float startPosY = staticBoy->transform.position.y = 200.0f;

  staticBoy->GetComponent<RectCollider>()->Init(Vector2<float>(startPosX, startPosY), Vector2<float>(startPosX + textureSize.x, startPosY + textureSize.y));
  staticBoy->GetComponent<RectCollider>()->SetStatic(true);


}

void GameManager::Destroy()
{
  SDL_DestroyRenderer(_renderer);
  SDL_DestroyWindow(_window);

  _renderer = nullptr;
  _window = nullptr;

  SDL_Quit();
  TTF_Quit();

  _player.reset();
  _mainCamera.reset();
  _gameEntities.clear();
}

void GameManager::BeginGameLoop()
{
  SDL_Event event;
  Timer clock;
  //start the timer
  clock.Start();

  //initialize the functions
  //FrameFunction input = std::bind(&GameManager::UpdateInput, this);
  UpdateFunction update = std::bind(&GameManager::Update, this, std::placeholders::_1);
  //FrameFunction draw = std::bind(&GameManager::Draw, this);

  for (;;)
  {
    //! Check for quit
    if (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        return;
      }
    }

    //! Do pre-input set up stuff
    for (auto sprite : ComponentManager<Sprite>::Get().All())
      sprite->OnFrameBegin();

    //! Collect inputs from controllers (this means AI controllers as well as Player controllers)
    UpdateInput(&event);

    //! Update all components
    clock.Update(update);

    //! Finally render the scene
    Draw();
  }
}

GameManager::GameManager() : _initialized(false)
{
  _playerInput = std::unique_ptr<IInputHandler>(new KeyboardInputHandler());
}

void GameManager::Update(float deltaTime)
{
  /*for (auto& entity : _gameEntities)
    entity.Update(deltaTime);

  for (auto& entity : _gameEntities)
    entity.PushToRenderer();*/

  //Update characters
  ComponentManager<GameActor>::Get().Update(deltaTime);
  //
  ComponentManager<Physics>::Get().Update(deltaTime);
  //
  ComponentManager<RectCollider>::Get().Update(deltaTime);
  //Update sprites last
  ComponentManager<Sprite>::Get().Update(deltaTime);

}

void GameManager::UpdateInput(SDL_Event* event)
{
  //update keys pressed here
  ICommand* command = _playerInput->HandleInput(event);

  //then process the keys pressed depending on the state
  if (command)
  {
    command->Execute(_player.get());
    command = nullptr;
    delete command;
  }
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
