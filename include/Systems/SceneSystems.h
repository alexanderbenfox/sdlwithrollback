#pragma once
#include "Systems/ISystem.h"
#include "Components/Input.h"

class StartSceneInputSystem : public ISystem<GameInputComponent>
{
public:
  static void DoTick(float dt)
  {
    for(auto& tuple : Tuples)
    {
      GameInputComponent* inputHandler = std::get<GameInputComponent*>(tuple.second);
      const InputBuffer& inputState = inputHandler->QueryInput();

      if(inputState.Latest() != InputState::NONE)
      {
        // now we change to mode select
        GameManager::Get().RequestSceneChange(SceneType::CSELECT);
      }
    }
  }
};

class CharacterSelectInputSystem : public ISystem<GameInputComponent>
{
public:
  static void DoTick(float dt)
  {
    for(auto& tuple : Tuples)
    {
      GameInputComponent* inputHandler = std::get<GameInputComponent*>(tuple.second);
      const InputBuffer& inputState = inputHandler->QueryInput();

      if(inputState.Latest() != InputState::NONE)
      {
        GameManager::Get().RequestSceneChange(SceneType::BATTLE);
      }
    }
  }
};

class ResultsSceneSystem : public ISystem<GameInputComponent>
{
public:
  static void DoTick(float dt)
  {
    for(auto& tuple : Tuples)
    {
      GameInputComponent* inputHandler = std::get<GameInputComponent*>(tuple.second);
      const InputBuffer& inputState = inputHandler->QueryInput();

      if(inputState.Latest() != InputState::NONE)
      {
        GameManager::Get().RequestSceneChange(SceneType::START);
      }
    }
  }
};
