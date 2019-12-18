#include "Input.h"

//Analog joystick dead zone
const int JOYSTICK_DEAD_ZONE = 8000;

//______________________________________________________________________________
void operator|=(InputState& the, InputState other)
{
  the = (InputState)((unsigned char)the | (unsigned char)other);
}

//______________________________________________________________________________
void operator&=(InputState& the, InputState other)
{
  the = (InputState)((unsigned char)the & (unsigned char)other);
}

//______________________________________________________________________________
InputState operator&(InputState a, InputState b)
{
  return (InputState)((unsigned char)a & (unsigned char)b);
}

//______________________________________________________________________________
InputState operator~(InputState og)
{
  return (InputState)~((unsigned char)og);
}

//______________________________________________________________________________
KeyboardInputHandler::KeyboardInputHandler()
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
ICommand* KeyboardInputHandler::HandleInput(SDL_Event* input)
{
  InputState frameState = _lastFrameState;
  ICommand* command = nullptr;

  _keyStates = SDL_GetKeyboardState(0);
  if (input)
  {
    SDL_Keycode key = input->key.keysym.sym;
    switch (input->type)
    {
    case SDL_KEYDOWN:
      frameState |= _config[key];
      break;
    case SDL_KEYUP:
      frameState &= (~_config[key]);
      break;
    }
  }

  if (HasState(frameState, InputState::UP))
    command = new UpCommand;
  else if (HasState(frameState, InputState::DOWN))
    command = new DownCommand;
  else if (HasState(frameState, InputState::RIGHT))
    command = new RightCommand;
  else if (HasState(frameState, InputState::LEFT))
    command = new LeftCommand;
  else if (HasState(frameState, InputState::BTN1))
    command = new JabCommand;
  else command = new EmptyCommand;

  _lastFrameState = frameState;
  return command;
}

//______________________________________________________________________________
JoystickInputHandler::JoystickInputHandler()
{
  if(SDL_NumJoysticks() < 1) {}
  else
  {
    SDL_JoystickEventState(SDL_ENABLE);
    _gameController = SDL_JoystickOpen(_joyStickID);
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
ICommand* JoystickInputHandler::HandleInput(SDL_Event* input)
{
  InputState frameState = _lastFrameState;

  //reset movement state
  frameState &= ~((InputState)(0xf0));

  if (input)
  {
    switch (input->type)
    {
    case SDL_JOYAXISMOTION:
      //action on axis of this controller
      if(input->jaxis.which == _joyStickID)
      {
        //movement on x axis
        if(input->jaxis.axis == 0)
        {
          if(input->jaxis.value < -JOYSTICK_DEAD_ZONE)
          {
            frameState |= InputState::RIGHT;
          }
          else if (input->jaxis.value > JOYSTICK_DEAD_ZONE)
          {
            frameState |= InputState::LEFT;
          }
        }
        else if (input->jaxis.axis == 1)
        {
          if(input->jaxis.value < -JOYSTICK_DEAD_ZONE)
          {
            frameState |= InputState::DOWN;
          }
          else if (input->jaxis.value > JOYSTICK_DEAD_ZONE)
          {
            frameState |= InputState::UP;
          }
        }
      }
      break;
    case SDL_JOYBUTTONDOWN:
      if(input->jbutton.which == _joyStickID)
      {
        frameState |= _config[input->jbutton.button];
      }
      break;
    case SDL_JOYBUTTONUP:
      if(input->jbutton.which == _joyStickID)
      {
        frameState &= (~_config[input->jbutton.button]);
      }
      break;
    }
  }
  return new EmptyCommand;
}
