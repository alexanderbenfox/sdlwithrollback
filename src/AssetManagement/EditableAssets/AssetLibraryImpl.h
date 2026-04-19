#pragma once
#include "AssetLibrary.h"
#include "Core/Utility/FilePath.h"
#include "Core/Utility/JsonFile.h"
#include "DebugGUI/GUIController.h"

//! Estimated typical field height in ImGui
const int fieldHeight = 25;

//______________________________________________________________________________
struct AssetLoaderFn
{
  template <typename T> static void OnLoad(T& asset);
  template <typename T> static ImVec2 GetDisplaySize();
  template <typename T> static std::string GUIHeaderLabel;
  template <typename T> static std::string GUIItemLabel;
};

// Inline variable template specializations (C++17)
struct SpriteSheet;
class AnimationAsset;
class ActionAsset;
template <> inline std::string AssetLoaderFn::GUIHeaderLabel<SpriteSheet> = "Sprite Sheets";
template <> inline std::string AssetLoaderFn::GUIItemLabel<SpriteSheet> = "Sprite Sheet";
template <> inline std::string AssetLoaderFn::GUIHeaderLabel<AnimationAsset> = "Animations";
template <> inline std::string AssetLoaderFn::GUIItemLabel<AnimationAsset> = "Animation";
template <> inline std::string AssetLoaderFn::GUIHeaderLabel<ActionAsset> = "Actions";
template <> inline std::string AssetLoaderFn::GUIItemLabel<ActionAsset> = "Action";

//______________________________________________________________________________
template <typename T>
inline void AssetLibrary<T>::LoadJsonData(const JsonFile& file)
{
  file.LoadContentsIntoMap(_library, &AssetLoaderFn::OnLoad);
}

//______________________________________________________________________________
template <typename T>
inline void AssetLibrary<T>::SaveDataJson(JsonFile& file) const
{
  file.SaveContentsIntoFile(_library);
}

//______________________________________________________________________________
template <typename T>
inline void AssetLibrary<T>::DisplayInGUI()
{
  std::string showCheckboxLabel = "Show " + AssetLoaderFn::GUIHeaderLabel<T> +" List";
  ImGui::Checkbox(showCheckboxLabel.c_str(), &_showItemList);

  std::vector<std::string> itemNames;
  if (_showItemList && ImGui::CollapsingHeader(AssetLoaderFn::GUIHeaderLabel<T>.c_str()))
  {
    static int counter = 0;
    std::pair<bool, std::string> deleteCall = std::make_pair(false, "");

    for (auto& item : _library)
    {
      itemNames.push_back(item.first);
      std::string name = "##item:" + std::to_string(counter++);
      ImGui::BeginChild(name.c_str(), AssetLoaderFn::GetDisplaySize<T>(), true);
      {
        ImGui::Text("%s", item.first.c_str());
        ImGui::SameLine();
        if (ImGui::Button("Rename"))
          OnRename(item.first);
        ImGui::SameLine();
        if (ImGui::Button("Copy"))
          OnCopy(item.first);
        ImGui::SameLine();
        if (ImGui::Button("Delete"))
          deleteCall = std::make_pair(true, item.first);
      }

      item.second.DisplayInEditor();
      ImGui::EndChild();
    }

    if (deleteCall.first)
      OnDelete(deleteCall.second);

    counter = 0;
  }
  else
  {
    for (auto& item : _library)
      itemNames.push_back(item.first);
  }

  ImGui::BeginGroup();
  ImGui::Spacing();
  std::string editItemTxt = "Edit " + AssetLoaderFn::GUIItemLabel<T> +": ";
  ImGui::Text("%s", editItemTxt.c_str());
  ImGui::SameLine();
  DropDown::DisplayList(itemNames, _dropdownSelection,
  [this]()
  {
    if (_library.find(_dropdownSelection) != _library.end())
    {
      std::string popupItem = _dropdownSelection;
      _dropdownSelection = "";

      GUIController::Get().CreatePopup("Edit " + AssetLoaderFn::GUIItemLabel<T>,
      [this, popupItem]()
      {
        _library[popupItem].DisplayInEditor();
      }, []() {});
    }
  });
  ImGui::Spacing();
  ImGui::EndGroup();

  std::string createNewItemTxt = "Create New " + AssetLoaderFn::GUIItemLabel<T>;
  if (ImGui::CollapsingHeader(createNewItemTxt.c_str()))
  {
    _newItemName.DisplayEditable("New Item Name");
    _newItem.DisplayInEditor();
    if (ImGui::Button("Add"))
      _library.insert(std::make_pair((std::string)_newItemName, _newItem));
  }
}

//______________________________________________________________________________
template <typename T>
inline void AssetLibrary<T>::DisplaySaveButton(const std::string& saveLocation)
{
  if (ImGui::Button("Save Data"))
  {
    FilePath path(saveLocation);
    JsonFile json(path.GetPath());
    json.SaveContentsIntoFile(_library);
  }
}

//______________________________________________________________________________
template <typename T>
inline void AssetLibrary<T>::OnRename(const std::string& item)
{
  static EditorString popupStr;
  popupStr = item;
  std::string label = "Rename " + AssetLoaderFn::GUIItemLabel<T>;
  GUIController::Get().CreatePopup(label,
    []()
    {
      popupStr.DisplayEditable("New Name");
    },
    [this, item]()
    {
      std::string newName = (std::string)popupStr;
      if (newName != item)
      {
        T cpy = _library[item];
        _library.erase(item);
        _library[newName] = cpy;
        if (_onRenameCallback)
          _onRenameCallback(item, newName);
      }
    }, 350, 4 * fieldHeight);
}

//______________________________________________________________________________
template <typename T>
inline void AssetLibrary<T>::OnCopy(const std::string& item)
{
  static EditorString copyStr;
  copyStr = item;
  std::string label = "Copy " + AssetLoaderFn::GUIItemLabel<T>;
  GUIController::Get().CreatePopup(label,
    []()
    {
      copyStr.DisplayEditable("New Name");
    },
    [this, item]()
    {
      if ((std::string)copyStr != item)
      {
        T cpy = _library[item];
        _library[(std::string)copyStr] = cpy;
      }
    }, 350, 4 * fieldHeight);
}
