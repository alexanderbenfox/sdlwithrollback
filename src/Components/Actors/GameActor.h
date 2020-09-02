#pragma once
#include "Components/StateComponent.h"
#include "Components/InputHandlers/InputBuffer.h"

//______________________________________________________________________________
struct GameActor : public IComponent, ISerializable
{
  struct InputData
  {
    //! Last normal button pressed
    InputState normal = InputState::NONE;
    //! Last special input from sp input buffer
    SpecialInputState special = SpecialInputState::NONE;
  };

  //! Set of last processed input states
  InputData input;
  //! Copy of last state. Used to check if latest input should be analyzed
  StateComponent lastState;
  //! Tells systems that they should check inputs
  bool newInputs = true;

  //! Forces this game actor to update the input state and flag newInputs so that the state would be checked again
  bool forceNewInputOnNextFrame = false;
  //! indicates current action is complete and entity should trigger its "TransitionTo" action if it has one
  bool actionTimerComplete = false;

  //! Updates input state helper function
  void TransferInputData(const InputBuffer& buffer, const StateComponent* stateInfo);
  //! Serializer overrides
  void Serialize(std::ostream& os) const override;
  void Deserialize(std::istream& is) override;
  std::string Log() override;

};

//______________________________________________________________________________
//! Empty flag for identifying entity as an actor in the scene
struct Actor : public IComponent {};
