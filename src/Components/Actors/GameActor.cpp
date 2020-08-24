#include "Components/Actors/GameActor.h"

//______________________________________________________________________________
void GameActor::TransferInputData(const InputBuffer& buffer, const StateComponent* stateInfo)
{
  newInputs = false;
  if (forceNewInputOnNextFrame || buffer.Latest() != input.normal || buffer.GetLastSpecialInput() != input.special)
  {
    input.normal = buffer.Latest();
    input.special = buffer.GetLastSpecialInput();
    newInputs = true;
  }
  forceNewInputOnNextFrame = false;

  if (stateInfo->collision != lastState.collision || stateInfo->onNewState)
  {
    lastState = *stateInfo;
    newInputs = true;
  }
}

//______________________________________________________________________________
void GameActor::Serialize(std::ostream& os) const
{
  Serializer<InputState>::Serialize(os, input.normal);
  Serializer<SpecialInputState>::Serialize(os, input.special);
  Serializer<bool>::Serialize(os, newInputs);
  Serializer<bool>::Serialize(os, forceNewInputOnNextFrame);
  Serializer<bool>::Serialize(os, actionTimerComplete);
  lastState.Serialize(os);
}

//______________________________________________________________________________
void GameActor::Deserialize(std::istream& is)
{
  Serializer<InputState>::Deserialize(is, input.normal);
  Serializer<SpecialInputState>::Deserialize(is, input.special);
  Serializer<bool>::Deserialize(is, newInputs);
  Serializer<bool>::Deserialize(is, forceNewInputOnNextFrame);
  Serializer<bool>::Deserialize(is, actionTimerComplete);
  lastState.Deserialize(is);
}
