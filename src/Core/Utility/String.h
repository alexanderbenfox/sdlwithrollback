#pragma once
#include <string>
#include <vector>

struct StringUtils
{
  template <typename Out>
  static void Split(const std::string& s, char delim, Out result)
  {
    std::istringstream iss(s);
    std::string item;
    while (std::getline(iss, item, delim)) {
      *result++ = item;
    }
  }

  static std::vector<std::string> Split(const std::string& s, char delim);

  static std::string Connect(std::vector<std::string>::iterator const& beg, std::vector<std::string>::iterator const& end, char delim);

  static std::string CorrectPath(std::string file);
};
