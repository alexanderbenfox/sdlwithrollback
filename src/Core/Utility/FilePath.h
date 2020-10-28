#pragma once
#include <string>
#include "String.h"

#ifdef _WIN32
#include <filesystem>
#else
#include <sys/types.h>
#include <dirent.h>
#endif

class FilePath
{
public:
  FilePath() = default;
  FilePath(const std::string& path) : _path(StringUtils::CorrectPath(path)) {}

#ifndef _WIN32
  void read_directories(const std::string& path, std::vector<std::string>& v)
  {
    DIR* dirp = opendir(path.c_str());
    struct dirent * dp;

    while ((dp = readdir(dirp)) != NULL)
    {
        v.push_back(dp->d_name);
    }
    closedir(dirp);
  }
#endif

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
#ifdef _WIN32
    for (auto& path : std::filesystem::recursive_directory_iterator(_path))
    {
      if (path.is_directory())
        directories.push_back(path.path().string());
    }
#else
  std::vector<std::string> dirNames;
  read_directories(_path, dirNames);
  for (const std::string& dir : dirNames)
  {
    if(dir != "." && dir != "..")
    {
      FilePath fp(_path);
      fp.Append(dir.c_str());
      directories.push_back(fp);
    }
  }
#endif
    return directories;
  }

  bool Create() const
  {
    return std::filesystem::create_directory(_path);
  }

private:
  std::string _path;
};
