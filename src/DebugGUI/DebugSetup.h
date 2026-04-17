#pragma once

class GameManager;
class Timer;
class AvgCounter;

//! Registers all debug GUI windows (engine stats, action parameters,
//! scene selection, ECS snapshots, GGPO, asset editors).
void SetupDebugWindows(GameManager& gm, Timer& clock, AvgCounter& tracker);
