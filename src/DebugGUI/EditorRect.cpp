#include "EditorRect.h"
#include "../imgui/imgui.h"

//______________________________________________________________________________
void EditorPoint::Load(const Json::Value& json)
{
  if (!json["x"].isNull())
    _geom.x = json["x"].asFloat();
  if (!json["y"].isNull())
    _geom.y = json["y"].asFloat();
}

//______________________________________________________________________________
void EditorPoint::Write(Json::Value& json) const
{
  json["x"] = _geom.x;
  json["y"] = _geom.y;
}

//______________________________________________________________________________
void EditorPoint::DisplayInEditor() {}

//______________________________________________________________________________
void EditorRect::Load(const Json::Value& json)
{
  if (!json["beg"].isNull())
  {
    _geom.beg.x = json["beg"]["x"].asFloat();
    _geom.beg.y = json["beg"]["y"].asFloat();
  }
  if (!json["end"].isNull())
  {
    _geom.end.x = json["end"]["x"].asFloat();
    _geom.end.y = json["end"]["y"].asFloat();
  }
}

//______________________________________________________________________________
void EditorRect::Write(Json::Value& json) const
{
  json["beg"]["x"] = _geom.beg.x;
  json["beg"]["y"] = _geom.beg.y;
  json["end"]["x"] = _geom.end.x;
  json["end"]["y"] = _geom.end.y;
}

//______________________________________________________________________________
void EditorRect::DisplayInEditor() {}
