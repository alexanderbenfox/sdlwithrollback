#pragma once

class CharacterEditor
{
public:
  static CharacterEditor& Get()
  {
    static CharacterEditor instance;
    return instance;
  }

  void AddCreateNewCharacterButton();

private:
  CharacterEditor() = default;

};

