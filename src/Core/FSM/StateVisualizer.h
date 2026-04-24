#pragma once
#include "Core/FSM/CharacterState.h"
#include "Core/FSM/FighterStateTable.h"
#include <vector>
#include <string>

struct ImVec2;
struct ImDrawList;
typedef unsigned int ImU32;

//______________________________________________________________________________
class StateVisualizer
{
public:
  void Display(const std::string& characterName,
               const std::vector<std::string>& animationNames = {},
               const std::vector<std::string>& actionNames = {});

private:
  // --- Layout ---
  static constexpr float kNodeWidth = 130.0f;
  static constexpr float kNodeHeight = 30.0f;
  static constexpr float kDetailPanelWidth = 340.0f;

  struct NodePos { float x, y; };
  static const NodePos kPositions[static_cast<size_t>(FighterStateID::COUNT)];

  // --- Canvas state ---
  float _panX = 20.0f, _panY = 20.0f;
  float _zoom = 1.0f;

  // --- Selection ---
  int _selectedState = -1;
  int _selectedTarget = -1;   // arrow: selectedState -> selectedTarget
  int _selectedRuleIdx = -1;  // which rule within the arrow bundle

  bool _dirty = false;

  // --- Add state / transition UI ---
  int _addStateSelection = 0;
  int _addTransitionTarget = 0;

  // --- Available asset names (set each frame by Display) ---
  std::vector<std::string> _animationNames;
  std::vector<std::string> _actionNames;

  // --- Drawn arrows for hit testing ---
  struct DrawnArrow
  {
    float x0, y0, x1, y1;
    int sourceState, targetState;
  };
  std::vector<DrawnArrow> _drawnArrows;

  // --- Drawing ---
  void DrawCanvas(FighterStateTable::StateArray& states);
  void DrawNodes(ImDrawList* dl, const FighterStateTable::StateArray& states,
                 float ox, float oy);
  void DrawArrows(ImDrawList* dl, const FighterStateTable::StateArray& states,
                  float ox, float oy);
  void HandleInput(const FighterStateTable::StateArray& states, float ox, float oy);

  // --- Detail panel ---
  void DrawDetailPanel(FighterStateTable::StateArray& states,
                       const std::string& characterName);
  void DrawStateProperties(StateDefinition& state,
                           const std::vector<std::string>& animationNames,
                           const std::vector<std::string>& actionNames);
  void DrawTransitionEditor(FighterStateTable::StateArray& states);
  void DrawFlagCheckboxes(const char* label, ConditionFlags& flags);

  // --- Geometry helpers ---
  static ImU32 StanceColor(StanceState stance);
  static ImU32 PriorityColor(int8_t priority);
  static void EdgePoint(float cx, float cy, float tx, float ty,
                        float halfW, float halfH, float& outX, float& outY);
  static float PointToSegmentDist(float px, float py,
                                  float ax, float ay, float bx, float by);
  static void DrawArrowHead(ImDrawList* dl, float tipX, float tipY,
                            float fromX, float fromY, ImU32 color);
};
