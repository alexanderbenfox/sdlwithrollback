#include "Core/FSM/FighterFSMComponent.h"
#include "Core/FSM/FighterStateTable.h"

//______________________________________________________________________________
void FighterFSMComponent::Deserialize(std::istream& is)
{
  Serializer<uint8_t>::Deserialize(is, characterID);

  uint8_t stateVal;
  Serializer<uint8_t>::Deserialize(is, stateVal);
  currentState = static_cast<FighterStateID>(stateVal);

  Serializer<uint8_t>::Deserialize(is, stateVal);
  previousState = static_cast<FighterStateID>(stateVal);

  Serializer<float>::Deserialize(is, statePlayTime);
  Serializer<int>::Deserialize(is, stateFrame);
  Serializer<int>::Deserialize(is, stateTotalFrames);
  Serializer<bool>::Deserialize(is, stateJustEntered);
  Serializer<bool>::Deserialize(is, hitConfirmed);

  // Restore the non-serialized pointer from characterID
  stateTable = &FighterStateTable::Get().GetTable(characterID);
}
