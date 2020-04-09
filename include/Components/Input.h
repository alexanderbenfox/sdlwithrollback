#pragma once
#include <SDL2/SDL.h>
#include <unordered_map>
#include "Utils.h"
#include "Components/IComponent.h"

//______________________________________________________________________________
enum class InputState : unsigned char
{
  NONE = 0x00,
  UP = 0x01,
  DOWN = 0x02,
  RIGHT = 0x04,
  LEFT = 0x08,
  BTN1 = 0x10,
  BTN2 = 0x20,
  BTN3 = 0x40,
  BTN4 = 0x80
};

//______________________________________________________________________________
void operator|=(InputState& the, InputState other);
//______________________________________________________________________________
void operator&=(InputState& the, InputState other);
//______________________________________________________________________________
InputState operator&(InputState a, InputState b);
//______________________________________________________________________________
InputState operator~(InputState const& other);
//______________________________________________________________________________
static bool HasState(const InputState& state, InputState other) { return (state & other) == other; }

//______________________________________________________________________________
//! Interface for input handlers
//template <class InputSource>
class IInputHandler : public IComponent
{
public:
  IInputHandler(std::shared_ptr<Entity> owner) :
    _lastFrameState(InputState::NONE), IComponent(owner) {}
  //! Destructor
  virtual ~IInputHandler() {}
  //! Gets the command based on the type of input received from the controller
  virtual InputState CollectInputState() = 0;

protected:
  //! Last state received by the input controller
  InputState _lastFrameState;

};

//______________________________________________________________________________
//! Keyboard handler specification
class KeyboardInputHandler : public IInputHandler
{
public:
  //!
  KeyboardInputHandler(std::shared_ptr<Entity> owner);
  //!
  ~KeyboardInputHandler();
  //!
  virtual InputState CollectInputState() final;
  //!
  virtual void SetKey(SDL_Keycode keyCode, InputState action)
  {
    _config[keyCode] = action;
  }

private:
  //!
  const uint8_t* _keyStates = nullptr;
  //!
  ConfigMap<SDL_Keycode, InputState> _config;

};

//______________________________________________________________________________
//!
class JoystickInputHandler : public IInputHandler
{
public:
  //!
  JoystickInputHandler(std::shared_ptr<Entity> owner);
  //!
  ~JoystickInputHandler();
  //!
  virtual InputState CollectInputState() final;

private:
  //!
  SDL_Joystick* _gameController = nullptr;
  //!
  const int _joyStickID = 0;
  //!
  ConfigMap<uint8_t, InputState> _config;
  
};

#ifdef _WIN32
#include <GGPO/ggponet.h>

struct GGPOInput
{
  GGPOSession* session;
  InputState** inputs;
  GGPOPlayerHandle** handles;
};

//______________________________________________________________________________
//!
class GGPOInputHandler : public IInputHandler
{
public:
  //!
  GGPOInputHandler(std::shared_ptr<Entity> owner) : IInputHandler(owner) {}
  //!
  ~GGPOInputHandler() {}
  //!
  virtual InputState CollectInputState() final;

private:
  std::shared_ptr<GGPOInput> _input;
};

#endif
