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
SpecialMoveState InputBuffer::Evaluate(const TrieNode<InputState, SpecialMoveState>& spMoveDict) const
{
  std::list<InputState> latestCompletedSequence = {};
  std::deque<std::list<InputState>> potentialSequences;
  // push back an empty list to begin search
  potentialSequences.push_back({});

  for (auto it = _buffer.rbegin(); it != _buffer.rend(); ++it)
  {
    InputState curr = *it;
    std::deque<std::list<InputState>> outputSequences;

    while (!potentialSequences.empty())
    {
      std::list<InputState>& frontier = potentialSequences.front();
      for (InputState i = InputState::NONE; i <= InputState::BTN4; i = (InputState)((unsigned char)i + 1))
      {
        if ((i & curr) != InputState::NONE)
        {
          // copy frontier
          std::list<InputState> searchInput = frontier;
          searchInput.push_back(i);
          auto result = spMoveDict.Search(searchInput);

          if (result == TrieReturnValue::Leaf)
            latestCompletedSequence = searchInput;
          else if (result == TrieReturnValue::Branch)
            outputSequences.push_back(searchInput);
        }
      }
      // if its neither a leaf or branch, add it back into the potential sequences for later
      outputSequences.push_back(frontier);

      potentialSequences.pop_front();
    }
    // use the output of this round for the next round
    potentialSequences = outputSequences;
    
  }

  if (latestCompletedSequence.empty())
    return SpecialMoveState::NONE;
  return spMoveDict.GetKeyValue(latestCompletedSequence);
}

//______________________________________________________________________________
KeyboardInputHandler::KeyboardInputHandler(std::shared_ptr<Entity> owner) : IInputHandler(owner)
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
JoystickInputHandler::JoystickInputHandler(std::shared_ptr<Entity> owner) : IInputHandler(owner)
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
      else if (input.jaxis.axis == 1)
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