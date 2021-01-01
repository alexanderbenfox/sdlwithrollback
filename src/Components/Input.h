#pragma once
#include <SDL2/SDL.h>

#include "Components/DebugComponent/DebugComponent.h"
#include "Components/InputHandlers/KeyboardHandler.h"
#include "Components/InputHandlers/JoystickInputHandler.h"
#include "Components/InputHandlers/GamepadInputHandler.h"
#include "Components/InputHandlers/AIInputHandler.h"

enum class InputType : int
{
  Keyboard = 0, Joystick = 1, Gamepad = 2, DefendAll = 3, DefendAfter = 4, RepeatCM = 5, NetworkCtrl = 6
};

//______________________________________________________________________________
//!
class GameInputComponent : public IDebugComponent, ISerializable
{
public:
  GameInputComponent() : _input(20), _assignedHandler(InputType::Keyboard), _handler(new KeyboardInputHandler(_input)), IDebugComponent("Input Handler") {}
  ~GameInputComponent()
  {
    if (_handler)
    {
      delete _handler;
      _handler = nullptr;
    }
  }

  void OnRemove(const EntityID& entity) override
  {
    IDebugComponent::OnRemove(entity);
    if (_handler)
    {
      delete _handler;
      _handler = nullptr;
    }
  }

  GameInputComponent& operator=(const GameInputComponent& other)
  {
    AssignHandler(other.GetAssignedHandler());
    return *this;
  }
  //! Assign this component to use a certain handler type
  void AssignHandler(InputType type);
  //! Handler interprets latest raw input and returns it
  //InputBuffer& QueryInput(const SDL_Event& local);
  InputState TranslateEvent(const SDL_Event& local) { return _handler->TranslateEvent(local); }
  //!
  void PushState(const InputState state) { _handler->CommitInput(state); }
  //! Gets input after it has already been interpretted by QueryInput (and GGPO, if applicable)
  InputBuffer const& GetInput() { return _handler->GetInterprettedInput(); }
  //! Sync last input from synced ggpo session
  void Sync(InputState syncState) { _handler->SyncLastInput(syncState); }
  //! Clears the input buffer
  void Clear();
  //! Assigns handler key to an action
  template <typename KeyType>
  void AssignActionKey(KeyType key, InputState action) {}
  //! Allows switching of input mode and key assignments
  void OnDebug() override;

  InputType GetAssignedHandler() const { return _assignedHandler; }

  //! Override ISerializable functions (only serialize buffer state)
  void Serialize(std::ostream& os) const override { _input.Serialize(os); }
  void Deserialize(std::istream& is) override { _input.Deserialize(is); }
  std::string Log() override { return _input.Log(); }

protected:
  IInputHandler* _handler;
  InputType _assignedHandler;
  InputBuffer _input;

};
