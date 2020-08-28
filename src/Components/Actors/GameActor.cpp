#include "Components/Actors/GameActor.h"
#include <sstream>

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

//______________________________________________________________________________
std::string GameActor::Log()
{
  std::stringstream ss;
  ss << "GameActor: \n";
  ss << "\tInput State: " << (int)input.normal << "\n";
  ss << "\tSpecial Input State: " << (int)input.special << "\n";
  ss << "\tIs New Input?: " << newInputs << "\n";
  ss << "\tForce new input: " << forceNewInputOnNextFrame << "\n";
  ss << "\tAction complete: " << actionTimerComplete << "\n";
  return ss.str();
}
