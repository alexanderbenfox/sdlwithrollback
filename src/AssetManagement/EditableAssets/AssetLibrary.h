#pragma once
#include "IJsonLoadable.h"
#include "Core/Utility/FilePath.h"
#include "Core/Utility/JsonFile.h"
#include "DebugGUI/GUIController.h"
#include "DebugGUI/EditorString.h"

#include <string>
#include <unordered_map>

//! Estimated typical field height in ImGui
const int fieldHeight = 25;

//______________________________________________________________________________
struct AssetLoaderFn
{
  template <typename T> static void OnLoad(T& asset);
  template <typename T> static ImVec2 GetDisplaySize();
};

//______________________________________________________________________________
//! Static library for any of the json loadable types
template <typename T = IJsonLoadable>
class AssetLibrary
{
public:
  typedef std::unordered_map<std::string, T> LibType;

  void LoadJsonData(const JsonFile& file);
  void SaveDataJson(JsonFile& file) const;

  void DisplayInGUI();
  void DisplaySaveButton(const std::string& saveLocation);

  LibType const& GetLibrary() const { return _library; }
  T const& Get(const std::string& id);
  T& GetModifiable(const std::string& id);

private:
  void OnRename(const std::string& item);
  void OnCopy(const std::string& item);
  void OnDelete(const std::string& item);

  //!
  LibType _library;

  //! Used to create a new item in the library
  EditorString _newItemName;
  T _newItem;
  std::string _dropdownSelection = "";

};

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
  static int counter = 0;
  std::pair<bool, std::string> deleteCall = std::make_pair(false, "");
  std::vector<std::string> itemNames;

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

  if(deleteCall.first)
    OnDelete(deleteCall.second);

  counter = 0;

  ImGui::BeginGroup();
  ImGui::Spacing();
  ImGui::Text("Edit Item: ");
  ImGui::SameLine();
  DropDown::DisplayList(itemNames, _dropdownSelection,
  [this]()
  {
    if (_library.find(_dropdownSelection) != _library.end())
    {
      std::string popupItem = _dropdownSelection;
      _dropdownSelection = "";

      GUIController::Get().CreatePopup(
      [this, popupItem]()
      {
        _library[popupItem].DisplayInEditor();
      }, []() {});
    }
  });
  ImGui::Spacing();
  ImGui::EndGroup();

  if (ImGui::CollapsingHeader("Create New"))
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
inline T const& AssetLibrary<T>::Get(const std::string& id)
{
  return _library[id];
}

//______________________________________________________________________________
template <typename T>
inline T& AssetLibrary<T>::GetModifiable(const std::string& id)
{
  return _library[id];
}

//______________________________________________________________________________
template <typename T>
inline void AssetLibrary<T>::OnRename(const std::string& item)
{
  static EditorString popupStr;
  popupStr = item;
  GUIController::Get().CreatePopup([]()
    {
      popupStr.DisplayEditable("New Name");
    },
    [this, item]()
    {
      if ((std::string)popupStr != item)
      {
        T cpy = _library[item];
        _library.erase(item);
        _library[(std::string)popupStr] = cpy;
      }
    }, 350, 4 * fieldHeight);
}

//______________________________________________________________________________
template <typename T>
inline void AssetLibrary<T>::OnCopy(const std::string& item)
{
  static EditorString copyStr;
  copyStr = item;
  GUIController::Get().CreatePopup([]()
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

//______________________________________________________________________________
template <typename T>
inline void AssetLibrary<T>::OnDelete(const std::string& item)
{
  _library.erase(item);
}
