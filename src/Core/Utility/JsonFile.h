#pragma once
#include "AssetManagement/EditableAssets/IJsonLoadable.h"
#include <json/writer.h>
#include <json/reader.h>

#include <fstream>
#include <unordered_map>
#include <iostream>

class JsonFile
{
public:
  JsonFile(const std::string& path) : _path(path)
  {
    std::fstream file;
    file.open(path, std::ios::in);

    try
    {
      file >> _obj;
    }
    catch (const Json::RuntimeError& err)
    {
      std::cout << "Error parsing file " << path << "\nReturned with error: " << err.what() << "\n";
    }

    file.close();

    if (!_obj.isNull())
      _valid = true;
  }

  bool IsValid() const { return _valid; }
  const Json::Value& GetJsonConst() const { return _obj; }
  Json::Value& GetJson() { return _obj; }

  //! Must be a json loadable type
  template <typename T = IJsonLoadable>
  void LoadContentsIntoMap(std::unordered_map<std::string, T>& map) const
  {
    for (auto& item : _obj.getMemberNames())
    {
      map[item].Load(_obj[item]);
    }
  }

  //! Specializer for something that might have to be done on load
  template <typename T = IJsonLoadable>
  void LoadContentsIntoMap(std::unordered_map<std::string, T>& map, void (*onLoad)(T&)) const
  {
    for (auto& item : _obj.getMemberNames())
    {
      map[item].Load(_obj[item]);
      onLoad(map[item]);
    }
  }

  template <typename T = IJsonLoadable>
  void SaveContentsIntoMap(const std::unordered_map<std::string, T>& map)
  {
    for (auto& item : map)
    {
      OverwriteMemberInFile(item.first, item.second);
    }
  }

  template <typename T = IJsonLoadable>
  void OverwriteMemberInFile(const std::string& memberName, const T& data)
  {
    std::fstream dataFile;
    dataFile.open(_path, std::fstream::in | std::fstream::out);
    Json::Value fileObj;

    // remove existing entry for this move if it exists
    if (dataFile.is_open())
    {
      dataFile >> fileObj;
      if (!fileObj.isNull())
      {
        fileObj.removeMember(memberName);
      }
      dataFile.close();
    }

    // Get a reference to loaded file object
    auto& item = fileObj[memberName];

    // append new data to existing json
    data.Write(item);

    dataFile.open(_path, std::fstream::out);
    dataFile << fileObj.toStyledString() << std::endl;
    dataFile.close();
  }

private:
  std::string _path;
  bool _valid = false;
  Json::Value _obj;

};
