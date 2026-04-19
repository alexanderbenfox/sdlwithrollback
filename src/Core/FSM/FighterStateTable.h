#pragma once
#include "Core/FSM/CharacterState.h"
#include <array>
#include <string>
#include <unordered_map>
#include <vector>

namespace Json { class Value; }

//______________________________________________________________________________
class FighterStateTable
{
public:
  using StateArray = std::array<StateDefinition, static_cast<size_t>(FighterStateID::COUNT)>;

  static FighterStateTable& Get();

  const StateArray& GetTable(uint8_t characterID) const;

  // Hot-reload a single character's state table from disk
  void Reload(const std::string& characterName);

private:
  FighterStateTable();

  // Discover character directories and load their states.json files
  void DiscoverAndLoad();

  // Load a single character's states.json into _tables
  bool LoadCharacter(const std::string& name, const std::string& jsonPath);

  // Parse a single state definition from JSON, applying template if present
  void ParseState(const std::string& stateName, const Json::Value& stateJson,
                  const std::unordered_map<std::string, Json::Value>& stateTemplates,
                  const std::unordered_map<std::string, TransitionRule>& transitionTemplates,
                  StateDefinition& outState);

  // Apply fields from a JSON object onto a StateDefinition
  void ApplyStateFields(const Json::Value& json, StateDefinition& state,
                        const std::unordered_map<std::string, TransitionRule>& transitionTemplates);

  // Parse a single transition rule (inline object or template reference)
  TransitionRule ParseTransition(const Json::Value& json);

  // Resolve special variable strings like "$nDashFrames"
  int ResolveTimerFrames(const Json::Value& val);

  // Character name → state array. unordered_map guarantees pointer stability.
  std::unordered_map<std::string, StateArray> _tables;

  // Ordered list of character names, indexed by characterID
  std::vector<std::string> _characterOrder;
};
