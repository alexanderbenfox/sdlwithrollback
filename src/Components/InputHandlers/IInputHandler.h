#pragma once
#include "Components/InputHandlers/InputBuffer.h"
#include "Core/Utility/ConfigMap.h"

//______________________________________________________________________________
//! Interface for input handlers
class IInputHandler
{
public:
  // initialize with input buffer of 6 frames
  IInputHandler() : _inputBuffer(20) {}
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