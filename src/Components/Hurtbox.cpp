#include "Components/Hurtbox.h"
#include "Entity.h"

Hurtbox::Hurtbox(std::shared_ptr<Entity> owner) : RectColliderD(owner), DebugItem("Hurtbox") {}

void Hurtbox::OnDebug()
{
  int entityId = _owner->GetID();
  std::string pName = "P" + std::to_string(entityId);
  if(ImGui::CollapsingHeader(pName.c_str()))
    ImGui::Checkbox("Show Hurtbox", &_drawDebug);
}
