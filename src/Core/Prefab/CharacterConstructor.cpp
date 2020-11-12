#include "Core/Prefab/CharacterConstructor.h"

#include "Managers/ResourceManager.h"
#include "Managers/GameManagement.h"
#include "Managers/AnimationCollectionManager.h"

#include "Components/Animator.h"
#include "Components/RenderComponent.h"
#include "Components/UIComponents.h"
#include "Components/Rigidbody.h"
#include "Components/Actors/GameActor.h"
#include "Components/Hurtbox.h"
#include "Components/MetaGameComponents.h"
#include "Components/Input.h"

#include "Systems/TimerSystem/TimerContainer.h"
#include "Systems/DestroyEntitiesSystem.h"
#include "Systems/TimerSystem/TimerSystem.h"

//______________________________________________________________________________
void CharacterConstructor::InitSpatialComponents(std::shared_ptr<Entity> player, const std::string& character, Vector2<float> position)
{
  Vector2<int> textureSize = ResourceManager::Get().GetTextureWidthAndHeight("spritesheets\\ryu.png");
  Vector2<double> entitySize(m_characterWidth, m_characterHeight);
  position.y = static_cast<float>(m_nativeHeight) - static_cast<float>(entitySize.y);

  // quick hack to make the debug stuff for attacks work... need to remove eventually
  player->RemoveComponents<Transform, Animator>();

  player->AddComponents<Transform, GameInputComponent, Animator, RenderComponent<RenderType>, RenderProperties, Rigidbody, Gravity, GameActor, DynamicCollider, Hurtbox, StateComponent, TeamComponent>();

  player->GetComponent<Gravity>()->force = GlobalVars::Gravity;
  player->GetComponent<Animator>()->animCollectionID = GAnimArchive.GetCollectionID(character);

  player->GetComponent<Transform>()->SetWidthAndHeight(entitySize.x, entitySize.y);
  auto rp = player->GetComponent<RenderProperties>();

  player->GetComponent<DynamicCollider>()->Init(Vector2<double>::Zero, entitySize);
  player->GetComponent<Hurtbox>()->Init(Vector2<double>::Zero, entitySize);

  const Vector2<float> scale(1.4f, 1.7f);
  player->SetScale(scale);
  player->GetComponent<Transform>()->position = position;

  player->GetComponent<DynamicCollider>()->MoveToTransform(*player->GetComponent<Transform>());
  player->GetComponent<Hurtbox>()->MoveToTransform(*player->GetComponent<Transform>());

  // sets this as the player entity to distinguish between fireballs (which are on the same team) from the player
  player->GetComponent<TeamComponent>()->playerEntity = true;

  //add offset of corner to center of render space
  rp->rectTransform = entitySize;
}

//______________________________________________________________________________
std::shared_ptr<Entity> CharacterConstructor::InitUIComponents(std::shared_ptr<Entity> player)
{
  const Vector2<float> healthBarOffset = { 30.0f, 20.0f };
  const Vector2<int> lifeBarSize = { 200, 25 };
  const Vector2<int> margin = { 2, 2 };

  player->AddComponents<UIContainer, TimerContainer>();
  auto playerUIContainerComponent = player->GetComponent<UIContainer>();

  // set up outline first
  auto healthbarOutline = GameManager::Get().CreateEntity<UITransform, RenderProperties>();
  healthbarOutline->GetComponent<UITransform>()->rect = Rect<float>(0, 0, lifeBarSize.x + 2 * margin.x, lifeBarSize.y + 2 * margin.y);
  healthbarOutline->GetComponent<UITransform>()->position = healthBarOffset;
  healthbarOutline->AddComponent<UIRectangleRenderComponent>();

  // now create a new entity for the fill (which will be automatically destroyed on scene end)
  auto healthbarFill = GameManager::Get().CreateEntity<UITransform, RenderProperties, DestroyOnSceneEnd>();
  healthbarFill->GetComponent<UITransform>()->rect = Rect<float>(0, 0, lifeBarSize.x, lifeBarSize.y);
  healthbarFill->GetComponent<UITransform>()->position = margin;
  healthbarFill->GetComponent<UITransform>()->anchor = UIAnchor::TL;
  healthbarFill->GetComponent<UITransform>()->parent = healthbarOutline->GetComponent<UITransform>();

  healthbarFill->GetComponent<RenderProperties>()->SetDisplayColor(255, 0, 0);

  healthbarFill->AddComponent<UIRectangleRenderComponent>();
  healthbarFill->GetComponent<UIRectangleRenderComponent>()->isFilled = true;

  playerUIContainerComponent->uiUpdaters.push_back(
    UIContainer::Updater{ healthbarFill, [lifeBarSize](std::shared_ptr<Entity> entity, const StateComponent* lastState, const StateComponent* info)
    {
      float p = (float)info->hp / 100.0f;
      entity->GetComponent<UIRectangleRenderComponent>()->shownSize.w = (float)lifeBarSize.x * p;
    } });

  // add the combo hit counter entity
  // create the counter text without render properties so that it wont be visible
  auto comboTextEntity = GameManager::Get().CreateEntity<UITransform, TextRenderer, TimerContainer, DestroyOnSceneEnd>();
  comboTextEntity->GetComponent<TextRenderer>()->SetFont(ResourceManager::Get().GetFontWriter("fonts\\Eurostile.ttf", 36));

  // set parent transform and offset
  comboTextEntity->GetComponent<UITransform>()->parent = healthbarOutline->GetComponent<UITransform>();
  comboTextEntity->GetComponent<UITransform>()->anchor = UIAnchor::BL;
  comboTextEntity->GetComponent<UITransform>()->position = Vector2<float>(5.0f, 20.0f);

  // set the ui data transfer callback 
  playerUIContainerComponent->uiUpdaters.push_back(
  UIContainer::Updater{ comboTextEntity, [](std::shared_ptr<Entity> entity, const StateComponent* lastState, const StateComponent* newState)
  {
    if (lastState->hitting && newState->comboCounter > 1)
    {
      auto& activeTimers = entity->GetComponent<TimerContainer>()->timings;
      if (!activeTimers.empty())
      {
        for (auto timer : activeTimers)
          timer->Cancel();
      }

      const int comboTextVisibleFrames = 35;
      // replace active timer with new one that will remove render properties to hide the text
      std::shared_ptr<ActionTimer> endComboText = std::shared_ptr<ActionTimer>(new SimpleActionTimer([entity]()
        {
          entity->RemoveComponent<RenderProperties>();
        }, comboTextVisibleFrames));
      entity->GetComponent<TimerContainer>()->timings.push_back(endComboText);

      // ensure the combo text is visible
      entity->AddComponent<RenderProperties>();

      std::string comboText = "Combo: " + std::to_string(newState->comboCounter);
      entity->GetComponent<TextRenderer>()->SetText(comboText, TextAlignment::Left);
    }
  } });

  // anchor the combo text to the outline
  comboTextEntity->GetComponent<UITransform>()->parent = healthbarOutline->GetComponent<UITransform>();

  // set this as the parent for all other ui components
  return healthbarOutline;
}
