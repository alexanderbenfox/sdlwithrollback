#include "AssetManagement/StaticAssets/AnimationAssetData.h"
#include "AssetManagement/StaticAssets/CharacterConfig.h"
#include "Entity.h"

#include "Components/Transform.h"
#include "Components/Animator.h"
#include "Components/RenderComponent.h"
#include "Components/Rigidbody.h"
#include "Components/Hitbox.h"
#include "Components/Hurtbox.h"
#include "Components/StateComponent.h"

void EntityCreationData::AddComponents(Transform* creator, const StateComponent* creatorState, std::shared_ptr<Entity> entity) const
{
  Vector2<float> scale(1.0f, 1.0f);
  std::vector<RectColliderD*> moveableColliders;

  for (auto instruction : instructions)
  {
    if(instruction.first == "Transform")
    {
      ComponentInitParams<Transform> params;
      scale = params.scale = Vector2<float>(instruction.second["scalex"].asFloat(), instruction.second["scaley"].asFloat());
      params.size = Vector2<float>(instruction.second["entitysizex"].asFloat(), instruction.second["entitysizey"].asFloat());

      params.position = Vector2<float>(instruction.second["x"].asFloat(), instruction.second["y"].asFloat());

      if (instruction.second["relative"].asBool())
      {
        if (!creatorState->onLeftSide)
        {
          params.position.x = -params.position.x;
          params.position.x += creator->rect.Width();
          params.position.x -= (params.size.x * scale.x);
        }
        params.position.x += creator->position.x;
        params.position.y += creator->position.y;
      }
      
      params.scale = Vector2<float>(1.0f, 1.0f);

      entity->AddComponent<Transform>(params);
    }
    else if (instruction.first == "Animator")
    {
      ComponentInitParams<Animator> params;
      params.collection = &AnimCollectionsGetter::GetCollection(instruction.second["collection"].asString());
      params.isLooped = instruction.second["isLooped"].asBool();
      params.name = instruction.second["anim"].asString();

      params.horizontalFlip = !creatorState->onLeftSide;
      params.speed = 1.0f;

      entity->AddComponent<Animator>(params);

      entity->AddComponent<RenderComponent<RenderType>>();
      auto renderer = entity->GetComponent<RenderComponent<RenderType>>();

      Animation* anim = params.collection->GetAnimation(params.name);
      if (anim)
      {
        renderer->SetRenderResource(anim->GetSheetTexture<RenderType>());
        renderer->sourceRect = anim->GetFrameSrcRect(0);
      }
    }
    else if (instruction.first == "RenderComponent")
    {
      entity->AddComponent<RenderComponent<RenderType>>();
    }
    else if (instruction.first == "RenderProperties")
    {
      ComponentInitParams<RenderProperties> params;
      params.offsetFromCenter = Vector2<int>(instruction.second["offsetCenterX"].asInt(), instruction.second["offsetCenterY"].asInt());
      params.r = instruction.second["color_r"].asUInt();
      params.g = instruction.second["color_g"].asUInt();
      params.b = instruction.second["color_b"].asUInt();
      params.a = instruction.second["color_a"].asUInt();
      entity->AddComponent<RenderProperties>(params);

      entity->GetComponent<RenderProperties>()->horizontalFlip = !creatorState->onLeftSide;
    }
    else if (instruction.first == "Rigidbody")
    {
      ComponentInitParams<Rigidbody> params;
      params.velocity = Vector2<float>(instruction.second["initVelocityX"].asFloat(), instruction.second["initVelocityY"].asFloat());
      if (!creatorState->onLeftSide)
        params.velocity.x = -params.velocity.x;
      params.useGravity = instruction.second["useGravity"].asBool();
      entity->AddComponent<Rigidbody>(params);
    }
    else if (instruction.first == "DynamicCollider")
    {
      ComponentInitParams<DynamicCollider> params;
      params.size = Vector2<float>(instruction.second["sizex"].asFloat(), instruction.second["sizey"].asFloat());
      entity->AddComponent<DynamicCollider>(params);
      moveableColliders.push_back(entity->GetComponent<DynamicCollider>().get());
    }
    else if (instruction.first == "Hurtbox")
    {
      ComponentInitParams<Hurtbox> params;
      params.size = Vector2<double>(instruction.second["x"].asDouble(), instruction.second["y"].asDouble());
      entity->AddComponent<Hurtbox>(params);
      moveableColliders.push_back(entity->GetComponent<Hurtbox>().get());
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
      moveableColliders.push_back(entity->GetComponent<Hitbox>().get());
    }
    else if (instruction.first == "StateComponent")
    {
      entity->AddComponent<StateComponent>();
    }
  }

  // anything created by an entity will be automatically assigned to its team
  entity->AddComponent<TeamComponent>();
  entity->GetComponent<TeamComponent>()->team = creator->GetComponent<TeamComponent>()->team;

  // set the scale
  entity->SetScale(scale);

  if (auto transform = entity->GetComponent<Transform>())
  {
    for (RectColliderD* collider : moveableColliders)
      collider->MoveToTransform(*transform);
  }
}
