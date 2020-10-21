#include "AssetManagement/EditableAssets/FrameData.h"
#include "DebugGUI/GUIController.h"

void FrameData::DisplayEditableData()
{
  ImGui::BeginGroup();
  ImGui::InputInt("Start Up Frames", &startUp);
  ImGui::InputInt("Active Frames", &active);
  ImGui::InputInt("Recovery Frames", &recover);
  ImGui::EndGroup();

  ImGui::BeginGroup();
  ImGui::InputInt("On Hit Frame Advantage", &onHitAdvantage);
  ImGui::InputInt("On Block Frame Advantage", &onBlockAdvantage);
  ImGui::EndGroup();

  ImGui::BeginGroup();
  ImGui::InputInt("Damage", &damage);
  ImGui::InputFloat2("Knockback Vector", &knockback.x, 1);
  ImGui::InputInt("Hit stop", &hitstop);
  ImGui::EndGroup();

  ImGui::BeginGroup();
  ImGui::Text("Hit Type");
  const char* items[] = { "Low", "Mid", "High" };
  static const char* current_item = type == HitType::Low ? "Low" : type == HitType::Mid ? "Mid" : "High";
  auto func = [this](const std::string& i)
  {
    type = FromString(i);
  };
  DropDown::Show(current_item, items, 3, func);
  ImGui::EndGroup();
}

void FrameData::Load(const Json::Value& json)
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
    if (frameData["knockdown"])
    {
      knockdown = frameData["knockdown"].asBool();
    }
    if (frameData["isThrow"])
    {
      isThrow = frameData["isThrow"].asBool();
    }

    // hit type check
    if (frameData["hittype"])
    {
      auto ht = frameData["hittype"].asString();
      if (ht == "Low")
        type = HitType::Low;
      else if (ht == "Mid")
        type = HitType::Mid;
      else if (ht == "High")
        type = HitType::High;
    }
    else
    {
      type = HitType::Mid;
    }
  }
}

void FrameData::Write(Json::Value& json) const
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
    frameData["knockdown"] = knockdown;
    if (isThrow)
    {
      frameData["isThrow"] = true;
    }
    frameData["hittype"] = ToString(type);
  }
}
