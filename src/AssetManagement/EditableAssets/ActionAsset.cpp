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
#include "AssetLibraryImpl.h"

//______________________________________________________________________________
template <> void AssetLoaderFn::OnLoad(ActionAsset& asset) {}

//______________________________________________________________________________
template <> ImVec2 AssetLoaderFn::GetDisplaySize<ActionAsset>()
{
  return ImVec2(500, 6 * fieldHeight);
}

//______________________________________________________________________________
bool ProjectileData::IsEmpty() const
{
  return size.x == 0 && size.y == 0 && animName.empty() && damage == 0
      && velocity.x == 0 && velocity.y == 0 && hitboxSize.x == 0 && hitboxSize.y == 0;
}

//______________________________________________________________________________
void ProjectileData::AddComponents(EntityID creatorID, const Transform* creator, const StateComponent* creatorState, std::shared_ptr<Entity> entity) const
{
  std::vector<RectColliderD*> moveableColliders;

  // Transform
  if (size.x > 0 || size.y > 0)
  {
    ComponentInitParams<Transform> params;
    params.scale = scale;
    params.size = size;
    params.position = position * creator->scale;

    if (relativeToCreator)
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

    entity->AddComponent<RenderProperties>();
    entity->GetComponent<RenderProperties>()->rectTransform = params.size;
  }

  // Animator
  if (!animName.empty())
  {
    ComponentInitParams<Animator> params;
    params.collectionID = GAnimArchive.GetCollectionID(animCollection);
    params.isLooped = animLooped;
    params.name = animName;
    params.horizontalFlip = !creatorState->onLeftSide;
    params.speed = 1.0f;

    entity->AddComponent<Animator>(params);
    entity->AddComponent<RenderProperties>();
    entity->AddComponent<RenderComponent<RenderType>>();
    EnactAnimationActionSystem::PlayAnimation(entity->GetID(), params.name, params.isLooped, params.speed, true, !params.horizontalFlip);
  }

  // Rigidbody
  if (velocity.x != 0 || velocity.y != 0 || useGravity)
  {
    ComponentInitParams<Rigidbody> rbParams;
    rbParams.velocity = velocity;
    if (!creatorState->onLeftSide)
      rbParams.velocity.x = -rbParams.velocity.x;

    if (useGravity)
      entity->AddComponent<Gravity>(ComponentInitParams<Gravity>{ GlobalVars::Gravity });

    entity->AddComponent<Rigidbody>(rbParams);
  }

  // DynamicCollider
  if (colliderSize.x > 0 || colliderSize.y > 0)
  {
    ComponentInitParams<DynamicCollider> params;
    params.size = colliderSize;
    entity->AddComponent<DynamicCollider>(params);
    moveableColliders.push_back(entity->GetComponent<DynamicCollider>());
  }

  // Hurtbox
  if (hasHurtbox && (hurtboxSize.x > 0 || hurtboxSize.y > 0))
  {
    ComponentInitParams<Hurtbox> params;
    params.size = hurtboxSize;
    entity->AddComponent<Hurtbox>(params);
    moveableColliders.push_back(entity->GetComponent<Hurtbox>());
  }

  // Hitbox
  if (hitboxSize.x > 0 || hitboxSize.y > 0)
  {
    ComponentInitParams<Hitbox> params;
    params.size = hitboxSize;
    params.hData.framesInStunBlock = stunFramesBlock;
    params.hData.framesInStunHit = stunFramesHit;
    params.hData.knockback = knockback;
    params.hData.damage = damage;
    params.travelWithTransform = followTransform;
    params.destroyOnHit = destroyOnHit;

    entity->AddComponent<Hitbox>(params);
    moveableColliders.push_back(entity->GetComponent<Hitbox>());
  }

  // StateComponent (always added for projectiles)
  entity->AddComponent<StateComponent>();

  // Team — inherited from creator
  entity->AddComponent<TeamComponent>();
  entity->GetComponent<TeamComponent>()->team = GameManager::Get().GetEntityByID(creatorID)->GetComponent<TeamComponent>()->team;

  // Scale
  entity->SetScale(scale);

  if (auto transform = entity->GetComponent<Transform>())
  {
    for (RectColliderD* collider : moveableColliders)
      collider->MoveToTransform(*transform);
  }
}

//______________________________________________________________________________
void ProjectileData::Load(const Json::Value& json)
{
  if (json.isMember("Transform"))
  {
    const auto& t = json["Transform"];
    position = Vector2<float>(t["x"].asFloat(), t["y"].asFloat());
    size = Vector2<float>(t["entitysizex"].asFloat(), t["entitysizey"].asFloat());
    scale = Vector2<float>(t["scalex"].asFloat(), t["scaley"].asFloat());
    relativeToCreator = t["relative"].asBool();
  }

  if (json.isMember("Animator"))
  {
    const auto& a = json["Animator"];
    animCollection = a["collection"].asString();
    animName = a["anim"].asString();
    animLooped = a["isLooped"].asBool();
  }

  if (json.isMember("Rigidbody"))
  {
    const auto& r = json["Rigidbody"];
    velocity = Vector2<float>(r["initVelocityX"].asFloat(), r["initVelocityY"].asFloat());
    useGravity = r["useGravity"].asBool();
  }

  if (json.isMember("DynamicCollider"))
  {
    const auto& c = json["DynamicCollider"];
    colliderSize = Vector2<float>(c["sizex"].asFloat(), c["sizey"].asFloat());
  }

  if (json.isMember("Hitbox"))
  {
    const auto& h = json["Hitbox"];
    hitboxSize = Vector2<double>(h["x"].asDouble(), h["y"].asDouble());
    damage = h["damage"].asInt();
    knockback = Vector2<float>(h["knockbackx"].asFloat(), h["knockbacky"].asFloat());
    stunFramesHit = h["stunFramesHit"].asInt();
    stunFramesBlock = h["stunFramesBlock"].asInt();
    followTransform = h["follow"].asBool();
    destroyOnHit = h["destroyOnHit"].asBool();
  }

  if (json.isMember("Hurtbox"))
  {
    const auto& h = json["Hurtbox"];
    hasHurtbox = true;
    hurtboxSize = Vector2<double>(h["x"].asDouble(), h["y"].asDouble());
  }
}

//______________________________________________________________________________
void ProjectileData::Write(Json::Value& json) const
{
  if (size.x > 0 || size.y > 0)
  {
    Json::Value& t = json["Transform"];
    t["x"] = position.x;
    t["y"] = position.y;
    t["entitysizex"] = size.x;
    t["entitysizey"] = size.y;
    t["scalex"] = scale.x;
    t["scaley"] = scale.y;
    t["relative"] = relativeToCreator;
  }

  if (!animName.empty())
  {
    Json::Value& a = json["Animator"];
    a["collection"] = animCollection;
    a["anim"] = animName;
    a["isLooped"] = animLooped;
  }

  // RenderComponent/RenderProperties — always present when there's an animation
  if (!animName.empty())
  {
    json["RenderComponent"] = Json::Value(Json::ValueType::objectValue);
    Json::Value& rp = json["RenderProperties"];
    rp["color_r"] = 255;
    rp["color_g"] = 255;
    rp["color_b"] = 255;
    rp["color_a"] = 255;
  }

  if (velocity.x != 0 || velocity.y != 0 || useGravity)
  {
    Json::Value& r = json["Rigidbody"];
    r["initVelocityX"] = velocity.x;
    r["initVelocityY"] = velocity.y;
    r["useGravity"] = useGravity;
  }

  if (colliderSize.x > 0 || colliderSize.y > 0)
  {
    Json::Value& c = json["DynamicCollider"];
    c["sizex"] = colliderSize.x;
    c["sizey"] = colliderSize.y;
  }

  if (hitboxSize.x > 0 || hitboxSize.y > 0)
  {
    Json::Value& h = json["Hitbox"];
    h["x"] = hitboxSize.x;
    h["y"] = hitboxSize.y;
    h["damage"] = damage;
    h["knockbackx"] = knockback.x;
    h["knockbacky"] = knockback.y;
    h["stunFramesHit"] = stunFramesHit;
    h["stunFramesBlock"] = stunFramesBlock;
    h["follow"] = followTransform;
    h["destroyOnHit"] = destroyOnHit;
  }

  if (hasHurtbox && (hurtboxSize.x > 0 || hurtboxSize.y > 0))
  {
    Json::Value& h = json["Hurtbox"];
    h["x"] = hurtboxSize.x;
    h["y"] = hurtboxSize.y;
  }

  json["StateComponent"] = Json::Value(Json::ValueType::objectValue);
}

//______________________________________________________________________________
void ProjectileData::DisplayInEditor()
{
  ImGui::Indent();

  if (ImGui::TreeNode("Spawn Transform"))
  {
    ImGui::InputFloat2("Position##spawn", &position.x);
    ImGui::InputFloat2("Entity Size##spawn", &size.x);
    ImGui::InputFloat2("Scale##spawn", &scale.x);
    ImGui::Checkbox("Relative to Creator", &relativeToCreator);
    ImGui::TreePop();
  }

  if (ImGui::TreeNode("Animation##spawn"))
  {
    char collBuf[128] = {};
    strncpy(collBuf, animCollection.c_str(), sizeof(collBuf) - 1);
    if (ImGui::InputText("Collection", collBuf, sizeof(collBuf)))
      animCollection = collBuf;

    char animBuf[128] = {};
    strncpy(animBuf, animName.c_str(), sizeof(animBuf) - 1);
    if (ImGui::InputText("Animation Name", animBuf, sizeof(animBuf)))
      animName = animBuf;

    ImGui::Checkbox("Looped", &animLooped);
    ImGui::TreePop();
  }

  if (ImGui::TreeNode("Physics##spawn"))
  {
    ImGui::InputFloat2("Velocity", &velocity.x);
    ImGui::Checkbox("Use Gravity", &useGravity);
    ImGui::TreePop();
  }

  if (ImGui::TreeNode("Collider##spawn"))
  {
    ImGui::InputFloat2("Collider Size", &colliderSize.x);
    ImGui::TreePop();
  }

  if (ImGui::TreeNode("Hitbox##spawn"))
  {
    ImGui::InputDouble("Width##hb", &hitboxSize.x);
    ImGui::InputDouble("Height##hb", &hitboxSize.y);
    ImGui::InputInt("Damage##spawn", &damage);
    ImGui::InputFloat2("Knockback##spawn", &knockback.x);
    ImGui::InputInt("Stun on Hit", &stunFramesHit);
    ImGui::InputInt("Stun on Block", &stunFramesBlock);
    ImGui::Checkbox("Follow Transform", &followTransform);
    ImGui::Checkbox("Destroy on Hit", &destroyOnHit);
    ImGui::TreePop();
  }

  if (ImGui::TreeNode("Hurtbox##spawn"))
  {
    ImGui::Checkbox("Has Hurtbox", &hasHurtbox);
    if (hasHurtbox)
    {
      ImGui::InputDouble("Width##hurt", &hurtboxSize.x);
      ImGui::InputDouble("Height##hurt", &hurtboxSize.y);
    }
    ImGui::TreePop();
  }

  ImGui::Unindent();
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
  if (!create.IsEmpty())
  {
    Json::Value createJson(Json::ValueType::objectValue);
    create.Write(createJson);
    json["createentity"] = createJson;
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
  ImGui::InputFloat2("Movement", (float*)&movement, "%.2f");
  if (hitbox.Area() > 0)
  {
    ImGui::SameLine();
    if (ImGui::Button("Delete Hitbox"))
      hitbox = Rect<double>(0, 0, 0, 0);
  }

  // Entity spawn section
  if (create.IsEmpty())
  {
    if (ImGui::Button("+ Add Entity Spawn"))
    {
      // Initialize with sensible defaults for a projectile
      create.size = Vector2<float>(100.f, 100.f);
      create.scale = Vector2<float>(1.f, 1.f);
      create.relativeToCreator = true;
      create.followTransform = true;
      create.destroyOnHit = true;
      isActive = true;
    }
  }
  else
  {
    if (ImGui::CollapsingHeader("Entity Spawn"))
    {
      create.DisplayInEditor();

      ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f, 0.15f, 0.15f, 1.0f));
      if (ImGui::Button("Remove Entity Spawn"))
        create = ProjectileData();
      ImGui::PopStyleColor();
    }
  }
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
  int deleteIdx = -1;
  for (int i = 0; i < eventData.size(); i++)
  {
    std::string deleteLabel = "Delete Data " + std::to_string(i);
    if (ImGui::Button(deleteLabel.c_str()))
    {
      deleteIdx = i;
    }
    ImVec2 btnSize = ImGui::GetItemRectSize();
    ImGui::SameLine();

    std::string label = "Event Data for Frame " + std::to_string(i);
    if (ImGui::CollapsingHeader(label.c_str()))
    {
      ImGui::Dummy(ImVec2(btnSize.x, 0));
      eventData[i].DisplayInEditor();
    }
  }

  if (deleteIdx >= 0)
    eventData.erase(eventData.begin() + deleteIdx);
}