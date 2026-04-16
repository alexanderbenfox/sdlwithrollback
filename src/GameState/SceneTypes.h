#pragma once

enum class MainMenuOptions
{
  Main, Online, Versus, Solo, Options
};

enum class OnlineMenuOptions
{
  Ranked, Invite, Rankings
};

enum class SoloMenuOptions
{
  Arcade, VS_CPU, Career, Training, Lessons
};

enum class SceneType
{
  START, CTRLSETUP, BATTLEMODE, CSELECT, MATCH, RESULTS
};

// eventually use this for initiating different battle scene types
enum class BattleType
{
  Training,
  BestOf3,
  BestOf5
};

enum class MatchStage
{
  PREMATCH, BATTLE, POSTMATCH
};
