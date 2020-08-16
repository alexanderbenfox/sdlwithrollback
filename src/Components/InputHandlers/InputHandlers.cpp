#include "Components/InputHandlers/AIInputHandler.h"
#include "Components/InputHandlers/KeyboardHandler.h"
#include "Components/InputHandlers/JoystickInputHandler.h"
#include "Components/InputHandlers/GamepadInputHandler.h"

#include "Managers/GameManagement.h"

#include <deque>
#include <SDL2/SDL_events.h>

//Analog joystick dead zone
const int JOYSTICK_DEAD_ZONE = 8000;

//______________________________________________________________________________
InputBuffer::InputBuffer(int limit) : _limit(limit), _spMovesBuffer(limit, UnivSpecMoveDict)
{
  // construct by initializing the queue 
  while (_buffer.size() < _limit)
    _buffer.push_back(InputState::NONE);
}

//______________________________________________________________________________
void InputBuffer::Push(InputState item)
{
  _buffer.push_back(item);
  // pop
  _buffer.erase(_buffer.begin());

  _spMovesBuffer.PushInput(item);
}

//______________________________________________________________________________
InputState InputBuffer::LatestPressed() const
{
  InputState latest = _buffer[_limit - 1];
  
  if(HasState(_buffer[_limit - 2], latest))
    return InputState::NONE;
  return latest;
}

//______________________________________________________________________________
void InputBuffer::Clear()
{
  for(int i = 0; i < _limit; i++)
    _buffer[i] = InputState::NONE;
  _spMovesBuffer.Clear();
}

//______________________________________________________________________________
InputBuffer const& AIInputHandler::CollectInputState()
{
  if(_ai)
    _inputBuffer.Push(_ai->lastDecision);
  return _inputBuffer;
}

//______________________________________________________________________________
void AIInputHandler::SetAIProgram(AIComponent* comp, IAIProgram* program)
{
  _ai = comp;
  if(_ai)
    _ai->program = std::unique_ptr<IAIProgram>(program);
}

//______________________________________________________________________________
KeyboardInputHandler::KeyboardInputHandler() : IInputHandler()
{
  //assign direction buttons
  _config[SDLK_w] = InputState::UP;
  _config[SDLK_a] = InputState::LEFT;
  _config[SDLK_s] = InputState::DOWN;
  _config[SDLK_d] = InputState::RIGHT;

  //assign the button events
  _config[SDLK_u] = InputState::BTN1;
  _config[SDLK_i] = InputState::BTN2;
  _config[SDLK_o] = InputState::BTN3;
  _config[SDLK_p] = InputState::BTN4;
}

//______________________________________________________________________________
KeyboardInputHandler::~KeyboardInputHandler() {}

//______________________________________________________________________________
InputBuffer const& KeyboardInputHandler::CollectInputState()
{
  InputState frameState = _inputBuffer.Latest();

  _keyStates = SDL_GetKeyboardState(0);

  const SDL_Event& input = GameManager::Get().GetLocalInput();

  SDL_Keycode key = input.key.keysym.sym;
  switch (input.type)
  {
  case SDL_KEYDOWN:
    frameState |= _config[key];
    break;
  case SDL_KEYUP:
    frameState &= (~(InputState)_config[key]);
    break;
  }

  _inputBuffer.Push(frameState);
  return _inputBuffer;
}

//______________________________________________________________________________
JoystickInputHandler::JoystickInputHandler() : IInputHandler()
{
  if(SDL_NumJoysticks() < 1) {}
  else
  {
    SDL_JoystickEventState(SDL_ENABLE);
    _gameController = SDL_JoystickOpen(0);
  }

  _config[0] = InputState::BTN1;
  _config[1] = InputState::BTN2;
  _config[2] = InputState::BTN3;
  _config[3] = InputState::BTN4;
}

JoystickInputHandler::~JoystickInputHandler()
{
  //close game controller if it exists
  if(_gameController)
  {
    SDL_JoystickClose(_gameController);
    _gameController = nullptr;
  }
}

//______________________________________________________________________________
InputBuffer const& JoystickInputHandler::CollectInputState()
{
  InputState frameState = _inputBuffer.Latest();

  //reset movement state
  frameState &= ~((InputState)(0xf0));

  const SDL_Event& input = GameManager::Get().GetLocalInput();

  switch (input.type)
  {
  case SDL_JOYAXISMOTION:
    //action on axis of this controller
    if(input.jaxis.which == 0)
    {
      //movement on x axis
      if(input.jaxis.axis == 0)
      {
        if(input.jaxis.value < -JOYSTICK_DEAD_ZONE)
        {
          frameState |= InputState::RIGHT;
        }
        else if (input.jaxis.value > JOYSTICK_DEAD_ZONE)
        {
          frameState |= InputState::LEFT;
        }
      }
      if (input.jaxis.axis == 1)
      {
        if(input.jaxis.value < -JOYSTICK_DEAD_ZONE)
        {
          frameState |= InputState::DOWN;
        }
        else if (input.jaxis.value > JOYSTICK_DEAD_ZONE)
        {
          frameState |= InputState::UP;
        }
      }
    }
    break;
  case SDL_JOYBUTTONDOWN:
    if(input.jbutton.which == 0)
    {
      frameState |= _config[input.jbutton.button];
    }
    break;
  case SDL_JOYBUTTONUP:
    if(input.jbutton.which == 0)
    {
      frameState &= (~(InputState)_config[input.jbutton.button]);
    }
    break;
  }

  _inputBuffer.Push(frameState);
  return _inputBuffer;
}

//______________________________________________________________________________
GamepadInputHandler::GamepadInputHandler() : IInputHandler()
{
  if(SDL_NumJoysticks() < 1) {}
  else
  {
    for (int i = 0; i < SDL_NumJoysticks(); ++i) {
      if (SDL_IsGameController(i))
      {
        SDL_GameControllerEventState(SDL_ENABLE);
        _gameController = SDL_GameControllerOpen(i);
        _joyStick = SDL_GameControllerGetJoystick(_gameController);
      }
    }
  }

  _config[SDL_CONTROLLER_BUTTON_X] = InputState::BTN1;
  _config[SDL_CONTROLLER_BUTTON_Y] = InputState::BTN2;
  //_config[SDL_CONTROLLER_BUTTON_A] = InputState::BTN3;
  _config[SDL_CONTROLLER_BUTTON_B] = InputState::BTN4;
  _config[SDL_CONTROLLER_BUTTON_RIGHTSHOULDER] = InputState::BTN3;
  _config[SDL_CONTROLLER_BUTTON_DPAD_UP] = InputState::UP;
  _config[SDL_CONTROLLER_BUTTON_DPAD_DOWN] = InputState::DOWN;
  _config[SDL_CONTROLLER_BUTTON_DPAD_LEFT] = InputState::LEFT;
  _config[SDL_CONTROLLER_BUTTON_DPAD_RIGHT] = InputState::RIGHT;
}

GamepadInputHandler::~GamepadInputHandler()
{
  //close game controller if it exists
  if(_gameController)
  {
    SDL_GameControllerClose(_gameController);
    _gameController = nullptr;
  }
}

//______________________________________________________________________________
InputBuffer const& GamepadInputHandler::CollectInputState()
{
  InputState frameState = _inputBuffer.Latest();

  const SDL_Event& input = GameManager::Get().GetLocalInput();
  switch (input.type)
  {
    case SDL_CONTROLLERBUTTONDOWN:
    {
      frameState |= _config[(SDL_GameControllerButton)input.cbutton.button];
      break;
    }
    case SDL_CONTROLLERBUTTONUP:
    {
      frameState &= (~(InputState)_config[(SDL_GameControllerButton)input.cbutton.button]);
      break;
    }

    case SDL_CONTROLLERAXISMOTION:
    {
      if (input.caxis.axis == SDL_CONTROLLER_AXIS_LEFTX)
      {
        if (input.caxis.value > JOYSTICK_DEAD_ZONE)
        {
          frameState |= InputState::RIGHT;
          frameState &= ~InputState::LEFT;
        }
        else if (input.caxis.value < -JOYSTICK_DEAD_ZONE)
        {
          frameState |= InputState::LEFT;
          frameState &= ~InputState::RIGHT;
        }
        else
        {
          frameState &= ~InputState::RIGHT;
          frameState &= ~InputState::LEFT;
        }
      }
      if (input.caxis.axis == SDL_CONTROLLER_AXIS_LEFTY)
      {
        if (input.caxis.value > JOYSTICK_DEAD_ZONE)
        {
          frameState |= InputState::DOWN;
          frameState &= ~InputState::UP;
        }
        else if (input.caxis.value < -JOYSTICK_DEAD_ZONE)
        {
          frameState |= InputState::UP;
          frameState &= ~InputState::DOWN;
        }
        else
        {
          frameState &= ~InputState::UP;
          frameState &= ~InputState::DOWN;
        }
      }
    }
    break;
  }

  _inputBuffer.Push(frameState);
  return _inputBuffer;
}
