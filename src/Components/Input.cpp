#include "Components/Input.h"
#include "Components/GameActor.h"

#include "GameManagement.h"

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
InputState operator|(InputState a, InputState b)
{
  return (InputState)((unsigned char)a | (unsigned char)b);
}

//______________________________________________________________________________
InputState operator~(InputState const& other)
{
  return (InputState)~((unsigned char)other);
}

//______________________________________________________________________________
InputBuffer::InputBuffer(int limit) : _limit(limit)
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
}

//______________________________________________________________________________
SpecialInputState InputBuffer::Evaluate(const TrieNode<InputState, SpecialInputState>& spMoveDict) const
{
  std::list<InputState> latestCompletedSequence = {};
  std::deque<std::list<InputState>> prefixes;
  // push back an empty list to begin search
  prefixes.push_back({});

  for (auto it = _buffer.begin(); it != _buffer.end(); ++it)
  {
    InputState curr = *it;

    // only concerned with directional input here, so we just look at the bottom 4 bits
    unsigned char chopped = (unsigned char)curr << 4;
    curr = (InputState)(chopped >> 4);

    std::deque<std::list<InputState>> outputSequences;

    while (!prefixes.empty())
    {
      std::list<InputState>& frontier = prefixes.front();

      // copy frontier and push our next state onto the end
      std::list<InputState> searchInput = frontier;
      searchInput.push_back(curr);
      TrieReturnValue result = spMoveDict.Search(searchInput);

      if (result == TrieReturnValue::Leaf)
        latestCompletedSequence = searchInput;
      else if (result == TrieReturnValue::Branch)
      {
        if (std::find(outputSequences.begin(), outputSequences.end(), searchInput) == outputSequences.end())
          outputSequences.push_back(searchInput);
      }
      // only push unique states
      if (std::find(outputSequences.begin(), outputSequences.end(), frontier) == outputSequences.end())
        outputSequences.push_back(frontier);

      prefixes.pop_front();
    }
    // use the output of this round for the next round
    prefixes = outputSequences;
  }

  if (latestCompletedSequence.empty())
    return SpecialInputState::NONE;
  return spMoveDict.GetKeyValue(latestCompletedSequence);
}

//______________________________________________________________________________
void InputBuffer::Clear()
{
  for(int i = 0; i < _limit; i++)
    _buffer[i] = InputState::NONE;
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
    if(input.jaxis.which == _joyStickID)
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
    if(input.jbutton.which == _joyStickID)
    {
      frameState |= _config[input.jbutton.button];
    }
    break;
  case SDL_JOYBUTTONUP:
    if(input.jbutton.which == _joyStickID)
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

//______________________________________________________________________________
void GameInputComponent::AssignHandler(InputType type)
{
  _assignedHandler = type;
  switch (type)
  {
  case InputType::Keyboard:
    _handler = &_keyboard;
    break;
  case InputType::Gamepad:
    _handler = &_gamepad;
    break;
  case InputType::Joystick:
    _handler = &_joystick;
  default:
    break;
  }
}

//______________________________________________________________________________
InputBuffer const& GameInputComponent::QueryInput()
{
  return _handler->CollectInputState();
}

//______________________________________________________________________________
void GameInputComponent::Clear()
{
  _handler->ClearInputBuffer();
}

//______________________________________________________________________________
void GameInputComponent::OnDebug() 
{
  int entityId = _owner->GetID();
  std::string pName = "P" + std::to_string(entityId);
  if (ImGui::CollapsingHeader(pName.c_str()))
  {
    const char* items[] = { "Keyboard", "Joystick", "Gamepad" };
    static const char* currentItem = nullptr;
    currentItem = items[(int)_assignedHandler];
    auto func = [this](const std::string& i)
    {
      if (i == "Keyboard")
        AssignHandler(InputType::Keyboard);
      else if (i == "Joystick")
        AssignHandler(InputType::Joystick);
      else if (i == "Gamepad")
        AssignHandler(InputType::Gamepad);
    };
    DropDown::Show(currentItem, items, 3, func);
  }
}

//______________________________________________________________________________
template <>
void GameInputComponent::AssignActionKey<SDL_Keycode>(SDL_Keycode key, InputState action)
{
  _keyboard.AssignKey(key, action);
}

//______________________________________________________________________________
template <>
void GameInputComponent::AssignActionKey<uint8_t>(uint8_t key, InputState action)
{
  _joystick.AssignKey(key, action);
}

//______________________________________________________________________________
template <>
void GameInputComponent::AssignActionKey<SDL_GameControllerButton>(SDL_GameControllerButton key, InputState action)
{
  _gamepad.AssignKey(key, action);
}

#ifdef _WIN32
//______________________________________________________________________________
InputBuffer const& GGPOInputHandler::CollectInputState()
{
  // notify ggpo of local player's inputs
  GGPOErrorCode result = ggpo_add_local_input(_input->session, (*_input->handles)[0], &(*_input->inputs)[0], sizeof((*_input->inputs)[0]));

  // synchronize inputs
  if (GGPO_SUCCEEDED(result))
  {
    int disconnectFlags;
    result = ggpo_synchronize_input(_input->session, (*_input->inputs), sizeof(*_input->inputs), &disconnectFlags);
    if (GGPO_SUCCEEDED(result))
    {
      _inputBuffer.Push((*_input->inputs)[1]);
    }
    else
    {
      _inputBuffer.Push(InputState::NONE);
    }
  }
  else
  {
    _inputBuffer.Push(InputState::NONE);
  }
  return _inputBuffer;

}
#endif