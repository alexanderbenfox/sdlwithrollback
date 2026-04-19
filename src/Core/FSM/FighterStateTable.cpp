#include "Core/FSM/FighterStateTable.h"
#include "Core/FSM/StateEnumMaps.h"
#include "Managers/ResourceManager.h"
#include "Globals.h"

#include <json/json.h>
#include <fstream>
#include <algorithm>
#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;

//______________________________________________________________________________
FighterStateTable& FighterStateTable::Get()
{
  static FighterStateTable instance;
  return instance;
}

//______________________________________________________________________________
const FighterStateTable::StateArray& FighterStateTable::GetTable(uint8_t characterID) const
{
  if (characterID < _characterOrder.size())
  {
    auto it = _tables.find(_characterOrder[characterID]);
    if (it != _tables.end())
      return it->second;
  }
  // Fallback to first loaded character
  return _tables.begin()->second;
}

//______________________________________________________________________________
FighterStateTable::FighterStateTable()
{
  DiscoverAndLoad();
}

//______________________________________________________________________________
void FighterStateTable::DiscoverAndLoad()
{
  std::string charDir = ResourceManager::Get().GetResourcePath() + "json/characters/";

  if (!fs::exists(charDir))
  {
    std::cerr << "FighterStateTable: character directory not found: " << charDir << "\n";
    return;
  }

  for (const auto& entry : fs::directory_iterator(charDir))
  {
    if (!entry.is_directory()) continue;

    std::string name = entry.path().filename().string();
    std::string statesPath = entry.path().string() + "/states.json";

    if (fs::exists(statesPath))
    {
      if (LoadCharacter(name, statesPath))
        _characterOrder.push_back(name);
    }
  }

  if (_characterOrder.empty())
    std::cerr << "FighterStateTable: no characters loaded!\n";
  else
    std::cout << "FighterStateTable: loaded " << _characterOrder.size() << " character(s)\n";
}

//______________________________________________________________________________
void FighterStateTable::Reload(const std::string& characterName)
{
  std::string charDir = ResourceManager::Get().GetResourcePath() + "json/characters/";
  std::string statesPath = charDir + characterName + "/states.json";

  if (!fs::exists(statesPath))
  {
    std::cerr << "FighterStateTable::Reload: file not found: " << statesPath << "\n";
    return;
  }

  // Reload in-place — pointer stays stable since unordered_map doesn't move existing entries
  LoadCharacter(characterName, statesPath);
  std::cout << "FighterStateTable: reloaded " << characterName << "\n";
}

//______________________________________________________________________________
bool FighterStateTable::LoadCharacter(const std::string& name, const std::string& jsonPath)
{
  std::ifstream file(jsonPath);
  if (!file.is_open())
  {
    std::cerr << "FighterStateTable: failed to open " << jsonPath << "\n";
    return false;
  }

  Json::Value root;
  try
  {
    file >> root;
  }
  catch (const Json::RuntimeError& err)
  {
    std::cerr << "FighterStateTable: JSON parse error in " << jsonPath << ": " << err.what() << "\n";
    return false;
  }

  // --- Parse transition templates ---
  std::unordered_map<std::string, TransitionRule> transitionTemplates;
  const Json::Value& ttSection = root["transitionTemplates"];
  for (const auto& key : ttSection.getMemberNames())
  {
    transitionTemplates[key] = ParseTransition(ttSection[key]);
  }

  // --- Parse state templates (kept as raw JSON for field application) ---
  std::unordered_map<std::string, Json::Value> stateTemplates;
  const Json::Value& stSection = root["stateTemplates"];
  for (const auto& key : stSection.getMemberNames())
  {
    stateTemplates[key] = stSection[key];
  }

  // --- Parse states ---
  StateArray& states = _tables[name];

  // Initialize all state IDs
  for (size_t i = 0; i < static_cast<size_t>(FighterStateID::COUNT); ++i)
    states[i].id = static_cast<FighterStateID>(i);

  const Json::Value& statesSection = root["states"];
  for (const auto& stateName : statesSection.getMemberNames())
  {
    FighterStateID stateID = FighterStateIDFromString(stateName);
    if (stateID == FighterStateID::Idle && stateName != "Idle")
    {
      // FighterStateIDFromString returns Idle for unknown strings — skip
      std::cerr << "FighterStateTable: unknown state '" << stateName << "' in " << name << "\n";
      continue;
    }

    ParseState(stateName, statesSection[stateName], stateTemplates, transitionTemplates,
               states[static_cast<size_t>(stateID)]);
  }

  // Sort all transition lists by descending priority (first match wins)
  for (auto& state : states)
  {
    std::sort(state.transitions.begin(), state.transitions.end(),
      [](const TransitionRule& a, const TransitionRule& b) { return a.priority > b.priority; });
  }

  return true;
}

//______________________________________________________________________________
void FighterStateTable::ParseState(
    const std::string& stateName,
    const Json::Value& stateJson,
    const std::unordered_map<std::string, Json::Value>& stateTemplates,
    const std::unordered_map<std::string, TransitionRule>& transitionTemplates,
    StateDefinition& outState)
{
  outState.id = FighterStateIDFromString(stateName);

  // Apply template first (if specified), then override with state-specific fields
  if (stateJson.isMember("template"))
  {
    const std::string& tmplName = stateJson["template"].asString();
    auto it = stateTemplates.find(tmplName);
    if (it != stateTemplates.end())
      ApplyStateFields(it->second, outState, transitionTemplates);
    else
      std::cerr << "FighterStateTable: unknown state template '" << tmplName << "'\n";
  }

  // Apply state's own fields (overrides template)
  ApplyStateFields(stateJson, outState, transitionTemplates);
}

//______________________________________________________________________________
void FighterStateTable::ApplyStateFields(
    const Json::Value& json,
    StateDefinition& state,
    const std::unordered_map<std::string, TransitionRule>& transitionTemplates)
{
  // Animation
  if (json.isMember("animationName"))
    state.animationName = json["animationName"].asString();
  if (json.isMember("loopAnimation"))
    state.loopAnimation = json["loopAnimation"].asBool();
  if (json.isMember("playSpeed"))
    state.playSpeed = json["playSpeed"].asFloat();
  if (json.isMember("forceAnimRestart"))
    state.forceAnimRestart = json["forceAnimRestart"].asBool();

  // Completion
  if (json.isMember("completionType"))
    state.completionType = CompletionTypeFromString(json["completionType"].asString());
  if (json.isMember("timerFrames"))
    state.timerFrames = ResolveTimerFrames(json["timerFrames"]);
  if (json.isMember("completionTarget"))
    state.completionTarget = FighterStateIDFromString(json["completionTarget"].asString());

  // State properties
  if (json.isMember("stanceState"))
    state.stanceState = StanceStateFromString(json["stanceState"].asString());
  if (json.isMember("actionState"))
    state.actionState = ActionStateFromString(json["actionState"].asString());
  if (json.isMember("isHittable"))
    state.isHittable = json["isHittable"].asBool();
  if (json.isMember("canBlock"))
    state.canBlock = json["canBlock"].asBool();
  if (json.isMember("inKnockdown"))
    state.inKnockdown = json["inKnockdown"].asBool();
  if (json.isMember("isAttackState"))
    state.isAttackState = json["isAttackState"].asBool();
  if (json.isMember("isGrappleState"))
    state.isGrappleState = json["isGrappleState"].asBool();

  // Entry movement
  if (json.isMember("entryMovement"))
    state.entryMovement = EntryMovementFromString(json["entryMovement"].asString());
  if (json.isMember("horizontalOnly"))
    state.horizontalOnly = json["horizontalOnly"].asBool();

  // Cancel flags
  if (json.isMember("cancelFlags"))
  {
    uint8_t flags = StateDefinition::Cancel_None;
    const Json::Value& arr = json["cancelFlags"];
    for (const auto& f : arr)
      flags |= CancelFlagFromString(f.asString());
    state.cancelFlags = flags;
  }

  // Damage-related
  if (json.isMember("appliesDamage"))
    state.appliesDamage = json["appliesDamage"].asBool();
  if (json.isMember("isBlocking"))
    state.isBlocking = json["isBlocking"].asBool();
  if (json.isMember("setsJuggleGravity"))
    state.setsJuggleGravity = json["setsJuggleGravity"].asBool();
  if (json.isMember("resetsJuggleGravity"))
    state.resetsJuggleGravity = json["resetsJuggleGravity"].asBool();

  // Transitions (completely replaces any template transitions)
  if (json.isMember("transitions"))
  {
    state.transitions.clear();
    const Json::Value& arr = json["transitions"];
    for (const auto& item : arr)
    {
      if (item.isString())
      {
        // "$templateName" reference
        const std::string& ref = item.asString();
        if (!ref.empty() && ref[0] == '$')
        {
          std::string tmplName = ref.substr(1);
          auto it = transitionTemplates.find(tmplName);
          if (it != transitionTemplates.end())
            state.transitions.push_back(it->second);
          else
            std::cerr << "FighterStateTable: unknown transition template '$" << tmplName << "'\n";
        }
      }
      else if (item.isObject())
      {
        // Inline transition rule
        state.transitions.push_back(ParseTransition(item));
      }
    }
  }
}

//______________________________________________________________________________
TransitionRule FighterStateTable::ParseTransition(const Json::Value& json)
{
  TransitionRule rule;

  // Required flags
  if (json.isMember("required"))
  {
    for (const auto& f : json["required"])
      rule.requiredFlags.set(ConditionFlagFromString(f.asString()));
  }

  // Forbidden flags
  if (json.isMember("forbidden"))
  {
    for (const auto& f : json["forbidden"])
      rule.forbiddenFlags.set(ConditionFlagFromString(f.asString()));
  }

  // Target state
  if (json.isMember("target"))
    rule.targetState = FighterStateIDFromString(json["target"].asString());

  // Priority
  if (json.isMember("priority"))
    rule.priority = static_cast<int8_t>(json["priority"].asInt());

  return rule;
}

//______________________________________________________________________________
int FighterStateTable::ResolveTimerFrames(const Json::Value& val)
{
  if (val.isInt())
    return val.asInt();

  if (val.isString())
  {
    const std::string& str = val.asString();
    if (str == "$nDashFrames")
      return GlobalVars::nDashFrames;

    std::cerr << "FighterStateTable: unknown timer variable '" << str << "'\n";
    return 0;
  }

  return 0;
}
