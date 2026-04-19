#include "Core/FSM/StateVisualizer.h"
#include "Core/FSM/StateEnumMaps.h"
#include "Globals.h"

#include "../imgui/imgui.h"
#include <cmath>
#include <algorithm>

// Node positions indexed by FighterStateID enum value
// Layout: 4 columns — Movement | Attacks | Specials/Throws | Damage
const StateVisualizer::NodePos StateVisualizer::kPositions[] = {
  {0,   100},  // Idle
  {0,   0},    // WalkForward
  {0,   200},  // WalkBackward
  {0,   400},  // Jumping
  {0,   460},  // Falling
  {0,   300},  // CrouchTransition
  {0,   340},  // Crouched (skip a bit to group with CrouchTransition)
  {0,   150},  // ForwardDash
  {0,   250},  // BackDash
  {170, 0},    // StandingLight
  {170, 50},   // StandingMedium
  {170, 100},  // StandingHeavy
  {170, 300},  // CrouchingLight
  {170, 350},  // CrouchingMedium
  {170, 400},  // CrouchingHeavy
  {170, 460},  // JumpingLight
  {170, 510},  // JumpingMedium
  {170, 560},  // JumpingHeavy
  {340, 0},    // SpecialMove1
  {340, 50},   // SpecialMove2
  {340, 100},  // SpecialMove3
  {340, 150},  // SpecialMove4
  {340, 250},  // ForwardThrow
  {340, 300},  // BackThrow
  {340, 350},  // ThrowMiss
  {340, 400},  // Grappling
  {510, 0},    // Hitstun
  {510, 50},   // CrouchingHitstun
  {510, 100},  // BlockstunStanding
  {510, 150},  // BlockstunCrouching
  {510, 250},  // KnockdownAirborne
  {510, 300},  // KnockdownHitGround
  {510, 350},  // KnockdownOnGround
  {510, 400},  // Grappled
};

//______________________________________________________________________________
ImU32 StateVisualizer::StanceColor(StanceState stance)
{
  switch (stance)
  {
    case StanceState::STANDING:  return IM_COL32(70, 100, 170, 255);
    case StanceState::CROUCHING: return IM_COL32(60, 140, 80, 255);
    case StanceState::JUMPING:   return IM_COL32(60, 150, 160, 255);
    case StanceState::KNOCKDOWN: return IM_COL32(170, 60, 60, 255);
    default:                     return IM_COL32(120, 120, 120, 255);
  }
}

//______________________________________________________________________________
ImU32 StateVisualizer::PriorityColor(int8_t priority)
{
  if (priority >= 90) return IM_COL32(220, 50, 50, 255);    // hit/thrown/fall
  if (priority >= 70) return IM_COL32(220, 150, 50, 255);   // specials
  if (priority >= 55) return IM_COL32(200, 200, 60, 255);   // dashes/throws
  if (priority >= 40) return IM_COL32(60, 150, 220, 255);   // movement
  if (priority >= 20) return IM_COL32(60, 180, 80, 255);    // walk/return
  return IM_COL32(150, 150, 150, 255);                      // completion
}

//______________________________________________________________________________
void StateVisualizer::EdgePoint(float cx, float cy, float tx, float ty,
                                float halfW, float halfH, float& outX, float& outY)
{
  float dx = tx - cx, dy = ty - cy;
  if (dx == 0.0f && dy == 0.0f) { outX = cx; outY = cy; return; }

  float sx = (dx != 0.0f) ? halfW / fabsf(dx) : 1e6f;
  float sy = (dy != 0.0f) ? halfH / fabsf(dy) : 1e6f;
  float s = std::min(sx, sy);

  outX = cx + dx * s;
  outY = cy + dy * s;
}

//______________________________________________________________________________
float StateVisualizer::PointToSegmentDist(float px, float py,
                                          float ax, float ay, float bx, float by)
{
  float abx = bx - ax, aby = by - ay;
  float apx = px - ax, apy = py - ay;
  float dot = apx * abx + apy * aby;
  float lenSq = abx * abx + aby * aby;
  float t = (lenSq > 0.0f) ? std::clamp(dot / lenSq, 0.0f, 1.0f) : 0.0f;
  float cx = ax + t * abx - px, cy = ay + t * aby - py;
  return sqrtf(cx * cx + cy * cy);
}

//______________________________________________________________________________
void StateVisualizer::DrawArrowHead(ImDrawList* dl, float tipX, float tipY,
                                     float fromX, float fromY, ImU32 color)
{
  float dx = tipX - fromX, dy = tipY - fromY;
  float len = sqrtf(dx * dx + dy * dy);
  if (len < 1.0f) return;
  dx /= len; dy /= len;

  float headLen = 8.0f, headW = 4.0f;
  float bx = tipX - dx * headLen, by = tipY - dy * headLen;

  ImVec2 pts[3] = {
    {tipX, tipY},
    {bx + (-dy) * headW, by + dx * headW},
    {bx - (-dy) * headW, by - dx * headW}
  };
  dl->AddTriangleFilled(pts[0], pts[1], pts[2], color);
}

//______________________________________________________________________________
void StateVisualizer::Display(const std::string& characterName)
{
  FighterStateTable::StateArray* states = FighterStateTable::Get().GetMutableTable(characterName);
  if (!states)
  {
    ImGui::Text("No state table loaded for %s", characterName.c_str());
    return;
  }

  float totalWidth = ImGui::GetContentRegionAvail().x;
  float canvasWidth = totalWidth - kDetailPanelWidth - 8.0f;

  ImGui::BeginChild("##StateCanvas", ImVec2(canvasWidth, 0), true,
                     ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
  DrawCanvas(*states);
  ImGui::EndChild();

  ImGui::SameLine();

  ImGui::BeginChild("##StateDetail", ImVec2(0, 0), true);
  DrawDetailPanel(*states, characterName);
  ImGui::EndChild();
}

//______________________________________________________________________________
void StateVisualizer::DrawCanvas(FighterStateTable::StateArray& states)
{
  ImVec2 canvasPos = ImGui::GetCursorScreenPos();
  ImVec2 canvasSize = ImGui::GetContentRegionAvail();

  ImGui::InvisibleButton("##canvas_bg", canvasSize);
  bool hovered = ImGui::IsItemHovered();
  bool active = ImGui::IsItemActive();

  // Pan with right mouse drag
  if (active && ImGui::IsMouseDragging(ImGuiMouseButton_Right))
  {
    ImGuiIO& io = ImGui::GetIO();
    _panX += io.MouseDelta.x;
    _panY += io.MouseDelta.y;
  }

  // Zoom with scroll
  if (hovered)
  {
    float scroll = ImGui::GetIO().MouseWheel;
    if (scroll != 0.0f)
    {
      _zoom *= (1.0f + scroll * 0.1f);
      _zoom = std::clamp(_zoom, 0.3f, 3.0f);
    }
  }

  float ox = canvasPos.x + _panX;
  float oy = canvasPos.y + _panY;

  ImDrawList* dl = ImGui::GetWindowDrawList();

  // Background
  dl->AddRectFilled(canvasPos, ImVec2(canvasPos.x + canvasSize.x, canvasPos.y + canvasSize.y),
                    IM_COL32(30, 30, 30, 255));

  // Column labels
  const char* colLabels[] = {"Movement", "Attacks", "Specials / Throws", "Damage"};
  float colX[] = {0, 170, 340, 510};
  for (int c = 0; c < 4; ++c)
  {
    float lx = ox + colX[c] * _zoom;
    float ly = oy - 18.0f * _zoom;
    dl->AddText(ImVec2(lx, ly), IM_COL32(200, 200, 200, 120), colLabels[c]);
  }

  // Draw arrows first (behind nodes)
  DrawArrows(dl, states, ox, oy);
  // Draw nodes on top
  DrawNodes(dl, states, ox, oy);
  // Handle click input — only when canvas is hovered (prevents detail panel
  // combo clicks from clearing arrow selection)
  if (hovered)
    HandleInput(states, ox, oy);
}

//______________________________________________________________________________
void StateVisualizer::DrawNodes(ImDrawList* dl, const FighterStateTable::StateArray& states,
                                float ox, float oy)
{
  float halfW = kNodeWidth * _zoom * 0.5f;
  float halfH = kNodeHeight * _zoom * 0.5f;

  for (size_t i = 0; i < static_cast<size_t>(FighterStateID::COUNT); ++i)
  {
    const StateDefinition& state = states[i];
    if (state.animationName.empty() && state.transitions.empty())
      continue;

    float cx = ox + kPositions[i].x * _zoom + halfW;
    float cy = oy + kPositions[i].y * _zoom + halfH;

    ImVec2 tl(cx - halfW, cy - halfH);
    ImVec2 br(cx + halfW, cy + halfH);

    ImU32 fillColor = StanceColor(state.stanceState);
    dl->AddRectFilled(tl, br, fillColor, 4.0f);

    // Selection highlight
    if (static_cast<int>(i) == _selectedState)
      dl->AddRect(tl, br, IM_COL32(255, 220, 50, 255), 4.0f, 0, 2.5f);
    else
      dl->AddRect(tl, br, IM_COL32(200, 200, 200, 100), 4.0f);

    // Attack/grapple indicator — small colored dot in top-right corner
    if (state.isAttackState)
    {
      float dotR = 4.0f * _zoom;
      dl->AddCircleFilled(ImVec2(br.x - dotR - 2, tl.y + dotR + 2),
                           dotR, IM_COL32(255, 160, 50, 255));
    }
    else if (state.isGrappleState)
    {
      float dotR = 4.0f * _zoom;
      dl->AddCircleFilled(ImVec2(br.x - dotR - 2, tl.y + dotR + 2),
                           dotR, IM_COL32(120, 180, 255, 255));
    }

    // Label
    const char* name = FighterStateIDToString(state.id);
    ImVec2 textSize = ImGui::CalcTextSize(name);
    float tx = cx - textSize.x * 0.5f;
    float ty = cy - textSize.y * 0.5f;
    dl->AddText(ImVec2(tx, ty), IM_COL32(255, 255, 255, 255), name);
  }
}

//______________________________________________________________________________
void StateVisualizer::DrawArrows(ImDrawList* dl, const FighterStateTable::StateArray& states,
                                  float ox, float oy)
{
  _drawnArrows.clear();

  if (_selectedState < 0) return;

  const StateDefinition& srcState = states[_selectedState];
  float halfW = kNodeWidth * _zoom * 0.5f;
  float halfH = kNodeHeight * _zoom * 0.5f;

  float srcCX = ox + kPositions[_selectedState].x * _zoom + halfW;
  float srcCY = oy + kPositions[_selectedState].y * _zoom + halfH;

  // Group transitions by target to avoid overlapping arrows
  struct TargetGroup { int targetIdx; int8_t maxPriority; int count; };
  std::vector<TargetGroup> groups;

  for (const auto& rule : srcState.transitions)
  {
    int targetIdx = static_cast<int>(rule.targetState);
    if (rule.targetState == FighterStateID::COUNT)
      continue; // skip HitResolver — no node to draw to

    bool found = false;
    for (auto& g : groups)
    {
      if (g.targetIdx == targetIdx)
      {
        g.maxPriority = std::max(g.maxPriority, rule.priority);
        g.count++;
        found = true;
        break;
      }
    }
    if (!found)
      groups.push_back({targetIdx, rule.priority, 1});
  }

  for (const auto& g : groups)
  {
    float dstCX = ox + kPositions[g.targetIdx].x * _zoom + halfW;
    float dstCY = oy + kPositions[g.targetIdx].y * _zoom + halfH;

    float sx, sy, ex, ey;
    EdgePoint(srcCX, srcCY, dstCX, dstCY, halfW, halfH, sx, sy);
    EdgePoint(dstCX, dstCY, srcCX, srcCY, halfW, halfH, ex, ey);

    ImU32 color = PriorityColor(g.maxPriority);
    bool isSelected = (_selectedTarget == g.targetIdx);

    float thickness = isSelected ? 3.0f : 1.5f;
    if (isSelected) color = IM_COL32(255, 255, 100, 255);

    dl->AddLine(ImVec2(sx, sy), ImVec2(ex, ey), color, thickness);
    DrawArrowHead(dl, ex, ey, sx, sy, color);

    // Count badge
    if (g.count > 1)
    {
      float mx = (sx + ex) * 0.5f, my = (sy + ey) * 0.5f;
      char countStr[8];
      snprintf(countStr, sizeof(countStr), "x%d", g.count);
      dl->AddText(ImVec2(mx + 4, my - 10), color, countStr);
    }

    _drawnArrows.push_back({sx, sy, ex, ey, _selectedState, g.targetIdx});
  }
}

//______________________________________________________________________________
void StateVisualizer::HandleInput(const FighterStateTable::StateArray& states,
                                   float ox, float oy)
{
  if (!ImGui::IsMouseClicked(ImGuiMouseButton_Left))
    return;

  ImVec2 mousePos = ImGui::GetMousePos();
  float mx = mousePos.x, my = mousePos.y;

  float halfW = kNodeWidth * _zoom * 0.5f;
  float halfH = kNodeHeight * _zoom * 0.5f;

  // Check node clicks first
  for (size_t i = 0; i < static_cast<size_t>(FighterStateID::COUNT); ++i)
  {
    const StateDefinition& state = states[i];
    if (state.animationName.empty() && state.transitions.empty())
      continue;

    float cx = ox + kPositions[i].x * _zoom + halfW;
    float cy = oy + kPositions[i].y * _zoom + halfH;

    if (mx >= cx - halfW && mx <= cx + halfW && my >= cy - halfH && my <= cy + halfH)
    {
      if (_selectedState == static_cast<int>(i))
      {
        // Clicking same state deselects
        _selectedState = -1;
        _selectedTarget = -1;
        _selectedRuleIdx = -1;
      }
      else
      {
        _selectedState = static_cast<int>(i);
        _selectedTarget = -1;
        _selectedRuleIdx = -1;
      }
      return;
    }
  }

  // Check arrow clicks
  for (const auto& arrow : _drawnArrows)
  {
    float dist = PointToSegmentDist(mx, my, arrow.x0, arrow.y0, arrow.x1, arrow.y1);
    if (dist < 6.0f)
    {
      _selectedTarget = arrow.targetState;
      _selectedRuleIdx = 0;
      return;
    }
  }

  // Clicked empty space — deselect arrow but keep state selected
  _selectedTarget = -1;
  _selectedRuleIdx = -1;
}

//______________________________________________________________________________
void StateVisualizer::DrawDetailPanel(FighterStateTable::StateArray& states,
                                       const std::string& characterName)
{
  // Save / Reload buttons
  if (_dirty)
    ImGui::TextColored(ImVec4(1, 0.8f, 0.2f, 1), "Unsaved changes");

  if (ImGui::Button("Save"))
  {
    FighterStateTable::Get().WriteStatesToJson(characterName);
    _dirty = false;
  }
  ImGui::SameLine();
  if (ImGui::Button("Reload"))
  {
    FighterStateTable::Get().Reload(characterName);
    _dirty = false;
    _selectedState = -1;
    _selectedTarget = -1;
  }

  ImGui::Separator();

  if (_selectedState < 0)
  {
    ImGui::TextWrapped("Click a state node to view/edit properties.\n\n"
                       "Right-drag to pan, scroll to zoom.\n\n"
                       "Arrow colors:\n");
    ImGui::TextColored(ImVec4(0.86f, 0.2f, 0.2f, 1), "  Red: hit/thrown/fall (90+)");
    ImGui::TextColored(ImVec4(0.86f, 0.59f, 0.2f, 1), "  Orange: specials (70+)");
    ImGui::TextColored(ImVec4(0.78f, 0.78f, 0.24f, 1), "  Yellow: dashes/throws (55+)");
    ImGui::TextColored(ImVec4(0.24f, 0.59f, 0.86f, 1), "  Blue: movement (40+)");
    ImGui::TextColored(ImVec4(0.24f, 0.71f, 0.31f, 1), "  Green: walk/return (20+)");
    ImGui::TextColored(ImVec4(0.59f, 0.59f, 0.59f, 1), "  Gray: completion (<20)");
    return;
  }

  StateDefinition& state = states[_selectedState];
  ImGui::Text("%s", FighterStateIDToString(state.id));
  ImGui::Separator();

  // If an arrow is selected, show transition editor
  if (_selectedTarget >= 0)
  {
    DrawTransitionEditor(states);
    return;
  }

  // Otherwise show state properties
  DrawStateProperties(state);
}

//______________________________________________________________________________
void StateVisualizer::DrawStateProperties(StateDefinition& state)
{
  if (ImGui::CollapsingHeader("Animation", ImGuiTreeNodeFlags_DefaultOpen))
  {
    char animBuf[128];
    strncpy(animBuf, state.animationName.c_str(), sizeof(animBuf) - 1);
    animBuf[sizeof(animBuf) - 1] = 0;
    if (ImGui::InputText("Animation Name", animBuf, sizeof(animBuf)))
    {
      state.animationName = animBuf;
      _dirty = true;
    }

    if (ImGui::Checkbox("Loop", &state.loopAnimation)) _dirty = true;
    ImGui::SameLine();
    if (ImGui::Checkbox("Force Restart", &state.forceAnimRestart)) _dirty = true;
    if (ImGui::InputFloat("Play Speed", &state.playSpeed, 0.1f, 0.5f, "%.1f")) _dirty = true;
  }

  // Action linkage — action key is implicitly the animation name
  if (ImGui::CollapsingHeader("Action", ImGuiTreeNodeFlags_DefaultOpen))
  {
    if (state.isAttackState)
    {
      ImGui::TextColored(ImVec4(1.0f, 0.7f, 0.3f, 1.0f), "Action: %s", state.animationName.c_str());
      ImGui::TextWrapped("Hitbox and frame data are loaded from the action with this animation name.");
    }
    else if (state.isGrappleState)
    {
      ImGui::TextColored(ImVec4(0.6f, 0.8f, 1.0f, 1.0f), "Grapple action: %s", state.animationName.c_str());
    }
    else
    {
      ImGui::TextDisabled("No action linked (not an attack state)");
    }
  }

  if (ImGui::CollapsingHeader("Completion", ImGuiTreeNodeFlags_DefaultOpen))
  {
    const char* completionTypes[] = {"None", "Timer", "Animation"};
    int compType = static_cast<int>(state.completionType);
    if (ImGui::Combo("Type", &compType, completionTypes, 3))
    {
      state.completionType = static_cast<StateDefinition::CompletionType>(compType);
      _dirty = true;
    }

    if (state.completionType == StateDefinition::Timer)
    {
      if (ImGui::InputInt("Timer Frames", &state.timerFrames)) _dirty = true;
    }

    if (state.completionType != StateDefinition::None)
    {
      // Completion target dropdown
      int targetIdx = static_cast<int>(state.completionTarget);
      const char* targetPreview = FighterStateIDToString(state.completionTarget);
      if (ImGui::BeginCombo("Target", targetPreview))
      {
        for (int i = 0; i < static_cast<int>(FighterStateID::COUNT); ++i)
        {
          bool selected = (i == targetIdx);
          if (ImGui::Selectable(FighterStateIDToString(static_cast<FighterStateID>(i)), selected))
          {
            state.completionTarget = static_cast<FighterStateID>(i);
            _dirty = true;
          }
        }
        ImGui::EndCombo();
      }
    }
  }

  if (ImGui::CollapsingHeader("Properties", ImGuiTreeNodeFlags_DefaultOpen))
  {
    const char* stanceNames[] = {"Standing", "Crouching", "Jumping", "Knockdown"};
    int stanceIdx = static_cast<int>(state.stanceState);
    if (ImGui::Combo("Stance", &stanceIdx, stanceNames, 4))
    {
      state.stanceState = static_cast<StanceState>(stanceIdx);
      _dirty = true;
    }

    const char* actionNames[] = {"None", "Blockstun", "Hitstun", "Dashing", "Light", "Medium", "Heavy"};
    int actionIdx = static_cast<int>(state.actionState);
    if (ImGui::Combo("Action", &actionIdx, actionNames, 7))
    {
      state.actionState = static_cast<ActionState>(actionIdx);
      _dirty = true;
    }

    const char* entryNames[] = {"NoMovement", "Stop", "StopHorizontal", "UseHitKnockback", "UseHitKnockbackFull", "Custom"};
    int entryIdx = static_cast<int>(state.entryMovement);
    if (ImGui::Combo("Entry Movement", &entryIdx, entryNames, 6))
    {
      state.entryMovement = static_cast<StateDefinition::EntryMovement>(entryIdx);
      _dirty = true;
    }

    if (ImGui::Checkbox("Horizontal Only", &state.horizontalOnly)) _dirty = true;
  }

  if (ImGui::CollapsingHeader("Flags"))
  {
    if (ImGui::Checkbox("Hittable", &state.isHittable)) _dirty = true;
    if (ImGui::Checkbox("Can Block", &state.canBlock)) _dirty = true;
    if (ImGui::Checkbox("Attack State", &state.isAttackState)) _dirty = true;
    if (ImGui::Checkbox("Grapple State", &state.isGrappleState)) _dirty = true;
    if (ImGui::Checkbox("In Knockdown", &state.inKnockdown)) _dirty = true;
    if (ImGui::Checkbox("Applies Damage", &state.appliesDamage)) _dirty = true;
    if (ImGui::Checkbox("Is Blocking", &state.isBlocking)) _dirty = true;
    if (ImGui::Checkbox("Sets Juggle Gravity", &state.setsJuggleGravity)) _dirty = true;
    if (ImGui::Checkbox("Resets Juggle Gravity", &state.resetsJuggleGravity)) _dirty = true;

    ImGui::Separator();
    ImGui::Text("Cancel Flags:");
    bool cHG = state.cancelFlags & StateDefinition::Cancel_HitGround;
    bool cSp = state.cancelFlags & StateDefinition::Cancel_Special;
    bool cNo = state.cancelFlags & StateDefinition::Cancel_Normal;
    if (ImGui::Checkbox("HitGround", &cHG))
    {
      state.cancelFlags = (state.cancelFlags & ~StateDefinition::Cancel_HitGround) | (cHG ? StateDefinition::Cancel_HitGround : 0);
      _dirty = true;
    }
    if (ImGui::Checkbox("Special", &cSp))
    {
      state.cancelFlags = (state.cancelFlags & ~StateDefinition::Cancel_Special) | (cSp ? StateDefinition::Cancel_Special : 0);
      _dirty = true;
    }
    if (ImGui::Checkbox("Normal", &cNo))
    {
      state.cancelFlags = (state.cancelFlags & ~StateDefinition::Cancel_Normal) | (cNo ? StateDefinition::Cancel_Normal : 0);
      _dirty = true;
    }
  }

  // Transition summary — click to select arrow
  if (ImGui::CollapsingHeader("Transitions"))
  {
    for (size_t t = 0; t < state.transitions.size(); ++t)
    {
      const TransitionRule& rule = state.transitions[t];
      std::string targetName = (rule.targetState == FighterStateID::COUNT)
        ? "HitResolver" : FighterStateIDToString(rule.targetState);

      ImGui::PushID(static_cast<int>(t));
      std::string label = "[" + std::to_string(rule.priority) + "] -> " + targetName;
      if (ImGui::Selectable(label.c_str()))
      {
        _selectedTarget = static_cast<int>(rule.targetState);
        _selectedRuleIdx = static_cast<int>(t);
      }
      ImGui::PopID();
    }
  }
}

//______________________________________________________________________________
void StateVisualizer::DrawTransitionEditor(FighterStateTable::StateArray& states)
{
  StateDefinition& srcState = states[_selectedState];
  const char* srcName = FighterStateIDToString(srcState.id);
  const char* dstName = (_selectedTarget == static_cast<int>(FighterStateID::COUNT))
    ? "HitResolver" : FighterStateIDToString(static_cast<FighterStateID>(_selectedTarget));

  ImGui::Text("%s -> %s", srcName, dstName);
  ImGui::Separator();

  if (ImGui::Button("<- Back to state"))
  {
    _selectedTarget = -1;
    _selectedRuleIdx = -1;
    return;
  }

  ImGui::Separator();

  // Collect all transition indices that go to this target
  std::vector<size_t> ruleIndices;
  for (size_t t = 0; t < srcState.transitions.size(); ++t)
  {
    if (static_cast<int>(srcState.transitions[t].targetState) == _selectedTarget)
      ruleIndices.push_back(t);
  }

  if (ruleIndices.empty())
  {
    ImGui::Text("No transitions to this target.");
  }

  // Clamp selected rule
  if (_selectedRuleIdx < 0 || _selectedRuleIdx >= static_cast<int>(ruleIndices.size()))
    _selectedRuleIdx = 0;

  int deleteIdx = -1;

  for (int ri = 0; ri < static_cast<int>(ruleIndices.size()); ++ri)
  {
    size_t tIdx = ruleIndices[ri];
    TransitionRule& rule = srcState.transitions[tIdx];

    ImGui::PushID(static_cast<int>(tIdx));

    bool isOpen = ImGui::CollapsingHeader(
      ("Rule " + std::to_string(ri) + " [pri " + std::to_string(rule.priority) + "]").c_str(),
      ImGuiTreeNodeFlags_DefaultOpen);

    if (isOpen)
    {
      // Priority
      int pri = rule.priority;
      if (ImGui::InputInt("Priority", &pri))
      {
        rule.priority = static_cast<int8_t>(std::clamp(pri, -128, 127));
        _dirty = true;
      }

      // Target state
      const char* targetPreview = (rule.targetState == FighterStateID::COUNT)
        ? "HitResolver" : FighterStateIDToString(rule.targetState);
      if (ImGui::BeginCombo("Target", targetPreview))
      {
        // HitResolver option
        if (ImGui::Selectable("HitResolver", rule.targetState == FighterStateID::COUNT))
        {
          rule.targetState = FighterStateID::COUNT;
          _selectedTarget = static_cast<int>(FighterStateID::COUNT);
          _dirty = true;
        }
        for (int i = 0; i < static_cast<int>(FighterStateID::COUNT); ++i)
        {
          bool selected = (static_cast<int>(rule.targetState) == i);
          if (ImGui::Selectable(FighterStateIDToString(static_cast<FighterStateID>(i)), selected))
          {
            rule.targetState = static_cast<FighterStateID>(i);
            _selectedTarget = i;
            _dirty = true;
          }
        }
        ImGui::EndCombo();
      }

      // Required flags
      ImGui::Text("Required:");
      DrawFlagCheckboxes("req", rule.requiredFlags);

      // Forbidden flags
      ImGui::Text("Forbidden:");
      DrawFlagCheckboxes("forb", rule.forbiddenFlags);

      // Delete button
      ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7f, 0.2f, 0.2f, 1.0f));
      if (ImGui::Button("Delete Rule"))
        deleteIdx = static_cast<int>(tIdx);
      ImGui::PopStyleColor();
    }

    ImGui::PopID();
    ImGui::Separator();
  }

  // Handle deletion
  if (deleteIdx >= 0)
  {
    srcState.transitions.erase(srcState.transitions.begin() + deleteIdx);
    _dirty = true;
    if (ruleIndices.size() <= 1)
    {
      _selectedTarget = -1;
      _selectedRuleIdx = -1;
    }
  }

  // Add new transition button
  ImGui::Spacing();
  if (ImGui::Button("+ Add Transition"))
  {
    TransitionRule newRule;
    newRule.targetState = static_cast<FighterStateID>(_selectedTarget);
    newRule.priority = 10;
    srcState.transitions.push_back(newRule);
    _dirty = true;
  }
}

//______________________________________________________________________________
void StateVisualizer::DrawFlagCheckboxes(const char* label, ConditionFlags& flags)
{
  ImGui::PushID(label);
  ImGui::Columns(2, nullptr, false);
  for (size_t f = 0; f < static_cast<size_t>(CF_BuiltInCount); ++f)
  {
    bool has = flags.test(f);
    if (ImGui::Checkbox(ConditionFlagToString(static_cast<ConditionFlag>(f)), &has))
    {
      flags.set(f, has);
      _dirty = true;
    }
    ImGui::NextColumn();
  }
  ImGui::Columns(1);
  ImGui::PopID();
}
