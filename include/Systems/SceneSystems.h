#pragma once
#include "Systems/ISystem.h"
#include "Components/Input.h"

class StartSceneInputSystem : public ISystem<KeyboardInputHandler>
{
public:
  static void DoTick(float dt)
  {
    for(auto& tuple : Tuples)
    {
      KeyboardInputHandler* inputHandler = std::get<KeyboardInputHandler*>(tuple.second);
      const InputBuffer& inputState = inputHandler->CollectInputState();

      if(inputState.Latest() != InputState::NONE)
      {
        // now we change to mode select
        GameManager::Get().RequestSceneChange(SceneType::CSELECT);
      }
    }
  }
};

class CharacterSelectInputSystem : public ISystem<KeyboardInputHandler>
{
public:
  static void DoTick(float dt)
  {
    for(auto& tuple : Tuples)
    {
      KeyboardInputHandler* inputHandler = std::get<KeyboardInputHandler*>(tuple.second);
      const InputBuffer& inputState = inputHandler->CollectInputState();

      if(inputState.Latest() != InputState::NONE)
      {
        GameManager::Get().RequestSceneChange(SceneType::BATTLE);
      }
    }
  }
};

class ResultsSceneSystem : public ISystem<KeyboardInputHandler>
{
public:
  static void DoTick(float dt)
  {
    for(auto& tuple : Tuples)
    {
      KeyboardInputHandler* inputHandler = std::get<KeyboardInputHandler*>(tuple.second);
      const InputBuffer& inputState = inputHandler->CollectInputState();

      if(inputState.Latest() != InputState::NONE)
      {
        GameManager::Get().RequestSceneChange(SceneType::START);
      }
    }
  }
};
