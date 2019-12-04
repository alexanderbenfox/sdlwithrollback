#include "Input.h"

KeyboardInputHandler::KeyboardInputHandler()
{

}

KeyboardInputHandler::~KeyboardInputHandler()
{

}

ICommand* KeyboardInputHandler::HandleInput(SDL_Event* input)
{
  _keyStates = SDL_GetKeyboardState(0);
  if (input)
  {
    SDL_KeyboardEvent key = input->key;
    switch (input->type)
    {
    case SDL_KEYDOWN:
      break;
    case SDL_KEYUP:
      break;
    }
  }
  return new EmptyCommand;
}

JoystickInputHandler::JoystickInputHandler()
{

}

JoystickInputHandler::~JoystickInputHandler()
{

}

ICommand* JoystickInputHandler::HandleInput(SDL_Event* input)
{
  if (input)
  {
    switch (input->type)
    {
    case SDL_JOYAXISMOTION:
      break;
    case SDL_KEYUP:
      break;
    }
  }
  return new EmptyCommand;
}