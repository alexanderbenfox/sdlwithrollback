#include "GameManagement.h"
#include "Timer.h"

#include "Input.h"
#include <iostream>

#include "ResourceManager.h"

#include "Components/Camera.h"
#include "Components/Sprite.h"
#include "Components/GameActor.h"
#include "Components/Physics.h"
#include "Components/Collider.h"

const int ScreenWidth = 600;
const int ScreenHeight = 400;

const char* Title = "Game Title";

//______________________________________________________________________________
void ResourceManager::Initialize()
{
  char* basePath = SDL_GetBasePath();
  if (basePath)
    _resourcePath = std::string(basePath) + "..\\..\\..\\resources\\";
  else _resourcePath = "./";
}

//______________________________________________________________________________
Texture& ResourceManager::GetTexture(const std::string& file)
{
  if (_loadedTextures.find(file) == _loadedTextures.end())
  {
    _loadedTextures.insert(std::make_pair(file, Texture(_resourcePath + file)));
  }
  _loadedTextures[file].Load();
  return _loadedTextures[file];
}

//______________________________________________________________________________
Vector2<int> ResourceManager::GetTextureWidthAndHeight(const std::string& file)
{
  int width;
  int height;
  SDL_QueryTexture(GetTexture(file).Get(), nullptr, nullptr, &width, &height);
  return Vector2<int>(width, height);
}

//______________________________________________________________________________
void ResourceManager::RegisterBlitOp()
{
  _registeredSprites.push_back(BlitOperation());
  _registeredSprites.back().valid = false;
}

//______________________________________________________________________________
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

//______________________________________________________________________________
void GameManager::Initialize()
{
  SDL_Init(SDL_INIT_EVERYTHING);
  TTF_Init();

  SDL_CreateWindowAndRenderer(ScreenWidth, ScreenHeight, 0, &_window, &_renderer);
  SDL_SetWindowTitle(_window, Title);
  
  //create game entities

  auto camera = CreateEntity<Camera>();
  camera->GetComponent<Camera>()->Init(ScreenWidth, ScreenHeight);
  _mainCamera = camera->GetComponent<Camera>();

  Vector2<int> textureSize = ResourceManager::Get().GetTextureWidthAndHeight("spritesheets\\ryu.png");

  auto sprite = CreateEntity<Animator, Physics, GameActor, RectColliderD>();

  sprite->GetComponent<Animator>()->Init();
  sprite->GetComponent<Animator>()->RegisterAnimation("Idle", "spritesheets\\idle_and_walking.png", 6, 6, 0, 10);
  sprite->GetComponent<Animator>()->RegisterAnimation("WalkF", "spritesheets\\idle_and_walking.png", 6, 6, 10, 12);
  sprite->GetComponent<Animator>()->RegisterAnimation("WalkB", "spritesheets\\idle_and_walking.png", 6, 6, 22, 11);

  sprite->GetComponent<Animator>()->RegisterAnimation("Jumping", "spritesheets\\idle_walking_jab_jump_crouch.png", 8, 10, 41, 19);
  sprite->GetComponent<Animator>()->RegisterAnimation("Falling", "spritesheets\\idle_walking_jab_jump_crouch.png", 8, 10, 60, 13);

  sprite->GetComponent<Animator>()->Play("Idle", true);

  sprite->GetComponent<RectColliderD>()->Init(Vector2<double>(0.0, 0.0),
    Vector2<double>(static_cast<double>(textureSize.x), static_cast<double>(textureSize.y)));
  sprite->GetComponent<RectColliderD>()->SetStatic(false);
  _player = sprite->GetComponent<GameActor>();
  
  /*auto staticBoy = CreateEntity<Animator, RectColliderD>();
  float startPosX = staticBoy->transform.position.x = 200.0f;
  float startPosY = staticBoy->transform.position.y = 200.0f;

  staticBoy->GetComponent<Animator>()->Init();
  staticBoy->GetComponent<Animator>()->RegisterAnimation("Idle", "spritesheets\\idle.png", 3, 4, 0, 10);
  staticBoy->GetComponent<Animator>()->Play("Idle", true);
  staticBoy->GetComponent<RectColliderD>()->Init(Vector2<double>(startPosX, startPosY), Vector2<double>(startPosX + textureSize.x, startPosY + textureSize.y));
  staticBoy->GetComponent<RectColliderD>()->SetStatic(true);*/

  auto bottomBorder = CreateEntity<Sprite, RectColliderD>();
  bottomBorder->transform.position.x = 0.0;
  bottomBorder->transform.position.y = ScreenHeight;
  bottomBorder->GetComponent<Sprite>()->Init("spritesheets\\ryu.png");
  bottomBorder->GetComponent<RectColliderD>()->Init(Vector2<double>(0, ScreenHeight), Vector2<double>(ScreenWidth, ScreenHeight + 50.0f));
  bottomBorder->GetComponent<RectColliderD>()->SetStatic(true);


}

//______________________________________________________________________________
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

//______________________________________________________________________________
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
    for (auto animator : ComponentManager<Animator>::Get().All())
      animator->OnFrameBegin();

    //! Collect inputs from controllers (this means AI controllers as well as Player controllers)
    UpdateInput(&event);

    //! Update all components
    clock.Update(update);

    //! Finally render the scene
    Draw();
  }
}

//______________________________________________________________________________
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
  ComponentManager<RectColliderD>::Get().Update(deltaTime);
  //Update sprites last
  ComponentManager<Sprite>::Get().Update(deltaTime);
  //
  ComponentManager<Animator>::Get().Update(deltaTime);

}

//______________________________________________________________________________
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

//______________________________________________________________________________
void GameManager::Draw()
{
  //clear last frame graphics
  SDL_RenderClear(_renderer);

  // resource manager draw here?
  ResourceManager::Get().BlitSprites();

  //present this frame
  SDL_RenderPresent(_renderer);
}

//______________________________________________________________________________
GameManager::GameManager() : _initialized(false)
{
  _playerInput = std::unique_ptr<IInputHandler>(new KeyboardInputHandler());
}
