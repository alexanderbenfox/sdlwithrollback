#include "Components/Hurtbox.h"
#include "Entity.h"

Hurtbox::Hurtbox(std::shared_ptr<Entity> owner) : RectColliderD(owner), DebugItem("Hurtbox") {}

void Hurtbox::OnDebug()
{
  ImGui::Text("HP = %d", hp);
}
