#include "AnimationEditor.h"
#include "AssetManagement/EditableAssets/CharacterConfiguration.h"
#include "Core/Utility/FilePath.h"
#include "DebugGUI/GUIController.h"

#include "Managers/GameManagement.h"
#include "Managers/AnimationCollectionManager.h"

void CharacterEditor::Initialize()
{
  // Register editor windows for all characters discovered at startup
  for (const auto& name : AnimationCollectionManager::Get().GetCharacters())
    RegisterCharacterEditorWindow(name);

  // "Add New Character" menu item
  static EditorString characterName;
  GUIController::Get().AddMenuItem("Characters", "Add New", []()
  {
    GUIController::Get().CreatePopup("New Character",
    []()
    {
      characterName.DisplayEditable("New Character Folder Name");
    },
    []()
    {
      std::string name = (std::string)characterName;
      AnimationCollectionManager::Get().AddNewCharacter(name);
      CharacterEditor::Get().RegisterCharacterEditorWindow(name);
    });
  });
}

void CharacterEditor::RegisterCharacterEditorWindow(const std::string& name)
{
  GUIController::Get().AddImguiWindowFunction("Characters", name, "Assets", [this, name]()
  {
    // Lazily construct the CharacterConfiguration on first open
    auto it = _editors.find(name);
    if (it == _editors.end())
    {
      FilePath charDir(ResourceManager::Get().GetResourcePath() + "json");
      charDir.Append("characters");
      charDir.Append(name.c_str());
      auto config = std::make_unique<CharacterConfiguration>(charDir.GetPath());
      it = _editors.emplace(name, std::move(config)).first;
    }

    CharacterConfiguration& config = *it->second;
    config.DisplayEditorTabs();
  });
}
