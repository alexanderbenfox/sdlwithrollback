#pragma once
#include "Components/InputHandlers/InputBuffer.h"
#include "Core/Utility/ConfigMap.h"
#include <SDL2/SDL_events.h>

//______________________________________________________________________________
//! Interface for input handlers
class IInputHandler
{
public:
  // initialize with input buffer of 20 frames
  IInputHandler(InputBuffer& buffer) : _inputBuffer(buffer) {}
  //! Destructor
  virtual ~IInputHandler() {}
  //! Translates local events to an input state
  virtual InputState TranslateEvent(const SDL_Event&) = 0;
  //!
  virtual void SetInputMapKey(InputState value, SDL_Event key) = 0;
  //!
  virtual const char* GetInputName(InputState value) const { return ""; }
  //! Push input state to buffer
  virtual void CommitInput(const InputState& input) { _inputBuffer.Push(input); }
  //! Gets the command based on the type of input received from the controller
  //virtual InputBuffer& CollectInputState(const SDL_Event&) = 0;
  //! Use this to get interpretted input after it has been collected and synced if playing online
  virtual InputBuffer const& GetInterprettedInput() { return _inputBuffer; }
  //! Used for syncing ggpo input... replaces last input buffer with the synced input
  virtual void SyncLastInput(InputState syncInput) { _inputBuffer.Swap(syncInput); }
  //!
  virtual void ClearInputBuffer() { _inputBuffer.Clear(); }

protected:
  //! Last state received by the input controller
  InputBuffer& _inputBuffer;

};