#pragma once
#include "Systems/ISystem.h"

#include "Components/Input.h"
#include "Components/UIComponents.h"
#include "Components/RenderComponent.h"

struct MenuState : public IComponent
{
  Vector2<int> currentFocus;
  bool selected = false;
  bool cancel = false;
};

struct MenuItem : public IComponent
{
  Vector2<int> location;
  std::function<void()> callback;
};

class MenuInputSystem : public ISystem<GameInputComponent, MenuState>
{
public:
  static void DoTick(float dt)
  {
    for (const EntityID& entity : Registered)
    {
      GameInputComponent& input = ComponentArray<GameInputComponent>::Get().GetComponent(entity);
      MenuState& menu = ComponentArray<MenuState>::Get().GetComponent(entity);

      const InputBuffer& lastInput = input.QueryInput();
      // get button from keydown event
      InputState pressed = lastInput.LatestPressed();

      if(HasState(pressed, InputState::UP))
      {
        menu.currentFocus.y--;
      }
      if(HasState(pressed, InputState::DOWN))
      {
        menu.currentFocus.y++;
      }
      if(HasState(pressed, InputState::LEFT))
      {
        menu.currentFocus.x--;
      }
      if(HasState(pressed, InputState::RIGHT))
      {
        menu.currentFocus.x++;
      }

      menu.selected = HasState(pressed, InputState::BTN1);
      menu.cancel = !menu.selected && HasState(pressed, InputState::BTN2);
    }
  }
};

class UpdateMenuStateSystem : public IMultiSystem<SysComponents<MenuState>, SysComponents<MenuItem, RenderProperties>>
{
public:
  static void DoTick(float dt)
  {
    for (const EntityID& e1 : MainSystem::Registered)
    {
      MenuState& menu = ComponentArray<MenuState>::Get().GetComponent(e1);

      for(const EntityID& e2 : SubSystem::Registered)
      {
        MenuItem& item = ComponentArray<MenuItem>::Get().GetComponent(e2);
        RenderProperties& props = ComponentArray<RenderProperties>::Get().GetComponent(e2);

        // if the state is focused on this position, highlight the box
        if(menu.currentFocus == item.location)
        {
          props.SetDisplayColor(255, 255, 255, 255);

          if(menu.selected)
          {
            item.callback();
          }
        }
        else
        {
          props.SetDisplayColor(128, 128, 128, 128);
        }
      }
    }
  }
};
