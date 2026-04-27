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

//! Typed projectile/entity-spawn data — replaces the old unstructured component map.
//! Fields match the component set that AddComponents actually supports.
//! JSON format is unchanged (component-centric) for backward compatibility.
struct ProjectileData : public IJsonLoadable
{
  // Spawn transform (relative to creator)
  Vector2<float> position;
  Vector2<float> size;
  Vector2<float> scale = {1.f, 1.f};
  bool relativeToCreator = true;

  // Animation
  std::string animCollection;
  std::string animName;
  bool animLooped = true;

  // Physics
  Vector2<float> velocity;
  bool useGravity = false;

  // Collider
  Vector2<float> colliderSize;

  // Hitbox
  Vector2<double> hitboxSize;
  int damage = 0;
  Vector2<float> knockback;
  int stunFramesHit = 0;
  int stunFramesBlock = 0;
  bool followTransform = true;
  bool destroyOnHit = true;

  // Hurtbox (optional — for projectiles that can be destroyed)
  bool hasHurtbox = false;
  Vector2<double> hurtboxSize;

  bool IsEmpty() const;

  void AddComponents(EntityID creatorID, const Transform* creator, const StateComponent* creatorState, std::shared_ptr<Entity> entity) const;

  virtual void Load(const Json::Value& json) override;
  virtual void Write(Json::Value& json) const override;
  virtual void DisplayInEditor() override;
};

//! Struct defining the event occuring on a given frame
struct EventData : public IJsonLoadable
{
  Rect<double> hitbox;
  Vector2<float> movement;
  ProjectileData create;
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
