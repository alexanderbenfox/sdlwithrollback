#pragma once
#include <SDL2/SDL.h>

#include "Utils.h"
#include "Components/IComponent.h"
#include "DebugGUI/DebugItem.h"

#include <queue>
#include <unordered_map>

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

template <> struct std::hash<InputState>
{
  std::size_t operator()(const InputState& k) const
  {
    return hash<unsigned char>()((unsigned char)k);
  }
};

//______________________________________________________________________________
enum class SpecialMoveState : unsigned char
{
  NONE = 0x00,
  QCF = 0x01, // quarter circle forward
  QCB = 0x02, // quarter circle back
  DPF = 0x04, // dragon punch forward
  DPB = 0x08 // dragon punch back
};

//______________________________________________________________________________
void operator|=(InputState& the, InputState other);
//______________________________________________________________________________
void operator&=(InputState& the, InputState other);
//______________________________________________________________________________
InputState operator&(InputState a, InputState b);
//______________________________________________________________________________
InputState operator|(InputState a, InputState b);
//______________________________________________________________________________
InputState operator~(InputState const& other);

//______________________________________________________________________________
static bool HasState(const InputState& state, InputState other) { return (state & other) == other; }


//! Input buffer class for storing 
class InputBuffer
{
public:
  //! construct with a limit to the size of the buffer
  InputBuffer(int limit);
  //! push new input state into the buffer and remove oldest state from buffer
  void Push(InputState item);
  //! gets the most recently added item
  InputState const& Latest() const { return _buffer.back(); }
  //! evaluate possible special motions
  SpecialMoveState Evaluate(const TrieNode<InputState, SpecialMoveState>& spMoveDict) const;
  //!
  void Clear();

private:
  std::vector<InputState> _buffer;
  int _limit;

};

// simple move dict to test this out
const TrieNode<InputState, SpecialMoveState> UnivSpecMoveDict
{
  std::make_pair(std::list<InputState>{InputState::DOWN, InputState::DOWN | InputState::RIGHT, InputState::RIGHT}, SpecialMoveState::QCF),
  std::make_pair(std::list<InputState>{InputState::DOWN, InputState::DOWN | InputState::LEFT, InputState::LEFT}, SpecialMoveState::QCB)
};



//______________________________________________________________________________
//! Interface for input handlers
class IInputHandler
{
public:
  // initialize with input buffer of 6 frames
  IInputHandler() : _inputBuffer(10) {}
  //! Destructor
  virtual ~IInputHandler() {}
  //! Gets the command based on the type of input received from the controller
  virtual InputBuffer const& CollectInputState() = 0;
  //!
  virtual void ClearInputBuffer() { _inputBuffer.Clear(); }



protected:
  //! Last state received by the input controller
  InputBuffer _inputBuffer;

};

//______________________________________________________________________________
//! Keyboard handler specification
class KeyboardInputHandler : public IInputHandler
{
public:
  //!
  KeyboardInputHandler();
  //!
  ~KeyboardInputHandler();
  //!
  virtual InputBuffer const& CollectInputState() final;
  //!
  void AssignKey(SDL_Keycode keyCode, InputState action)
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
  JoystickInputHandler();
  //!
  ~JoystickInputHandler();
  //!
  virtual InputBuffer const& CollectInputState() final;

  void AssignKey(uint8_t keyCode, InputState action)
  {
    _config[keyCode] = action;
  }

private:
  //!
  SDL_Joystick* _gameController = nullptr;
  //!
  const int _joyStickID = 0;
  //!
  ConfigMap<uint8_t, InputState> _config;
  
};


//______________________________________________________________________________
//!
class GamepadInputHandler : public IInputHandler
{
public:
  //!
  GamepadInputHandler();
  //!
  ~GamepadInputHandler();
  //!
  virtual InputBuffer const& CollectInputState() final;

  void AssignKey(SDL_GameControllerButton keyCode, InputState action)
  {
    _config[keyCode] = action;
  }


private:
  //!
  SDL_GameController* _gameController = nullptr;
  //!
  SDL_Joystick* _joyStick;
  //!
  const int _joyStickID = 0;
  //!
  ConfigMap<SDL_GameControllerButton, InputState> _config;
  
};

enum class InputType : int
{
  Keyboard = 0, Joystick = 1, Gamepad = 2
};

//______________________________________________________________________________
//!
class GameInputComponent : public IComponent, public DebugItem
{
public:
  GameInputComponent(std::shared_ptr<Entity> owner) : IComponent(owner), DebugItem("Input Handler") {}
  //! Assign this component to use a certain handler type
  void AssignHandler(InputType type);
  //! Handler interprets latest raw input and returns it
  InputBuffer const& QueryInput();
  //! Clears the input buffer
  void Clear();
  //! Assigns handler key to an action
  template <typename KeyType>
  void AssignActionKey(KeyType key, InputState action) {}
  //! Allows switching of input mode and key assignments
  void OnDebug() override;

protected:
  IInputHandler* _handler = nullptr;
  InputType _assignedHandler = InputType::Keyboard;

  KeyboardInputHandler _keyboard;
  JoystickInputHandler _joystick;
  GamepadInputHandler _gamepad;

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
  GGPOInputHandler() : IInputHandler() {}
  //!
  ~GGPOInputHandler() {}
  //!
  virtual InputBuffer const& CollectInputState() final;

private:
  std::shared_ptr<GGPOInput> _input;
};

#endif
