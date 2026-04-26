#pragma once
#include "IJsonLoadable.h"
#include "DebugGUI/EditorString.h"

#include <string>
#include <functional>
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
  void Remove(const std::string& id) { _library.erase(id); }

  // Called after an asset is renamed in the GUI (oldName, newName)
  void SetOnRenameCallback(std::function<void(const std::string&, const std::string&)> cb)
  { _onRenameCallback = std::move(cb); }

  // Rename a key programmatically (no callback, no GUI popup)
  void RenameKey(const std::string& oldName, const std::string& newName)
  {
    auto it = _library.find(oldName);
    if (it != _library.end())
    {
      T cpy = std::move(it->second);
      _library.erase(it);
      _library[newName] = std::move(cpy);
    }
  }

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

  std::function<void(const std::string&, const std::string&)> _onRenameCallback;

};
