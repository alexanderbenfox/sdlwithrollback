#pragma once
#include "Core/ECS/IComponent.h"
#include "Core/Interfaces/Serializable.h"
#include "Core/FSM/CharacterState.h"

#include <sstream>
#include <array>

//______________________________________________________________________________
// Forward declaration
class FighterStateTable;

//______________________________________________________________________________
struct FighterFSMComponent : public IComponent, public ISerializable
{
  uint8_t characterID = 0;
  FighterStateID currentState = FighterStateID::Idle;
  FighterStateID previousState = FighterStateID::Idle;

  // Timer for timed states
  float statePlayTime = 0.0f;
  int stateFrame = 0;
  int stateTotalFrames = 0;

  // State was just entered this frame (triggers EnactState)
  bool stateJustEntered = true;

  // Latched hitting flag: set when state.hitting is true, cleared on state transition.
  // Survives through hitstop (when HitSystem and FSM may not tick) so the cancel
  // window extends until the player transitions out of the attack state.
  bool hitConfirmed = false;

  // Pointer to state definitions table (NOT serialized)
  const std::array<StateDefinition, static_cast<size_t>(FighterStateID::COUNT)>* stateTable = nullptr;

  const StateDefinition& GetCurrentStateDef() const
  {
    return (*stateTable)[static_cast<size_t>(currentState)];
  }

  void Serialize(std::ostream& os) const override
  {
    Serializer<uint8_t>::Serialize(os, characterID);
    Serializer<uint8_t>::Serialize(os, static_cast<uint8_t>(currentState));
    Serializer<uint8_t>::Serialize(os, static_cast<uint8_t>(previousState));
    Serializer<float>::Serialize(os, statePlayTime);
    Serializer<int>::Serialize(os, stateFrame);
    Serializer<int>::Serialize(os, stateTotalFrames);
    Serializer<bool>::Serialize(os, stateJustEntered);
    Serializer<bool>::Serialize(os, hitConfirmed);
  }

  void Deserialize(std::istream& is) override;

  std::string Log() override
  {
    std::stringstream ss;
    ss << "FighterFSMComponent:\n";
    ss << "\tCharacter ID: " << (int)characterID << "\n";
    ss << "\tCurrent State: " << (int)currentState << "\n";
    ss << "\tPrevious State: " << (int)previousState << "\n";
    ss << "\tState Frame: " << stateFrame << "/" << stateTotalFrames << "\n";
    ss << "\tJust Entered: " << stateJustEntered << "\n";
    return ss.str();
  }
};
