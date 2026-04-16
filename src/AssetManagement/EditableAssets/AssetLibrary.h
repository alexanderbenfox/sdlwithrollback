#pragma once
#include "IJsonLoadable.h"
#include "DebugGUI/EditorString.h"

#include <string>
#include <unordered_map>

class JsonFile;

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
  T const& Get(const std::string& id) { return _library[id]; }
  T& GetModifiable(const std::string& id) { return _library[id]; }

private:
  void OnRename(const std::string& item);
  void OnCopy(const std::string& item);
  void OnDelete(const std::string& item) { _library.erase(item); }

  //!
  LibType _library;

  //! Used to create a new item in the library
  EditorString _newItemName;
  T _newItem;
  std::string _dropdownSelection = "";

  bool _showItemList = false;

};
