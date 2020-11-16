#pragma once
#include "IJsonLoadable.h"
#include "Core/Utility/FilePath.h"
#include "Core/Utility/JsonFile.h"
#include "DebugGUI/GUIController.h"

#include <string>
#include <unordered_map>

struct AssetLoaderFn
{
  template <typename T> static void OnLoad(T& asset);
  template <typename T> static std::string SaveLocation();
};

//! Static library for any of the json loadable types
template <typename T = IJsonLoadable>
class AssetLibrary
{
public:

  static void LoadJsonData(const JsonFile& file)
  {
    file.LoadContentsIntoMap(_library, &AssetLoaderFn::OnLoad);
  }

  static T const& Get(const std::string& id)
  {
    return _library[id];
  }

  static void DisplayInGUI()
  {
    const int fieldHeight = 25;
    static int counter = 0;
    for (auto& item : _library)
    {
      std::string name = "##item:" + std::to_string(counter++);
      ImGui::BeginChild(name.c_str(), ImVec2(500, 6 * fieldHeight), true);

      {
        ImGui::Text("%s", item.first.c_str());
        ImGui::SameLine();
        if (ImGui::Button("Rename"))
          OnRename(item.first);
        ImGui::SameLine();
        if (ImGui::Button("Copy"))
          OnCopy(item.first);
      }
      
      item.second.DisplayInEditor();
      ImGui::EndChild();
    }
    counter = 0;

    if (ImGui::Button("Save Data"))
    {
      FilePath path(AssetLoaderFn::SaveLocation<T>());
      JsonFile json(path.GetPath());
      json.SaveContentsIntoFile(_library);
    }
  }

private:
  static void OnRename(const std::string& item)
  {
    static EditorString popupStr;
    popupStr = item;
    GUIController::Get().CreatePopup([]()
    {
      popupStr.DisplayEditable("New Name");
    },
    [item]()
    {
      if ((std::string)popupStr != item)
      {
        T cpy = _library[item];
        _library.erase(item);
        _library[(std::string)popupStr] = cpy;
      }
    });
  }

  static void OnCopy(const std::string& item)
  {
    static EditorString copyStr;
    copyStr = item;
    GUIController::Get().CreatePopup([]()
    {
        copyStr.DisplayEditable("New Name");
    },
    [item]()
    {
      if ((std::string)copyStr != item)
      {
        T cpy = _library[item];
        _library[(std::string)copyStr] = cpy;
      }
    });
  }

  static std::unordered_map<std::string, T> _library;

};

//______________________________________________________________________________
template <typename T>
std::unordered_map<std::string, T> AssetLibrary<T>::_library;
