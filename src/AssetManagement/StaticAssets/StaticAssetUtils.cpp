#include "AssetManagement/StaticAssets/StaticAssetUtils.h"
#include "GameManagement.h"
#include "DebugGUI/GUIController.h"
#include "Utils.h"

#include <fstream>
#include <json/json.h>

std::string StaticAssetUtils::jsonWriteOutLocation = StringUtils::CorrectPath("\\moveset_data\\ryu_moves.json");
bool StaticAssetUtils::loadFromFile = true;


void StaticAssetUtils::LoadNormal(const std::string& name, const AnimationInfo& data, AnimationCollection& collection)
{
  collection.RegisterAnimation(name, SpriteSheet(data.sheet.src.c_str(), data.sheet.rows, data.sheet.columns), data.startIndexOnSheet, data.frames, data.anchor);
}

void StaticAssetUtils::LoadAttackAnim(const std::string& name, const AttackAnimationData& data, AnimationCollection& collection)
{
  if (StaticAssetUtils::loadFromFile)
  {
    collection.RegisterAnimation(name, data.loadingInfo.sheet, data.loadingInfo.startIndexOnSheet, data.loadingInfo.frames, data.loadingInfo.anchor);
    collection.SetAnimationEvents(name, data.eventData, data.frameData);
  }
  else
  {
    std::string hitboxSheet = data.loadingInfo.sheet.src;
    hitboxSheet.erase(hitboxSheet.end() - 4, hitboxSheet.end());
    hitboxSheet += "_hitboxes.png";

    SpriteSheet sheet(data.loadingInfo.sheet.src.c_str(), data.loadingInfo.sheet.rows, data.loadingInfo.sheet.columns);
    AttackAnimationData copy = data;
    copy.loadingInfo.sheet = sheet;

    collection.RegisterAnimation(name, sheet, data.loadingInfo.startIndexOnSheet, data.loadingInfo.frames, data.loadingInfo.anchor);
    collection.SetAnimationEvents(name, CreateEventDataFromHitboxSheet(hitboxSheet.c_str(), copy), data.frameData);
  }

}

void StaticAssetUtils::LoadAnimations(std::unordered_map<std::string, AnimationInfo>& normalAnimations, std::unordered_map<std::string, AttackAnimationData>& attackAnimations,
  AnimationCollection& collection)
{
  // clear collection before assigning animations
  collection.Clear();

  //hack cause i suck
  auto it = normalAnimations.find("Idle");
  collection.RegisterAnimation("Idle", SpriteSheet(it->second.sheet.src.c_str(), it->second.sheet.rows, it->second.sheet.columns), it->second.startIndexOnSheet, it->second.frames, it->second.anchor);

  for (auto& anim : normalAnimations)
  {
    LoadNormal(anim.first, anim.second, collection);
  }

  if (StaticAssetUtils::loadFromFile)
  {
    // load from file and update attack animations for debug
    LoadCollectionFromJson(collection, attackAnimations, ResourceManager::Get().GetResourcePath() + StringUtils::CorrectPath("\\moveset_data\\ryu_spritesheets.json"), ResourceManager::Get().GetResourcePath() + jsonWriteOutLocation);
  }
  else
  {
    for (auto& anim : attackAnimations)
    {
      LoadAttackAnim(anim.first, anim.second, collection);
    }
  }
}

void StaticAssetUtils::CreateAnimationDebug(std::unordered_map<std::string, AnimationInfo>& normalAnimations, std::unordered_map<std::string, AttackAnimationData>& attackAnimations,
  AnimationCollection& collection)
{
  const size_t fileCharacterSize = 256;
  jsonWriteOutLocation.resize(fileCharacterSize);

  std::function<void()> f = [fileCharacterSize]() {
    ImGui::BeginGroup();
    ImGui::InputText("Write out Json location", StaticAssetUtils::jsonWriteOutLocation.data(), fileCharacterSize);
    if (ImGui::Button("Reload Animation Data", ImVec2(100, 20)))
    {
      GameManager::Get().TriggerOnSceneChange([]() { RyuConfig::Reload(); });
      GameManager::Get().RequestSceneChange(SceneType::BATTLE);
    }
    ImGui::EndGroup();
  };

  GUIController::Get().AddImguiWindowFunction("Ryu Attack Animations", f);

  for (auto& anim : attackAnimations)
  {
    std::string animName = anim.first;
    AttackAnimationData& data = anim.second;

    std::function<void()> ModFrameData = [animName, &data, &collection]()
    {
      if (ImGui::CollapsingHeader(animName.c_str()))
      {
        ImGui::BeginGroup();
        ImGui::InputInt("Start Up Frames", &data.frameData.startUp);
        ImGui::InputInt("Active Frames", &data.frameData.active);
        ImGui::InputInt("Recovery Frames", &data.frameData.recover);
        ImGui::EndGroup();

        ImGui::BeginGroup();
        ImGui::InputInt("On Hit Frame Advantage", &data.frameData.onHitAdvantage);
        ImGui::InputInt("On Block Frame Advantage", &data.frameData.onBlockAdvantage);
        ImGui::EndGroup();

        ImGui::BeginGroup();
        ImGui::InputInt("Damage", &data.frameData.damage);
        ImGui::InputFloat2("Knockback Vector", &data.frameData.knockback.x, 1);
        ImGui::InputInt("Hit stop", &data.frameData.hitstop);
        ImGui::EndGroup();

        static int frame = 0;
        Animation::ImGuiDisplayParams imParams = collection.GetAnimation(animName)->GetUVCoordsForFrame(128, frame);
        ImGui::Image((void*)(intptr_t)imParams.ptr, ImVec2(imParams.displaySize.x, imParams.displaySize.y),
          ImVec2(imParams.uv0.x, imParams.uv0.y), ImVec2(imParams.uv1.x, imParams.uv1.y));

        int nFrames = collection.GetAnimation(animName)->GetFrameCount();
        ImGui::BeginGroup();
        if (ImGui::Button("Back"))
        {
          frame = frame == 0 ? nFrames - 1 : frame - 1;
        }
        ImGui::SameLine();
        if (ImGui::Button("Forward"))
        {
          frame = (frame + 1) % nFrames;
        }
        ImGui::SameLine();
        ImGui::Text("%d", frame);
        ImGui::EndGroup();

        if (ImGui::Button("Set Frame Data"))
        {
          // generate event list based on new frame data
          collection.SetAnimationEvents(animName, collection.GetAnimation(animName)->animationEvents, data.frameData);
        }

        ImGui::Text("Write to json");
        if (ImGui::Button("Write Out Data"))
        {
          std::fstream dataFile;
          dataFile.open(ResourceManager::Get().GetResourcePath() + jsonWriteOutLocation, std::fstream::in | std::fstream::out);
          Json::Value moveListObj;

          if (dataFile.is_open())
          {
            dataFile >> moveListObj;

            // remove existing entry for this move if it exists
            if (!moveListObj.isNull())
            {
              moveListObj.removeMember(animName);
            }
            dataFile.close();
          }

          auto& item = moveListObj[animName];

          if (collection.GetEventList(animName) != nullptr)
          {
            Animation* a = collection.GetAnimation(animName);
            data.eventData = a->animationEvents;
          }

          // append new data to existing json
          data.Write(item);

          dataFile.open(ResourceManager::Get().GetResourcePath() + jsonWriteOutLocation, std::fstream::out);
          dataFile << moveListObj.toStyledString() << std::endl;
          dataFile.close();
        }
      }
    };
    GUIController::Get().AddImguiWindowFunction("Ryu Attack Animations", ModFrameData);
  }
}

//______________________________________________________________________________
void StaticAssetUtils::LoadCollectionFromJson(AnimationCollection& collection, std::unordered_map<std::string, AttackAnimationData>& attackAnimations, const std::string& spriteSheetJsonLocation, const std::string& movesetJsonLocation)
{
  std::fstream spriteSheetFile;
  spriteSheetFile.open(spriteSheetJsonLocation, std::ios::in);
  Json::Value obj;

  try
  {
    spriteSheetFile >> obj;
  }
  catch (const Json::RuntimeError& err)
  {
    std::cout << "Error parsing Spritesheet file " << spriteSheetJsonLocation << "\nReturned with error: " << err.what() << "\n";
  }
  
  if (obj.isNull())
    return;

  spriteSheetFile.close();

  std::map<std::string, SpriteSheet> loadedSpriteSheets;
  for (auto& item : obj)
  {
    SpriteSheet sheet;
    sheet.Load(item);
    loadedSpriteSheets.emplace(sheet.src, sheet);
  }

  Json::Value movesetObj;
  std::fstream movesetFile;
  movesetFile.open(movesetJsonLocation, std::ios::in);

  try
  {
    movesetFile >> movesetObj;
  }
  catch (const Json::RuntimeError& err)
  {
    std::cout << "Error parsing Move List file " << movesetJsonLocation << "\nReturned with error: " << err.what() << "\n";
  }

  if (movesetObj.isNull())
    return;

  movesetFile.close();

  for (auto& member : movesetObj.getMemberNames())
  {
    std::string animName = member;
    Json::Value& item = movesetObj[animName];

    if (item.isMember("framedata"))
    {
      AttackAnimationData& data = attackAnimations[animName];
      data = AttackAnimationData();
      data.Load(item);
      data.loadingInfo.sheet = loadedSpriteSheets[data.loadingInfo.sheetLocation];
      LoadAttackAnim(animName, data, collection);
    }
    else
    {
      AnimationInfo info;
      info.Load(item);
      collection.RegisterAnimation(animName, loadedSpriteSheets[info.sheetLocation], info.startIndexOnSheet, info.frames, info.anchor);
    }
  }
}

//______________________________________________________________________________
std::vector<AnimationActionEventData> StaticAssetUtils::CreateEventDataFromHitboxSheet(const char* hitboxesSheet, const AttackAnimationData& data)
{
  std::vector<Rect<double>> hitboxes = GetHitboxesFromFile(hitboxesSheet, data.loadingInfo);
  std::vector<AnimationActionEventData> eventData;
  eventData.resize(hitboxes.size());
  for (int i = 0; i < hitboxes.size(); i++)
  {
    eventData[i].hitbox = hitboxes[i];
    eventData[i].isActive = hitboxes[i].Area() > 0;
  }
  return eventData;
}

//______________________________________________________________________________
std::vector<Rect<double>> StaticAssetUtils::GetHitboxesFromFile(const char* hitboxesSheet, const AnimationInfo& animInfo)
{
  std::vector<Rect<double>> rects;
  rects.reserve(std::size_t(animInfo.frames + 1));

  std::string hitBoxFile = hitboxesSheet;
#ifndef _WIN32
  auto split = StringUtils::Split(hitBoxFile, '\\');
  if (split.size() > 1)
    hitBoxFile = StringUtils::Connect(split.begin(), split.end(), '/');
#endif
  Resource<SDL_Texture> hitboxes = Resource<SDL_Texture>(ResourceManager::Get().GetResourcePath() + hitBoxFile);
  hitboxes.Load();
  if (hitboxes.IsLoaded())
  {
    for (int i = 0; i < animInfo.frames; i++)
    {
      int x = (animInfo.startIndexOnSheet + i) % animInfo.sheet.columns;
      int y = (animInfo.startIndexOnSheet + i) / animInfo.sheet.columns;

      Rect<double> hitbox = ResourceManager::FindRect(hitboxes, animInfo.sheet.frameSize, Vector2<int>(x * animInfo.sheet.frameSize.x, y * animInfo.sheet.frameSize.y));
      rects.push_back(hitbox);
    }
  }
  return rects;
}
