#pragma once
#include <string>
#include "String.h"

#include <filesystem>

class FilePath
{
public:
  FilePath() = default;
  FilePath(const std::string& path) : _path(StringUtils::CorrectPath(path)) {}

  void Append(const char* path)
  {
    StringUtils::AppendSubPath(_path, path);
  }

  std::string GetPath() const { return _path; }

  //! Get last thing in the file path (working directory or file)
  std::string GetLast() const
  {
    return StringUtils::SplitFilePath(_path).back();
  }

  std::vector<FilePath> GetSubDirectories()
  {
    std::vector<FilePath> directories;
    for (auto& path : std::filesystem::recursive_directory_iterator(_path))
    {
      if (path.is_directory())
        directories.push_back(path.path().string());
    }
    return directories;
  }

private:
  std::string _path;
};
