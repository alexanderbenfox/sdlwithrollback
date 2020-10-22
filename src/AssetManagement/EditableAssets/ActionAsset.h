#pragma once
#include <unordered_map>
#include "Globals.h"
#include "Core/Geometry2D/Rect.h"

#include "IJsonLoadable.h"
#include "AnimationAsset.h"
#include "FrameData.h"

class Entity;
struct Transform;
class StateComponent;

// placeholder right now for attacks that will create an entity
struct EntityCreationData : public IJsonLoadable
{
  std::unordered_map<std::string, std::unordered_map<std::string, Json::Value>> instructions;

  void AddComponents(EntityID creatorID, const Transform* creator, const StateComponent* creatorState, std::shared_ptr<Entity> entity) const;

  virtual void Load(const Json::Value& json) override;
  virtual void Write(Json::Value& json) const override {}
  virtual void DisplayInEditor() override;

};

//! Struct defining the event occuring on a given frame
struct EventData : public IJsonLoadable
{
  Rect<double> hitbox;
  Vector2<float> movement;
  EntityCreationData create;
  bool isActive = false;

  virtual void Load(const Json::Value& json) override;
  virtual void Write(Json::Value& json) const override;
  virtual void DisplayInEditor() override;

};

//! combines animation and events
struct ActionAsset : public IJsonLoadable
{
  ActionAsset() = default;
  ActionAsset(FrameData fData) : frameData(fData) {}
  FrameData frameData;
  std::vector<EventData> eventData;

  virtual void Load(const Json::Value& json) override;
  virtual void Write(Json::Value& json) const override;
  virtual void DisplayInEditor() override;
};
