#pragma once
#include "IJsonLoadable.h"
#include "Core/Utility/JsonFile.h"

#include "../imgui/imgui.h"

#include <string>
#include <unordered_map>

struct AssetLoaderFn
{
  template <typename T> static void OnLoad(T& asset);
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
      ImGui::Text("%s", item.first.c_str());
      item.second.DisplayInEditor();
      ImGui::EndChild();
    }
    counter = 0;
  }

private:
  static std::unordered_map<std::string, T> _library;

};

//______________________________________________________________________________
template <typename T>
std::unordered_map<std::string, T> AssetLibrary<T>::_library;
