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

  virtual void Load(const Json::Value& json) override
  {
    for (auto& component : json.getMemberNames())
    {
      std::unordered_map<std::string, Json::Value> args;
      for (auto& param : json[component].getMemberNames())
      {
        args.emplace(param, json[component][param]);
      }
      instructions.emplace(component, args);
    }
  }

  virtual void Write(Json::Value& json) const override {}

};

//! Struct defining the event occuring on a given frame
struct EventData : public IJsonLoadable
{
  Rect<double> hitbox;
  Vector2<float> movement;
  EntityCreationData create;
  bool isActive = false;

  virtual void Load(const Json::Value& json) override
  {
    if (!json["hitbox"].isNull())
    {
      Vector2<double> position(json["hitbox"]["position"]["x"].asDouble(), json["hitbox"]["position"]["y"].asDouble());
      Vector2<double> size(json["hitbox"]["size"]["x"].asDouble(), json["hitbox"]["size"]["y"].asDouble());
      hitbox = Rect<double>(position.x, position.y, position.x + size.x, position.y + size.y);
      isActive = true;
    }

    if (!json["movement"].isNull())
    {
      movement = Vector2<int>(json["movement"]["x"].asInt(), json["movement"]["y"].asInt());
    }

    if (!json["createentity"].isNull())
    {
      create.Load(json["createentity"]);
      isActive = true;
    }
    if (!json["active"].isNull())
    {
      isActive = true;
    }
  }

  virtual void Write(Json::Value& json) const override
  {
    bool written = false;
    if (!(movement.x == 0 || movement.y == 0))
    {
      json["movement"]["x"] = movement.x;
      json["movement"]["y"] = movement.y;
      written = true;
    }
    if (hitbox.Area() != 0)
    {
      json["hitbox"]["position"]["x"] = hitbox.beg.x;
      json["hitbox"]["position"]["y"] = hitbox.beg.y;
      json["hitbox"]["size"]["x"] = hitbox.Width();
      json["hitbox"]["size"]["y"] = hitbox.Height();
      written = true;
    }
    if (!create.instructions.empty())
    {
      /* PLACEHOLDER SECTION UNTIL I FIGURE OUT HOW WE WANT TO USE THIS MAYBE PREFABS INSTEAD*/
      written = true;
    }
    if (!written && isActive)
    {
      json["active"] = true;
    }
  }
};

//! combines animation and events
struct ActionAsset : public IJsonLoadable
{
  ActionAsset() = default;
  ActionAsset(FrameData fData) : frameData(fData) {}
  FrameData frameData;
  std::vector<EventData> eventData;

  virtual void Load(const Json::Value& json) override
  {
    frameData.Load(json);
    if (!json["events"].isNull() && json["events"].isArray())
    {
      for (auto& value : json["events"])
      {
        eventData.emplace_back(EventData());
        eventData.back().Load(value);
      }
    }
  }

  virtual void Write(Json::Value& json) const override
  {
    frameData.Write(json);

    if (json["events"].isNull())
      json["events"] = Json::Value(Json::ValueType::arrayValue);

    for (auto& data : eventData)
    {
      Json::Value jsonObject(Json::ValueType::objectValue);
      data.Write(jsonObject);
      json["events"].append(jsonObject);
    }
  }
};
