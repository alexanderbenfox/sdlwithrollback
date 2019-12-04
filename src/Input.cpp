#include "Input.h"

//Analog joystick dead zone
const int JOYSTICK_DEAD_ZONE = 8000;

KeyboardInputHandler::KeyboardInputHandler()
{
  //assign direction buttons
  _config[SDLK_w] = InputState::UP;
  _config[SDLK_a] = InputState::LEFT;
  _config[SDLK_s] = InputState::DOWN;
  _config[SDLK_d] = InputState::RIGHT;

  //assign the button events
  _config[SDLK_i] = InputState::BTN1;
  _config[SDLK_o] = InputState::BTN2;
  _config[SDLK_p] = InputState::BTN3;
}

KeyboardInputHandler::~KeyboardInputHandler()
{

}

ICommand* KeyboardInputHandler::HandleInput(SDL_Event* input)
{
  InputState frameState = _lastFrameState;

  _keyStates = SDL_GetKeyboardState(0);
  if (input)
  {
    SDL_KeyboardEvent key = input->key;
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

  return new EmptyCommand;
}

JoystickInputHandler::JoystickInputHandler()
{
  if(SDL_NumJoysticks() < 1) {}
  else
  {
    SDL_JoystickEventState(SDL_ENABLE);
    _gameController = SDL_JoystickOpen(_joyStickID);
  }
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

ICommand* JoystickInputHandler::HandleInput(SDL_Event* input)
{
  InputState frameState = _lastFrameState;

  //reset movement state
  frameState &= ~((InputState)(0x0e));

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
