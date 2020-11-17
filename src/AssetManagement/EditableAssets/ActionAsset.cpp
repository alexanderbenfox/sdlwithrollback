#include "ActionAsset.h"
#include "Core/ECS/Entity.h"

#include "Components/Transform.h"
#include "Components/Animator.h"
#include "Components/RenderComponent.h"
#include "Components/Rigidbody.h"
#include "Components/Hitbox.h"
#include "Components/Hurtbox.h"
#include "Components/StateComponent.h"
#include "Components/MetaGameComponents.h"

#include "Managers/AnimationCollectionManager.h"
#include "Managers/GameManagement.h"

#include "Systems/ActionSystems/EnactActionSystem.h"

//______________________________________________________________________________
template <> void AssetLoaderFn::OnLoad(ActionAsset& asset) {}

//______________________________________________________________________________
void EntityCreationData::AddComponents(EntityID creatorID, const Transform* creator, const StateComponent* creatorState, std::shared_ptr<Entity> entity) const
{
  float entityWidth = 0.0f;
  Vector2<float> scale(1.0f, 1.0f);
  std::vector<RectColliderD*> moveableColliders;

  for (auto instruction : instructions)
  {
    if (instruction.first == "Transform")
    {
      ComponentInitParams<Transform> params;
      scale = params.scale = Vector2<float>(instruction.second["scalex"].asFloat(), instruction.second["scaley"].asFloat());
      params.size = Vector2<float>(instruction.second["entitysizex"].asFloat(), instruction.second["entitysizey"].asFloat());

      params.position = Vector2<float>(instruction.second["x"].asFloat(), instruction.second["y"].asFloat());
      params.position *= creator->scale;

      if (instruction.second["relative"].asBool())
      {
        if (!creatorState->onLeftSide)
        {
          params.position.x = -params.position.x;

          // this is because transform scaling extends from middle... also needs to be FIXED
          Rect<float> scaleHelper(0, 0, params.size.x, params.size.y);
          scaleHelper.Scale(Vector2<float>(1, 1), params.scale);
          params.position.x += (creator->rect.Width() - (params.size.x - scaleHelper.beg.x));
        }
        params.position.x += creator->position.x;
        params.position.y += creator->position.y;
      }

      // return scale to 1 because transform scaling is messed up and needs to be FIXED
      params.scale = Vector2<float>(1.0f, 1.0f);
      entity->AddComponent<Transform>(params);

      // set rendering basis to the entity size
      entity->AddComponent<RenderProperties>();
      entity->GetComponent<RenderProperties>()->rectTransform = params.size;
    }
    else if (instruction.first == "Animator")
    {
      ComponentInitParams<Animator> params;
      params.collectionID = GAnimArchive.GetCollectionID(instruction.second["collection"].asString());
      params.isLooped = instruction.second["isLooped"].asBool();
      params.name = instruction.second["anim"].asString();
      params.horizontalFlip = !creatorState->onLeftSide;
      params.speed = 1.0f;

      entity->AddComponent<Animator>(params);
      entity->AddComponent<RenderProperties>();
      entity->AddComponent<RenderComponent<RenderType>>();
      EnactAnimationActionSystem::PlayAnimation(entity->GetID(), params.name, params.isLooped, params.speed, true, !params.horizontalFlip);
    }
    else if (instruction.first == "RenderComponent")
    {
      entity->AddComponent<RenderComponent<RenderType>>();
    }
    else if (instruction.first == "RenderProperties")
    {
      ComponentInitParams<RenderProperties> params;
      params.r = instruction.second["color_r"].asUInt();
      params.g = instruction.second["color_g"].asUInt();
      params.b = instruction.second["color_b"].asUInt();
      params.a = instruction.second["color_a"].asUInt();
      entity->AddComponent<RenderProperties>();
      RenderProperties* props = entity->GetComponent<RenderProperties>();
      props->horizontalFlip = !creatorState->onLeftSide;
      props->SetDisplayColor(params.r, params.g, params.b, params.a);
    }
    else if (instruction.first == "Rigidbody")
    {
      ComponentInitParams<Rigidbody> params;
      params.velocity = Vector2<float>(instruction.second["initVelocityX"].asFloat(), instruction.second["initVelocityY"].asFloat());
      if (!creatorState->onLeftSide)
        params.velocity.x = -params.velocity.x;

      bool useGravity = instruction.second["useGravity"].asBool();
      if (useGravity)
      {
        entity->AddComponent<Gravity>(ComponentInitParams<Gravity>{ GlobalVars::Gravity });
      }
      entity->AddComponent<Rigidbody>(params);
    }
    else if (instruction.first == "DynamicCollider")
    {
      ComponentInitParams<DynamicCollider> params;
      params.size = Vector2<float>(instruction.second["sizex"].asFloat(), instruction.second["sizey"].asFloat());
      entity->AddComponent<DynamicCollider>(params);
      moveableColliders.push_back(entity->GetComponent<DynamicCollider>());

      entityWidth = params.size.x;
    }
    else if (instruction.first == "Hurtbox")
    {
      ComponentInitParams<Hurtbox> params;
      params.size = Vector2<double>(instruction.second["x"].asDouble(), instruction.second["y"].asDouble());
      entity->AddComponent<Hurtbox>(params);
      moveableColliders.push_back(entity->GetComponent<Hurtbox>());
    }
    else if (instruction.first == "Hitbox")
    {
      ComponentInitParams<Hitbox> params;
      params.size = Vector2<double>(instruction.second["x"].asFloat(), instruction.second["y"].asFloat());
      params.hData.framesInStunBlock = instruction.second["stunFramesBlock"].asInt();
      params.hData.framesInStunHit = instruction.second["stunFramesHit"].asInt();
      params.hData.knockback = Vector2<float>(instruction.second["knockbackx"].asFloat(), instruction.second["knockbacky"].asFloat());
      params.hData.damage = instruction.second["damage"].asInt();
      params.travelWithTransform = instruction.second["follow"].asBool();
      params.destroyOnHit = instruction.second["destroyOnHit"].asBool();

      entity->AddComponent<Hitbox>(params);
      moveableColliders.push_back(entity->GetComponent<Hitbox>());
    }
    else if (instruction.first == "StateComponent")
    {
      entity->AddComponent<StateComponent>();
    }
  }

  // anything created by an entity will be automatically assigned to its team
  entity->AddComponent<TeamComponent>();
  entity->GetComponent<TeamComponent>()->team = GameManager::Get().GetEntityByID(creatorID)->GetComponent<TeamComponent>()->team;

  // set the scale
  entity->SetScale(scale);

  if (auto transform = entity->GetComponent<Transform>())
  {
    for (RectColliderD* collider : moveableColliders)
      collider->MoveToTransform(*transform);
  }
}

//______________________________________________________________________________
void EntityCreationData::Load(const Json::Value& json)
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

//______________________________________________________________________________
void EntityCreationData::DisplayInEditor()
{
}

//______________________________________________________________________________
void EventData::Load(const Json::Value& json)
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
    movement = Vector2<float>(json["movement"]["x"].asFloat(), json["movement"]["y"].asFloat());
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

//______________________________________________________________________________
void EventData::Write(Json::Value& json) const
{
  bool written = false;
  if (movement.x != 0 || movement.y != 0)
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

//______________________________________________________________________________
void EventData::DisplayInEditor()
{
  ImGui::InputFloat2("Movement", (float*)&movement, 2);
}

//______________________________________________________________________________
void ActionAsset::Load(const Json::Value& json)
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

//______________________________________________________________________________
void ActionAsset::Write(Json::Value& json) const
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

//______________________________________________________________________________
void ActionAsset::DisplayInEditor()
{
  frameData.DisplayInEditor();
  for (int i = 0; i < eventData.size(); i++)
  {
    std::string label = "Event Data for Frame " + std::to_string(i);
    if (ImGui::CollapsingHeader(label.c_str()))
    {
      eventData[i].DisplayInEditor();
    }
  }
}