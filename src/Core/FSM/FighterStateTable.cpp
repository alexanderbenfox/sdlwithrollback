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
const FighterStateTable::StateArray& FighterStateTable::GetTable(const std::string& characterName) const
{
  auto it = _tables.find(characterName);
  if (it != _tables.end())
    return it->second;
  // Fallback to first loaded character
  return _tables.begin()->second;
}

//______________________________________________________________________________
int FighterStateTable::GetCharacterID(const std::string& characterName) const
{
  for (int i = 0; i < static_cast<int>(_characterOrder.size()); ++i)
  {
    if (_characterOrder[i] == characterName)
      return i;
  }
  return -1;
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

  // Cancel type
  if (json.isMember("cancelType"))
    rule.cancelType = CancelTypeFromString(json["cancelType"].asString());
  else if (json.isMember("isCancel") && json["isCancel"].asBool())
    rule.cancelType = CancelType::Cancel; // backwards compat

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

//______________________________________________________________________________
FighterStateTable::StateArray* FighterStateTable::GetMutableTable(const std::string& characterName)
{
  auto it = _tables.find(characterName);
  if (it != _tables.end())
    return &it->second;
  return nullptr;
}

//______________________________________________________________________________
std::string FighterStateTable::GetJsonPath(const std::string& characterName) const
{
  return ResourceManager::Get().GetResourcePath() + "json/characters/" + characterName + "/states.json";
}

//______________________________________________________________________________
void FighterStateTable::WriteStatesToJson(const std::string& characterName) const
{
  auto it = _tables.find(characterName);
  if (it == _tables.end()) return;

  const StateArray& states = it->second;
  std::string jsonPath = GetJsonPath(characterName);

  // Read existing file to preserve transitionTemplates and stateTemplates
  Json::Value root;
  {
    std::ifstream inFile(jsonPath);
    if (inFile.is_open())
    {
      try { inFile >> root; }
      catch (...) { root = Json::Value(Json::objectValue); }
    }
  }

  // Rebuild the "states" section with current data
  Json::Value& statesJson = root["states"];
  statesJson = Json::Value(Json::objectValue);

  for (size_t i = 0; i < static_cast<size_t>(FighterStateID::COUNT); ++i)
  {
    const StateDefinition& state = states[i];
    if (state.animationName.empty() && state.transitions.empty())
      continue;

    const char* stateName = FighterStateIDToString(state.id);
    Json::Value& sj = statesJson[stateName];

    // Animation
    sj["animationName"] = state.animationName;
    if (state.loopAnimation) sj["loopAnimation"] = true;
    if (state.forceAnimRestart) sj["forceAnimRestart"] = true;
    if (state.playSpeed != 1.0f) sj["playSpeed"] = state.playSpeed;

    // Completion
    if (state.completionType != StateDefinition::None)
    {
      sj["completionType"] = CompletionTypeToString(state.completionType);
      sj["completionTarget"] = FighterStateIDToString(state.completionTarget);
      if (state.completionType == StateDefinition::Timer && state.timerFrames > 0)
      {
        if (state.timerFrames == GlobalVars::nDashFrames)
          sj["timerFrames"] = "$nDashFrames";
        else
          sj["timerFrames"] = state.timerFrames;
      }
      else if (state.completionType == StateDefinition::Timer)
        sj["timerFrames"] = state.timerFrames;
    }

    // State properties
    sj["stanceState"] = StanceStateToString(state.stanceState);
    if (state.actionState != ActionState::NONE)
      sj["actionState"] = ActionStateToString(state.actionState);
    if (state.isHittable) sj["isHittable"] = true;
    if (state.canBlock) sj["canBlock"] = true;
    if (state.inKnockdown) sj["inKnockdown"] = true;
    if (state.isAttackState) sj["isAttackState"] = true;
    if (state.isGrappleState) sj["isGrappleState"] = true;

    // Entry movement
    if (state.entryMovement != StateDefinition::NoMovement)
      sj["entryMovement"] = EntryMovementToString(state.entryMovement);
    if (state.horizontalOnly) sj["horizontalOnly"] = true;

    // Damage-related
    if (state.appliesDamage) sj["appliesDamage"] = true;
    if (state.isBlocking) sj["isBlocking"] = true;
    if (state.setsJuggleGravity) sj["setsJuggleGravity"] = true;
    if (state.resetsJuggleGravity) sj["resetsJuggleGravity"] = true;

    // Transitions (written as inline rules, not template references)
    if (!state.transitions.empty())
    {
      Json::Value& transArr = sj["transitions"];
      transArr = Json::Value(Json::arrayValue);
      for (const auto& rule : state.transitions)
      {
        Json::Value tj;

        Json::Value& req = tj["required"];
        req = Json::Value(Json::arrayValue);
        for (size_t f = 0; f < ConditionFlagCapacity; ++f)
          if (rule.requiredFlags.test(f))
            req.append(ConditionFlagToString(static_cast<ConditionFlag>(f)));

        if (rule.forbiddenFlags.any())
        {
          Json::Value& forb = tj["forbidden"];
          forb = Json::Value(Json::arrayValue);
          for (size_t f = 0; f < ConditionFlagCapacity; ++f)
            if (rule.forbiddenFlags.test(f))
              forb.append(ConditionFlagToString(static_cast<ConditionFlag>(f)));
        }

        if (rule.targetState == FighterStateID::COUNT)
          tj["target"] = "HitResolver";
        else
          tj["target"] = FighterStateIDToString(rule.targetState);

        tj["priority"] = rule.priority;
        if (rule.cancelType != CancelType::NotCancel)
          tj["cancelType"] = CancelTypeToString(rule.cancelType);
        transArr.append(tj);
      }
    }
  }

  // Write back
  std::ofstream outFile(jsonPath);
  outFile << root.toStyledString();
  std::cout << "FighterStateTable: wrote " << jsonPath << "\n";
}

//______________________________________________________________________________
bool FighterStateTable::CreateDefaultTable(const std::string& characterName)
{
  std::string jsonPath = GetJsonPath(characterName);

  // Don't overwrite an existing file
  if (fs::exists(jsonPath))
  {
    std::cerr << "FighterStateTable::CreateDefaultTable: file already exists: " << jsonPath << "\n";
    // Still load it if it's not in memory
    if (_tables.find(characterName) == _tables.end())
    {
      if (LoadCharacter(characterName, jsonPath))
        _characterOrder.push_back(characterName);
      return true;
    }
    return false;
  }

  // Create a default state array with Idle initialized
  StateArray& states = _tables[characterName];
  for (size_t i = 0; i < static_cast<size_t>(FighterStateID::COUNT); ++i)
    states[i].id = static_cast<FighterStateID>(i);

  // Set up Idle as the minimum viable state
  StateDefinition& idle = states[static_cast<size_t>(FighterStateID::Idle)];
  idle.animationName = "Idle";
  idle.loopAnimation = true;
  idle.stanceState = StanceState::STANDING;
  idle.isHittable = true;
  idle.canBlock = true;

  // Hitstun — standing hit, timer-based recovery
  StateDefinition& hitstun = states[static_cast<size_t>(FighterStateID::Hitstun)];
  hitstun.animationName = "LightHitstun";
  hitstun.forceAnimRestart = true;
  hitstun.completionType = StateDefinition::Timer;
  hitstun.timerFrames = 0; // driven by hit data at runtime
  hitstun.completionTarget = FighterStateID::Idle;
  hitstun.stanceState = StanceState::STANDING;
  hitstun.actionState = ActionState::HITSTUN;
  hitstun.isHittable = true;
  hitstun.entryMovement = StateDefinition::UseHitKnockbackFull;
  hitstun.appliesDamage = true;

  // CrouchingHitstun — crouching hit, timer-based recovery
  StateDefinition& crouchHit = states[static_cast<size_t>(FighterStateID::CrouchingHitstun)];
  crouchHit.animationName = "CrouchingHitstun";
  crouchHit.forceAnimRestart = true;
  crouchHit.completionType = StateDefinition::Timer;
  crouchHit.timerFrames = 0;
  crouchHit.completionTarget = FighterStateID::Idle;
  crouchHit.stanceState = StanceState::CROUCHING;
  crouchHit.actionState = ActionState::HITSTUN;
  crouchHit.isHittable = true;
  crouchHit.entryMovement = StateDefinition::UseHitKnockbackFull;
  crouchHit.appliesDamage = true;

  // BlockstunStanding — standing block, timer-based recovery
  StateDefinition& blockStand = states[static_cast<size_t>(FighterStateID::BlockstunStanding)];
  blockStand.animationName = "BlockMid";
  blockStand.forceAnimRestart = true;
  blockStand.completionType = StateDefinition::Timer;
  blockStand.timerFrames = 0;
  blockStand.completionTarget = FighterStateID::Idle;
  blockStand.stanceState = StanceState::STANDING;
  blockStand.actionState = ActionState::BLOCKSTUN;
  blockStand.isHittable = true;
  blockStand.canBlock = true;
  blockStand.entryMovement = StateDefinition::StopHorizontal;
  blockStand.appliesDamage = true;
  blockStand.isBlocking = true;

  // BlockstunCrouching — crouching block, timer-based recovery
  StateDefinition& blockCrouch = states[static_cast<size_t>(FighterStateID::BlockstunCrouching)];
  blockCrouch.animationName = "BlockLow";
  blockCrouch.forceAnimRestart = true;
  blockCrouch.completionType = StateDefinition::Timer;
  blockCrouch.timerFrames = 0;
  blockCrouch.completionTarget = FighterStateID::Idle;
  blockCrouch.stanceState = StanceState::CROUCHING;
  blockCrouch.actionState = ActionState::BLOCKSTUN;
  blockCrouch.isHittable = true;
  blockCrouch.canBlock = true;
  blockCrouch.entryMovement = StateDefinition::StopHorizontal;
  blockCrouch.appliesDamage = true;
  blockCrouch.isBlocking = true;

  // KnockdownAirborne — airborne after knockdown hit, lands into KnockdownHitGround
  StateDefinition& kdAir = states[static_cast<size_t>(FighterStateID::KnockdownAirborne)];
  kdAir.animationName = "Knockdown_Air";
  kdAir.forceAnimRestart = true;
  kdAir.stanceState = StanceState::KNOCKDOWN;
  kdAir.actionState = ActionState::HITSTUN;
  kdAir.isHittable = true;
  kdAir.inKnockdown = true;
  kdAir.entryMovement = StateDefinition::UseHitKnockbackFull;
  kdAir.appliesDamage = true;
  kdAir.setsJuggleGravity = true;
  {
    TransitionRule onGrounded;
    onGrounded.requiredFlags.set(CF_IsGrounded);
    onGrounded.targetState = FighterStateID::KnockdownHitGround;
    onGrounded.priority = 10;
    kdAir.transitions.push_back(onGrounded);
  }

  // KnockdownHitGround — playing ground impact anim, transitions to KnockdownOnGround
  StateDefinition& kdGround = states[static_cast<size_t>(FighterStateID::KnockdownHitGround)];
  kdGround.animationName = "Knockdown_HitGround";
  kdGround.completionType = StateDefinition::Animation;
  kdGround.completionTarget = FighterStateID::KnockdownOnGround;
  kdGround.stanceState = StanceState::KNOCKDOWN;
  kdGround.actionState = ActionState::HITSTUN;
  kdGround.isHittable = true;
  kdGround.inKnockdown = true;
  kdGround.entryMovement = StateDefinition::StopHorizontal;

  // KnockdownOnGround — lying on ground, gets up to Idle (invincible)
  StateDefinition& kdOnGround = states[static_cast<size_t>(FighterStateID::KnockdownOnGround)];
  kdOnGround.animationName = "Knockdown_OnGround";
  kdOnGround.completionType = StateDefinition::Animation;
  kdOnGround.completionTarget = FighterStateID::Idle;
  kdOnGround.stanceState = StanceState::KNOCKDOWN;
  kdOnGround.entryMovement = StateDefinition::StopHorizontal;
  kdOnGround.resetsJuggleGravity = true;

  // Grappled — being thrown, timer expires into KnockdownAirborne
  StateDefinition& grappled = states[static_cast<size_t>(FighterStateID::Grappled)];
  grappled.animationName = "HeavyHitstun";
  grappled.forceAnimRestart = true;
  grappled.completionType = StateDefinition::Timer;
  grappled.timerFrames = 0;
  grappled.completionTarget = FighterStateID::KnockdownAirborne;
  grappled.stanceState = StanceState::KNOCKDOWN;
  grappled.isGrappleState = true;

  _characterOrder.push_back(characterName);

  // Write to disk
  WriteStatesToJson(characterName);
  std::cout << "FighterStateTable: created default table for " << characterName << "\n";
  return true;
}
