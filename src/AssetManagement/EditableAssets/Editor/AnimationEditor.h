#pragma once
#include <string>
#include <unordered_map>
#include <memory>

#include "AssetManagement/EditableAssets/CharacterConfiguration.h"

class CharacterEditor
{
public:
  static CharacterEditor& Get()
  {
    static CharacterEditor instance;
    return instance;
  }

  //! Register editor windows for all discovered characters + "Add New" button
  void Initialize();

private:
  CharacterEditor() = default;

  void RegisterCharacterEditorWindow(const std::string& name);

  //! Lazily-constructed editor instances (created when tab is first opened)
  std::unordered_map<std::string, std::unique_ptr<CharacterConfiguration>> _editors;

};
