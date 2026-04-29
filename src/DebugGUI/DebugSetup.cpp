#include "DebugGUI/DebugSetup.h"
#include "Managers/GameManagement.h"
#include "Managers/ResourceManager.h"
#include "Managers/AnimationCollectionManager.h"
#include "Managers/GGPOManager.h"

#include "Components/Input.h"
#include "Systems/InputHandlerSystem.h"
#include "DebugGUI/GUIController.h"
#include "AssetManagement/EditableAssets/Editor/AnimationEditor.h"
#include "AssetManagement/EditableAssets/AssetLibraryImpl.h"

#include "Core/FSM/FighterStateTable.h"
#include "Core/FSM/StateEnumMaps.h"
#include "Components/FighterFSMComponent.h"
#include "Components/Actors/GameActor.h"
#include "Components/Rigidbody.h"
#include "Globals.h"

//______________________________________________________________________________
void SetupDebugWindows(GameManager& gm, Timer& clock, AvgCounter& tracker)
{
  // Debug-only snapshot storage
  static std::vector<SBuffer> p1Snapshots;
  static std::vector<SBuffer> p2Snapshots;
  static std::vector<SBuffer> gameStateSnapshots;

  GUIController::Get().AddImguiWindowFunction("Main Debug Window", "Engine Stats",
    [&clock, &tracker]()
    {
      ImGui::BeginGroup();
      ImGui::Text("Update function time average %.3f ms/frame", (double)clock.GetUpdateTime() / 1000000.0);
      ImGui::PlotLines("Update speed over time (ms/frame) - updated every 10 frames",
        [](void* data, int idx) { return (float)((long long*)data)[idx] / 1000000.0f; },
        tracker.GetValues(), tracker.NumValues(), 0, nullptr, FLT_MAX, FLT_MAX, ImVec2(200, 100));
      ImGui::EndGroup();
    });

  static float ts[40];
  for (int i = 0; i < 40; i++)
    ts[i] = static_cast<float>(i);

  GUIController::Get().AddImguiWindowFunction("Main Debug Window", "Dash Function parameters", []()
  {
    ImGui::BeginGroup();

    ImGui::InputFloat("Walk speed", &GlobalVars::BaseWalkSpeed, 1.0f, 10.0f, 0);
    ImGui::InputFloat("Jump velocity", &GlobalVars::JumpVelocity, 1.0f, 10.0f, 0);
    ImGui::InputFloat2("Gravity force", &GlobalVars::Gravity.x, 1);
    ImGui::InputFloat2("Juggle Gravity force", &GlobalVars::JuggleGravity.x, 1);
    ImGui::InputInt("number of frames for dash", &GlobalVars::nDashFrames);
    ImGui::InputInt("Hit stop frames ON HIT", &GlobalVars::HitStopFramesOnHit);
    ImGui::InputInt("Hit stop frames ON BLOCK", &GlobalVars::HitStopFramesOnBlock);

    ImGui::Checkbox("Show hit effects", &GlobalVars::ShowHitEffects);

    ImGui::Separator();
    if (ImGui::Button("Reload State Tables"))
    {
      FighterStateTable::Get().Reload("Ryu");
    }

    if (ImGui::CollapsingHeader("Dash Function"))
    {
      ImGui::PlotLines("Plateau",
        [](void* data, int idx)
        {
          return Interpolation::Plateau::F(static_cast<float*>(data)[idx], 19, 1.0f);
        },
        ts, 40, 0, nullptr, FLT_MAX, FLT_MAX, ImVec2(200, 100));

      ImGui::InputFloat("a value", &Interpolation::Plateau::a, 1.0f, 1.0f, 5);
      ImGui::InputFloat("modifier value", &Interpolation::Plateau::modifier, 0.5f, 1.0f, 5);
      ImGui::InputFloat("distribution width value", &Interpolation::Plateau::d, 0.0000001f, 0.00001f, 10);
      ImGui::InputFloat("X axis offset", &Interpolation::Plateau::xAxisOffset, 0.001f, 0.01f, 5);
    }

    ImGui::EndGroup();
  });

  GUIController::Get().AddImguiWindowFunction("Main Debug Window", "Scene Selection",
    [&gm]()
    {
      const char* items[] = { "Start", "Character Select", "Battle", "Results" };
      static const char* current_item = NULL;
      auto func = [&gm](const std::string& i)
      {
        if (i == "Start")
          gm.RequestSceneChange(SceneType::START);
        else if (i == "Character Select")
          gm.RequestSceneChange(SceneType::CSELECT);
        else if (i == "Battle")
          gm.RequestSceneChange(SceneType::MATCH);
        else
          gm.RequestSceneChange(SceneType::RESULTS);
      };
      DropDown::Show(current_item, items, 4, func);
    });

  GUIController::Get().AddImguiWindowFunction("ECS Status", "Registered Components", []() {
    ImGui::Text("Components = %d", ECSGlobalStatus::NRegisteredComponents);
  });

  GUIController::Get().AddImguiWindowFunction("ECS Status", "Entity Snapshots",
    [&gm]()
    {
      auto p1 = gm.GetP1();
      auto p2 = gm.GetP2();

      if (ImGui::Button("Make P1 Snapshot"))
      {
        gm.TriggerBeginningOfFrame([&gm]() { p1Snapshots.push_back(gm.GetP1()->CreateEntitySnapshot()); });
      }
      if (ImGui::CollapsingHeader("P1 Snapshot List"))
      {
        for (int i = 0; i < (int)p1Snapshots.size(); i++)
        {
          std::string btnLabel = "SNAPSHOT " + std::to_string(i + 1);
          if (ImGui::Button(btnLabel.c_str()))
          {
            gm.TriggerBeginningOfFrame([&gm, i]() { gm.GetP1()->LoadEntitySnapshot(p1Snapshots[i]); });
          }
        }
      }

      if (ImGui::Button("Make P2 Snapshot"))
      {
        gm.TriggerBeginningOfFrame([&gm]() { p2Snapshots.push_back(gm.GetP2()->CreateEntitySnapshot()); });
      }
      if (ImGui::CollapsingHeader("P2 Snapshot List"))
      {
        for (int i = 0; i < (int)p2Snapshots.size(); i++)
        {
          std::string btnLabel = "SNAPSHOT " + std::to_string(i + 1);
          if (ImGui::Button(btnLabel.c_str()))
          {
            gm.TriggerBeginningOfFrame([&gm, i]() { gm.GetP2()->LoadEntitySnapshot(p2Snapshots[i]); });
          }
        }
      }

      if (ImGui::Button("Make Game State Snapshot"))
      {
        gm.TriggerBeginningOfFrame([&gm]() { gameStateSnapshots.push_back(gm.CreateGameStateSnapshot()); });
      }
      if (ImGui::CollapsingHeader("Game State Snapshot List"))
      {
        for (int i = 0; i < (int)gameStateSnapshots.size(); i++)
        {
          std::string btnLabel = "SNAPSHOT " + std::to_string(i + 1);
          if (ImGui::Button(btnLabel.c_str()))
          {
            gm.TriggerBeginningOfFrame([&gm, i]() { gm.LoadGamestateSnapshot(gameStateSnapshots[i]); });
          }
        }
      }

      ImGui::Separator();

      // Snapshot round-trip validation: save → load → save → compare
      static std::string validationResult;
      if (ImGui::Button("Validate Snapshot Round-Trip"))
      {
        gm.TriggerBeginningOfFrame([&gm]()
        {
          // 1. Save current state
          SBuffer snapA = gm.CreateGameStateSnapshot();

          // 2. Load that snapshot back (overwrites live state with deserialized data)
          gm.LoadGamestateSnapshot(snapA);

          // 3. Save again from the restored state
          SBuffer snapB = gm.CreateGameStateSnapshot();

          // 4. Compare byte-for-byte
          if (snapA.size() != snapB.size())
          {
            validationResult = "FAIL: size mismatch (A=" + std::to_string(snapA.size()) +
                               " B=" + std::to_string(snapB.size()) + ")";
          }
          else
          {
            bool match = true;
            size_t firstDiff = 0;
            for (size_t i = 0; i < snapA.size(); i++)
            {
              if (snapA[i] != snapB[i])
              {
                match = false;
                firstDiff = i;
                break;
              }
            }
            if (match)
              validationResult = "PASS: round-trip OK (" + std::to_string(snapA.size()) + " bytes)";
            else
              validationResult = "FAIL: divergence at byte " + std::to_string(firstDiff) +
                                 " of " + std::to_string(snapA.size());
          }
        });
      }
      if (!validationResult.empty())
      {
        bool pass = validationResult.find("PASS") != std::string::npos;
        if (pass)
          ImGui::TextColored(ImVec4(0, 1, 0, 1), "%s", validationResult.c_str());
        else
          ImGui::TextColored(ImVec4(1, 0, 0, 1), "%s", validationResult.c_str());
      }
    });

  static int localPlayerIndex = 0;

#ifdef _WIN32
  GUIController::Get().AddImguiWindowFunction("GGPO", "Connect Player",
    [&gm]()
    {
      ImGui::BeginGroup();

      if (!GGPOManager::Get().InMatch())
      {
        static int localUDPPort = static_cast<int>(NetGlobals::LocalUDPPort);
        if (ImGui::InputInt("Local Port", &localUDPPort))
          NetGlobals::LocalUDPPort = static_cast<unsigned short>(localUDPPort);

        ImGui::InputInt("Frame Delay", &NetGlobals::FrameDelay);

        static char ip[128];
        ImGui::InputText("Remote Player Address", ip, 128);

        static int port;
        ImGui::InputInt("Connection Port", &port);

        if (ImGui::Button("Connect On Position 1"))
        {
          InputHandlerSystem::AssignHandler(gm.GetP2()->GetID(), *gm.GetP2()->GetComponent<GameInputComponent>(), InputType::NetworkCtrl);
          localPlayerIndex = 0;
          std::string_view remoteIP = ip;
          unsigned short pport = (unsigned short)port;

          GGPOPlayer players[2] = { GGPOManager::Get().CreateLocalPlayer(), GGPOManager::Get().CreateRemotePlayer(remoteIP, pport) };
          players[0].player_num = 1;
          players[1].player_num = 2;

          GGPOManager::Get().BeginSession(players);
        }

        if (ImGui::Button("Connect On Position 2"))
        {
          InputHandlerSystem::AssignHandler(gm.GetP1()->GetID(), *gm.GetP1()->GetComponent<GameInputComponent>(), InputType::NetworkCtrl);
          localPlayerIndex = 1;
          std::string_view remoteIP = ip;
          unsigned short pport = (unsigned short)port;

          GGPOPlayer players[2] = { GGPOManager::Get().CreateRemotePlayer(remoteIP, pport), GGPOManager::Get().CreateLocalPlayer() };
          players[0].player_num = 1;
          players[1].player_num = 2;

          GGPOManager::Get().BeginSession(players);
        }
      }
      ImGui::EndGroup();
    });

  GUIController::Get().AddImguiWindowFunction("GGPO", "Network Stats", []()
    {
      ImGui::BeginGroup();

      if (GGPOManager::Get().InMatch())
      {
        for (int i = 0; i < 2; i++)
        {
          if (i == localPlayerIndex)
            continue;

          ImGui::BeginGroup();

          GGPONetworkStats stats = GGPOManager::Get().GetPlayerStats(i);
          ImGui::Text("P%d Stats\n", (i + 1));
          ImGui::Text("Ping: %d ms", stats.network.ping);
          ImGui::Text("Frame Lag: %.1f frames", stats.network.ping ? stats.network.ping * 60.0 / 1000 : 0);
          ImGui::Text("Bandwidth: %.2f kilobytes/sec", stats.network.kbps_sent / 8.0);
          ImGui::Text("Local Frames Behind: %d frames", stats.timesync.local_frames_behind);
          ImGui::Text("Remote Frames Behind: %d frames", stats.timesync.remote_frames_behind);

          ImGui::EndGroup();
        }
      }
      ImGui::EndGroup();
    });
#endif

  GUIController::Get().AddImguiWindowFunction("Main Debug Window", "FSM State",
    [&gm]()
    {
      auto showPlayerState = [](const char* label, std::shared_ptr<Entity> player)
      {
        if (!player) return;
        auto* fsm = player->GetComponent<FighterFSMComponent>();
        if (!fsm || !fsm->stateTable) return;
        auto* actor = player->GetComponent<GameActor>();
        auto* rb = player->GetComponent<Rigidbody>();
        auto* state = player->GetComponent<StateComponent>();

        const char* curName = FighterStateIDToString(fsm->currentState);
        const char* prevName = FighterStateIDToString(fsm->previousState);
        ImGui::Text("%s: %s", label, curName);
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "(prev: %s, frame %d/%d)",
                           prevName, fsm->stateFrame, fsm->stateTotalFrames);

        if (actor && rb && state)
        {
          ImGui::Text("  Input: 0x%02X  Grounded: %s  Side: %s  NewInputs: %s",
                       (unsigned char)actor->input.normal,
                       rb->IsGrounded() ? "Y" : "N",
                       state->onLeftSide ? "L" : "R",
                       actor->newInputs ? "Y" : "N");
        }
      };

      showPlayerState("P1", gm.GetP1());
      showPlayerState("P2", gm.GetP2());
    });

  CharacterEditor::Get().Initialize();

  GUIController::Get().AddImguiWindowFunction("Assets", "Sprite Sheets", []()
  {
    ImGui::BeginGroup();
    ResourceManager::Get().gSpriteSheets.DisplayInGUI();
    ResourceManager::Get().gSpriteSheets.DisplaySaveButton(SpriteSheet::SaveLocation());
    ImGui::EndGroup();
  });

  GUIController::Get().AddImguiWindowFunction("Assets", "General Animations", []()
    {
      if (ImGui::CollapsingHeader("General Animations"))
      {
        ImGui::BeginGroup();
        GAnimArchive.EditGeneralAnimations();
        ImGui::EndGroup();
      }
    });
}
