#pragma once
#include "Core/ECS/ISystem.h"

#include "Components/Input.h"
#include "Components/UIComponents.h"
#include "Components/RenderComponent.h"

struct MenuState : public IComponent
{
  Vector2<int> currentFocus;
  bool selected = false;
  bool cancel = false;
  bool waitingOnInput = false;
  SDL_Event rawInput = SDL_Event();
};

struct MenuItem : public IComponent
{
  Vector2<int> location;
  std::function<void(SDL_Event)> callback;
  // triggers callback on any raw input press
  bool callbackOnPressDown = false;
};

class MenuInputSystem : public ISystem<GameInputComponent, MenuState>
{
public:
  static void DoTick(float dt)
  {
    PROFILE_FUNCTION();
    for (const EntityID& entity : Registered)
    {
      GameInputComponent& input = ComponentArray<GameInputComponent>::Get().GetComponent(entity);
      MenuState& menu = ComponentArray<MenuState>::Get().GetComponent(entity);

      if (!menu.waitingOnInput)
      {
        const InputBuffer& lastInput = input.GetInput();
        // get button from keydown event
        InputState pressed = lastInput.LatestPressed();

        if (HasState(pressed, InputState::UP))
        {
          menu.currentFocus.y--;
        }
        if (HasState(pressed, InputState::DOWN))
        {
          menu.currentFocus.y++;
        }
        if (HasState(pressed, InputState::LEFT))
        {
          menu.currentFocus.x--;
        }
        if (HasState(pressed, InputState::RIGHT))
        {
          menu.currentFocus.x++;
        }

        Uint32 type = input.GetRawInput().type;
        if(type == SDL_JOYBUTTONDOWN || type == SDL_CONTROLLERBUTTONDOWN || type == SDL_KEYDOWN)
          menu.selected = HasState(pressed, InputState::BTN1);
        menu.cancel = !menu.selected && HasState(pressed, InputState::BTN2);
      }
      else
      {
        menu.rawInput = input.GetRawInput();
      }
      
    }
  }
};

class UpdateMenuStateSystem : public IMultiSystem<SysComponents<MenuState>, SysComponents<MenuItem, RenderProperties>>
{
public:
  static void DoTick(float dt)
  {
    PROFILE_FUNCTION();
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
            if (item.callbackOnPressDown)
            {
              if (!menu.waitingOnInput)
              {
                menu.waitingOnInput = true;
              }   
              else
              {
                // one of these SDL_EventType triggers the callback, in this case
                if (menu.rawInput.type == SDL_JOYBUTTONDOWN || menu.rawInput.type == SDL_CONTROLLERBUTTONDOWN || menu.rawInput.type == SDL_KEYDOWN)
                {
                  item.callback(menu.rawInput);
                  menu.waitingOnInput = false;
                  menu.selected = false;
                }
              }
            }
            else
            {
              item.callback(menu.rawInput);
              menu.selected = false;
            }
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
