#pragma once
#include "Systems/ISystem.h"

#include "Components/Input.h"
#include "Components/UIComponents.h"
#include "Components/RenderComponent.h"

struct MenuState : public IComponent
{
  MenuState(std::shared_ptr<Entity> e) : IComponent(e) {}
  Vector2<int> currentFocus;
  bool selected = false;
  bool cancel = false;
};

struct MenuItem : public IComponent
{
  MenuItem(std::shared_ptr<Entity> e) : IComponent(e) {}
  Vector2<int> location;
  std::function<void()> callback;
};

class MenuInputSystem : public ISystem<GameInputComponent, MenuState>
{
public:
  static void DoTick(float dt)
  {
    for (auto tuple : Tuples)
    {
      GameInputComponent* input = std::get<GameInputComponent*>(tuple.second);
      MenuState* menu = std::get<MenuState*>(tuple.second);

      const InputBuffer& lastInput = input->QueryInput();
      // get button from keydown event
      InputState pressed = lastInput.LatestPressed();

      if(HasState(pressed, InputState::UP))
      {
        menu->currentFocus.y--;
      }
      if(HasState(pressed, InputState::DOWN))
      {
        menu->currentFocus.y++;
      }
      if(HasState(pressed, InputState::LEFT))
      {
        menu->currentFocus.x--;
      }
      if(HasState(pressed, InputState::RIGHT))
      {
        menu->currentFocus.x++;
      }

      menu->selected = HasState(pressed, InputState::BTN1);
      menu->cancel = !menu->selected && HasState(pressed, InputState::BTN2);
    }
  }
};

class UpdateMenuStateSystem : public IMultiSystem<SysComponents<MenuState>, SysComponents<MenuItem, RenderProperties>>
{
public:
  static void DoTick(float dt)
  {
    for (auto tuple : MainSystem::Tuples)
    {
      MenuState* menu = std::get<MenuState*>(tuple.second);

      for(auto subTuple : SubSystem::Tuples)
      {
        MenuItem* item = std::get<MenuItem*>(subTuple.second);
        RenderProperties* props = std::get<RenderProperties*>(subTuple.second);

        // if the state is focused on this position, highlight the box
        if(menu->currentFocus == item->location)
        {
          props->SetDisplayColor(255, 255, 255, 255);

          if(menu->selected)
          {
            item->callback();
          }
        }
        else
        {
          props->SetDisplayColor(128, 128, 128, 128);
        }
      }
    }
  }
};
