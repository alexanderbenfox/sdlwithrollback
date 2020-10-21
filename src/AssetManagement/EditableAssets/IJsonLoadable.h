#pragma once
#include <json/value.h>

struct IJsonLoadable
{
  virtual void Load(const Json::Value&) = 0;
  virtual void Write(Json::Value&) const = 0;
};
