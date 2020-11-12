#include "AnimationEditor.h"
#include "DebugGUI/GUIController.h"

#include "Managers/GameManagement.h"
#include "Managers/AnimationCollectionManager.h"

void CharacterEditor::AddCreateNewCharacterButton()
{
  static EditorString characterName;
  GUIController::Get().AddMenuItem("Characters", "Add New", []()
  {
    GUIController::Get().CreatePopup(
    []()
    {
      characterName.DisplayEditable("New Character Folder Name");
    },
    []()
    {
      AnimationCollectionManager::Get().AddNewCharacter((std::string)characterName);
    });
  });

}
