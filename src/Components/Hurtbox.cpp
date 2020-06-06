#include "Components/Collider.h"
#include "DebugGUI/GUIController.h"
#include "Entity.h"

Hurtbox::Hurtbox(std::shared_ptr<Entity> owner) : RectColliderD(owner)
{
  std::function<void()> func = [this]()
  {
    ImGui::BeginGroup();
    ImGui::Text("Player %d State Component", _owner->GetID());
    ImGui::Text("HP = %d", hp);
    ImGui::EndGroup();
  };

  debugID = GUIController::Get().AddImguiWindowFunction("State Components", func);
}

Hurtbox::~Hurtbox()
{
  if(debugID > 0)
    GUIController::Get().RemoveImguiWindowFunction("State Components", debugID);
}
