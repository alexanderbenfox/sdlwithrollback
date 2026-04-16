#include "Core/Utility/String.h"
#include <sstream>

std::vector<std::string> StringUtils::Split(const std::string& s, char delim)
{
  std::vector<std::string> elems;
  Split(s, delim, std::back_inserter(elems));
  return elems;
}

std::string StringUtils::Connect(std::vector<std::string>::iterator const& beg, std::vector<std::string>::iterator const& end, char delim)
{
  std::stringstream ss("");
  auto emit = [&ss, delim, need_delim = false](std::string const& x) mutable
  {
    if (need_delim) ss << delim;
    ss << x;
    need_delim = true;
  };

  for (auto it = beg; it != end; ++it)
    emit(*it);
  return ss.str();
}

std::string StringUtils::CorrectPath(std::string file)
{
  return file;
}

void StringUtils::AppendSubPath(std::string& path, const char* subPath)
{
  path += "/" + std::string(subPath);
}

std::vector<std::string> StringUtils::SplitFilePath(const std::string& path)
{
  return Split(path, '/');
}
