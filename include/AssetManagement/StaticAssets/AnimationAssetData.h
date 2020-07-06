#pragma once
#include "Core/Math/Vector2.h"
#include "Core/Geometry2D/Rect.h"

#include <vector>

#include <json/value.h>

struct IJsonLoadable
{
  virtual void Load(const Json::Value&) = 0;
  virtual void Write(Json::Value&) = 0;
};

struct FrameData : public IJsonLoadable
{
  FrameData() = default;
  FrameData(int su, int act, int rec, int oha, int oba, int dmg, Vector2<float> k, int hs) : startUp(su), active(act), recover(rec), onHitAdvantage(oha), onBlockAdvantage(oba), damage(dmg), knockback(k), hitstop(hs) {}
  // # of start up frames, active frames, and recovery frames
  int startUp, active, recover;
  // # of frames the receiver should be stunned on hit or block after attacker returns to neutral (can be + or -)
  int onHitAdvantage, onBlockAdvantage;
  // damage that the move does
  int damage;
  // knockback vector oriented from attack's source
  Vector2<float> knockback;
  // number of frames of action pause
  int hitstop;

  virtual void Load(const Json::Value& json) override
  {
    if (!json["framedata"].isNull())
    {
      const Json::Value& frameData = json["framedata"];
      startUp = frameData["startup"].asInt();
      active = frameData["active"].asInt();
      recover = frameData["recovery"].asInt();
      onHitAdvantage = frameData["onhit"].asInt();
      onBlockAdvantage = frameData["onblock"].asInt();
      damage = frameData["damage"].asInt();
      knockback = Vector2<float>(frameData["knockback"]["x"].asInt(), frameData["knockback"]["y"].asInt());
      hitstop = frameData["hitstop"].asInt();
    }
  }

  virtual void Write(Json::Value& json) override
  {
    if (json["framedata"].isNull())
    {
      json["framedata"] = Json::Value(Json::ValueType::objectValue);
    }

    if (!json["framedata"].isNull())
    {
      Json::Value& frameData = json["framedata"];
      frameData["startup"] = startUp;
      frameData["active"] = active;
      frameData["recovery"] = recover;
      frameData["onhit"] = onHitAdvantage;
      frameData["onblock"] = onBlockAdvantage;
      frameData["damage"] = damage;
      frameData["knockback"]["x"] = knockback.x;
      frameData["knockback"]["y"] = knockback.y;
      frameData["hitstop"] = hitstop;
    }
  }
};

//! Holds the data for one frame of attack that is hitting opponent
struct HitData
{
  // how many frames from the moment you get hit will you be unable to do another action
  int framesInStunBlock, framesInStunHit;
  Vector2<float> knockback;
  int damage;

};

enum class AnchorPoint
{
  TL, TR, BL, BR, Size
};

struct SpriteSheet : public IJsonLoadable
{
  SpriteSheet() = default;
  SpriteSheet(const char* src, int rows, int columns);
  SpriteSheet(const char* src, int rows, int columns, bool dontLoad) : src(src), rows(rows), columns(columns) {}

  void GenerateSheetInfo();
  //!
  std::string src;
  //!
  Vector2<int> frameSize;
  Vector2<int> sheetSize;
  //!
  int rows, columns;

  virtual void Load(const Json::Value& json) override
  {
    if (!json["sheet_location"].isNull())
    {
      src = json["sheet_location"].asString();
    }
    if (!json["rows"].isNull())
    {
      rows = json["rows"].asInt();
    }
    if (!json["columns"].isNull())
    {
      columns = json["columns"].asInt();
    }
    if (!json["frameSize"].isNull())
    {
      frameSize = Vector2<int>(json["frameSize"]["x"].asInt(), json["frameSize"]["y"].asInt());
    }
    if (!json["sheetSize"].isNull())
    {
      sheetSize = Vector2<int>(json["sheetSize"]["x"].asInt(), json["sheetSize"]["y"].asInt());
    }

    if (frameSize == Vector2<int>::Zero || sheetSize == Vector2<int>::Zero)
    {
      GenerateSheetInfo();
    }
  }

  virtual void Write(Json::Value& json) override
  {
    json["sheet_location"] = src;
    json["rows"] = rows;
    json["columns"] = columns;
    json["frameSize"]["x"] = frameSize.x;
    json["frameSize"]["y"] = frameSize.y;
    json["sheetSize"]["x"] = sheetSize.x;
    json["sheetSize"]["y"] = sheetSize.y;
  }
};

struct AnimationInfo : public IJsonLoadable
{
  AnimationInfo() = default;
  AnimationInfo(const std::string& loc, SpriteSheet sheet, int startIndex, int nFrames, AnchorPoint anch) : sheetLocation(loc), sheet(sheet), startIndexOnSheet(startIndex), frames(nFrames), anchor(anch) {}
  std::string sheetLocation;
  SpriteSheet sheet;
  int startIndexOnSheet;
  int frames;
  AnchorPoint anchor;

  virtual void Load(const Json::Value& json) override
  {
    if (!json["spritesheet"].isNull())
    {
      std::string spriteSheetName = json["spritesheet"].asString();
      /*if (loadedSpriteSheets.find(spriteSheetName) != loadedSpriteSheets.end())
      {
        sheet = loadedSpriteSheets[spriteSheetName];
      }*/
      sheetLocation = spriteSheetName;
    }
    if (!json["startIndex"].isNull())
    {
      startIndexOnSheet = json["startIndex"].asInt();
    }
    if (!json["numFrames"].isNull())
    {
      frames = json["numFrames"].asInt();
    }
    if (!json["anchor"].isNull())
    {
      std::string anch = json["anchor"].asString();
      if (anch == "TL")
        anchor = AnchorPoint::TL;
      else if (anch == "TR")
        anchor = AnchorPoint::TR;
      else if (anch == "BL")
        anchor = AnchorPoint::BL;
      else
        anchor = AnchorPoint::BR;
    }
  }

  virtual void Write(Json::Value& json) override
  {
    json["spritesheet"] = sheet.src;
    json["startIndex"] = startIndexOnSheet;
    json["numFrames"] = frames;
    json["anchor"] = anchor == AnchorPoint::TL ? "TL" : anchor == AnchorPoint::TR ? "TR" : anchor == AnchorPoint::BL ? "BL" : "BR";
  }
};

#include <unordered_map>
class Entity;
struct Transform;
class StateComponent;

// placeholder right now for attacks that will create an entity
struct EntityCreationData : public IJsonLoadable
{
  std::unordered_map<std::string, std::unordered_map<std::string, Json::Value>> instructions;

  void AddComponents(Transform* creator, const StateComponent* creatorState, std::shared_ptr<Entity> entity) const;

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

  virtual void Write(Json::Value& json) override
  {

  }


};

struct AnimationActionEventData : public IJsonLoadable
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

  virtual void Write(Json::Value& json) override
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

struct AttackAnimationData : public IJsonLoadable
{
  AttackAnimationData() = default;
  AttackAnimationData(AnimationInfo animInfo, FrameData fData) : loadingInfo(animInfo), frameData(fData) {}
  AnimationInfo loadingInfo;
  FrameData frameData;
  std::vector<AnimationActionEventData> eventData;

  virtual void Load(const Json::Value& json) override
  {
    loadingInfo.Load(json);
    frameData.Load(json);
    if (!json["events"].isNull() && json["events"].isArray())
    {
      for (auto& value : json["events"])
      {
        eventData.emplace_back(AnimationActionEventData());
        eventData.back().Load(value);
      }
    }
  }

  virtual void Write(Json::Value& json) override
  {
    loadingInfo.Write(json);
    frameData.Write(json);

    if(json["events"].isNull())
      json["events"] = Json::Value(Json::ValueType::arrayValue);

    for (auto& data : eventData)
    {
      Json::Value jsonObject(Json::ValueType::objectValue);
      data.Write(jsonObject);
      json["events"].append(jsonObject);
    }
  }

};