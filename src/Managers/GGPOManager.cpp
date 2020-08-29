#include "Managers/GGPOManager.h"
#include "Managers/GameManagement.h"
#include "Core/InputState.h"

// for log function
#include <fstream>

//______________________________________________________________________________
int fletcher32_checksum(short* data, size_t len)
{
  int sum1 = 0xffff, sum2 = 0xffff;

  while (len) {
    size_t tlen = len > 360 ? 360 : len;
    len -= tlen;
    do {
      sum1 += *data++;
      sum2 += sum1;
    } while (--tlen);
    sum1 = (sum1 & 0xffff) + (sum1 >> 16);
    sum2 = (sum2 & 0xffff) + (sum2 >> 16);
  }

  /* Second reduction step to reduce sums to 16 bits */
  sum1 = (sum1 & 0xffff) + (sum1 >> 16);
  sum2 = (sum2 & 0xffff) + (sum2 >> 16);
  return sum2 << 16 | sum1;
}

//______________________________________________________________________________
void GGPOManager::Player::SetConnectionState(ConnectionState nState)
{
  progress = 0;
  state = nState;
}

//______________________________________________________________________________
void GGPOManager::Player::SetDisconnectionTimeout(int when, int timeout)
{
  disconnectionStart = when;
  disconnectionTimeout = timeout;
  state = ConnectionState::Disconnecting;
}

//______________________________________________________________________________
void GGPOManager::Player::UpdateConnectProgress(int p)
{
  progress = p;
}

//______________________________________________________________________________
GGPOPlayer GGPOManager::CreateLocalPlayer()
{
  GGPOPlayer player;
  player.size = sizeof(GGPOPlayer);
  player.type = GGPO_PLAYERTYPE_LOCAL;
  return player;
}

//______________________________________________________________________________
GGPOPlayer GGPOManager::CreateRemotePlayer(std::string_view ip, unsigned short port)
{
  GGPOPlayer player;
  player.size = sizeof(GGPOPlayer);
  player.type = GGPO_PLAYERTYPE_REMOTE;

  // do size + 1 to save the null terminator
  memcpy_s(player.u.remote.ip_address, 32, ip.data(), (rsize_t)ip.size() + 1);
  player.u.remote.port = port;

  return player;
}

//______________________________________________________________________________
void GGPOManager::BeginSession(GGPOPlayer* players)
{
  if (_playingOnline)
    return;

  // start up win sockets which will get cleaned up on exit
  WSADATA wd = { 0 };
  WSAStartup(MAKEWORD(2, 2), &wd);

  GGPOErrorCode result = GGPO_OK;
#if defined(GGPO_SYNC_TEST)
  char name[5] = { 'D', 'u', 'e', 'l', '\0' };
  result = ggpo_start_synctest(&_session, &_callbacks, name, NUM_PLAYERS, sizeof(InputState), 1);
#else
  /* Start a new session */
   result = ggpo_start_session(&_session, &_callbacks,
    "Duel",    // application name
    NUM_PLAYERS,             // 2 players
    sizeof(InputState),   // size of an input packet
    NetGlobals::LocalUDPPort);         // our local udp port
#endif

  // automatically disconnect clients after 3000 ms and start our count-down timer
  // for disconnects after 1000 ms.   To completely disable disconnects, simply use
  // a value of 0 for ggpo_set_disconnect_timeout.
  ggpo_set_disconnect_timeout(_session, 3000);
  ggpo_set_disconnect_notify_start(_session, 1000);

  for (int i = 0; i < NUM_PLAYERS; i++)
  {
    GGPOPlayerHandle handle;
    result = ggpo_add_player(_session, players + i, &handle);
    _playerInfo[i].handle = handle;
    _playerInfo[i].type = players[i].type;
    if (players[i].type == GGPO_PLAYERTYPE_LOCAL)
    {
      _playerInfo[i].progress = 100;
      GetPlayer(handle)->SetConnectionState(ConnectionState::Connecting);
      ggpo_set_frame_delay(_session, handle, NetGlobals::FrameDelay);

      // set local player state values
      _localPlayerHandle = handle;
      _localPlayerIndex = i;
    }
    else
    {
      _playerInfo[i].progress = 0;

      // set remote player state values
      _remotePlayerIndex = i;
    }
  }

  if (GGPO_SUCCEEDED(result))
    _playingOnline = true;

}

//______________________________________________________________________________
void GGPOManager::ExitSession()
{
  if (_session)
  {
    _playingOnline = false;
    ggpo_close_session(_session);
    _session = nullptr;

    WSACleanup();
  }
}

//______________________________________________________________________________
GGPOManager::Player* GGPOManager::GetPlayer(GGPOPlayerHandle handle)
{
  for (int i = 0; i < NUM_PLAYERS; i++)
  {
    if (_playerInfo[i].handle == handle)
      return &_playerInfo[i];
  }
  return nullptr;
}

//______________________________________________________________________________
void GGPOManager::SetState(ConnectionState state)
{
  for (int i = 0; i < NUM_PLAYERS; i++)
  {
    _playerInfo[i].SetConnectionState(state);
  }
}

//______________________________________________________________________________
void GGPOManager::CallUpdate()
{
  InputState inputs[NUM_PLAYERS] = { InputState::NONE };
  int disconnectFlags = 0;

  ggpo_synchronize_input(_session, (void*)inputs, NUM_PLAYERS * sizeof(InputState), &disconnectFlags);

  // replace inputs with the synced inputs
  GameManager::Get().SyncPlayerInputs(inputs);

  // move game forward by 1/60th of a second
  GameManager::Get().Update(secPerFrame);
}

//______________________________________________________________________________
void GGPOManager::NotifyAdvanceFrame()
{
  ggpo_advance_frame(_session);
}

//______________________________________________________________________________
bool GGPOManager::SyncInputs(InputState* input)
{
  GGPOErrorCode result = GGPO_OK;
  int disconnectFlags;

  if (_localPlayerHandle != GGPO_INVALID_HANDLE)
  {
    result = ggpo_add_local_input(_session, _localPlayerHandle, &input[_localPlayerIndex], sizeof(InputState));
  }

  if (GGPO_SUCCEEDED(result))
  {
    result = ggpo_synchronize_input(_session, (void*)input, NUM_PLAYERS * sizeof(InputState), &disconnectFlags);
    return GGPO_SUCCEEDED(result);
  }
  return false;
}

//______________________________________________________________________________
GGPONetworkStats GGPOManager::GetPlayerStats(int index)
{
  GGPONetworkStats stats = { 0 };
  ggpo_get_network_stats(_session, _playerInfo[index].handle, &stats);
  return stats;
}


//______________________________________________________________________________
GGPOManager::GGPOManager() : _session(nullptr), _callbacks()
{
  //fill out the rest of the function pointers later...
  _callbacks.begin_game = &BeginGame;
  _callbacks.advance_frame = &AdvanceFrame;
  _callbacks.load_game_state = &LoadGameState;
  _callbacks.free_buffer = &FreeBuffer;
  _callbacks.log_game_state = &LogGameState;
  _callbacks.save_game_state = &SaveGameState;
  _callbacks.on_event = &OnEvent;
}

//______________________________________________________________________________
GGPOManager::~GGPOManager()
{
  ExitSession();
}

//______________________________________________________________________________
bool BeginGame(const char* game)
{
  return true;
}

//______________________________________________________________________________
bool SaveGameState(unsigned char** buffer, int* len, int* checksum, int frame)
{
  SBuffer gamestate = GameManager::Get().CreateGameStateSnapshot();
  size_t size = gamestate.size();

  // set data length parameter
  *len = static_cast<int>(size);

  //copy the contents
  *buffer = new unsigned char[size];
  if (!*buffer)
    return false;

  memcpy_s(*buffer, size, gamestate.data(), size);

  // using basic checksum from the ggpo example
  *checksum = fletcher32_checksum((short*)*buffer, *len / 2);

  return true;
}

//______________________________________________________________________________
bool LoadGameState(unsigned char* buffer, int len)
{
  SBuffer gamestate;
  size_t dataSize = static_cast<size_t>(len);
  gamestate.resize(dataSize);

  memcpy_s(gamestate.data(), dataSize, buffer, len);

  GameManager::Get().LoadGamestateSnapshot(gamestate);

  return true;
}

//______________________________________________________________________________
bool LogGameState(char* filename, unsigned char* buffer, int len)
{
  std::ofstream out(filename);

  // write contents to the stream
  std::stringstream stream;
  stream.write((const char*)buffer, len);

  // get the scene the snapshot was written in
  SceneType snapshotScene;
  Serializer<SceneType>::Deserialize(stream, snapshotScene);

  out << "State in Scene: " << (int)snapshotScene << "\n";

  bool frameStopActive;
  int frameStop = 0;
  Serializer<bool>::Deserialize(stream, frameStopActive);
  Serializer<int>::Deserialize(stream, frameStop);

  out << "Frame stop active: " << frameStopActive << " for " << frameStop << " frames\n";

  while (!stream.eof())
  {
    EntityID cpID = 0;
    Serializer<EntityID>::Deserialize(stream, cpID);

    if (stream.eof())
      break;

    std::cout << "Logging State for Entity :" << cpID << "\n";

    std::shared_ptr<Entity> entity = GameManager::Get().CreateEntity<>();
    entity->Deserialize(stream);

    auto signature = entity->GetSignature();

    std::cout << "Signature for entity: " << signature << "\n";
    /*std::cout << "Checking signature attachments via signature test: \n";
    
    for (int i = 0; i < signature.size(); i++)
    {
      if (signature.test(i))
      {
        std::cout << "\t" << ECSCoordinator::Get().GetComponentName(i) << "\n";
      }
    }*/

    std::cout << "Checking signature attachements via ECSGlobalStatus: \n";
    for (size_t compIndex = 0; compIndex < ECSGlobalStatus::NRegisteredComponents; compIndex++)
    {
      if (signature.test(compIndex))
      {
        std::cout << "\t" << ECSCoordinator::Get().GetComponentName(compIndex) << "\n";
      }
    }

    out << entity->Log();

    GameManager::Get().DestroyEntity(entity->GetID());
  }

  return true;
}

//______________________________________________________________________________
void FreeBuffer(void* buffer)
{
  delete[] buffer;
}

//______________________________________________________________________________
bool AdvanceFrame(int flags)
{
  GGPOManager::Get().CallUpdate();
  return true;
}

bool OnEvent(GGPOEvent* info)
{
  // handle event enum
  int progress;
  switch (info->code) {
  case GGPO_EVENTCODE_CONNECTED_TO_PEER:
    GGPOManager::Get().GetPlayer(info->u.connected.player)->SetConnectionState(ConnectionState::Synchronizing);
    break;
  case GGPO_EVENTCODE_SYNCHRONIZING_WITH_PEER:
    progress = 100 * info->u.synchronizing.count / info->u.synchronizing.total;
    GGPOManager::Get().GetPlayer(info->u.synchronizing.player)->UpdateConnectProgress(progress);
    break;
  case GGPO_EVENTCODE_SYNCHRONIZED_WITH_PEER:
    GGPOManager::Get().GetPlayer(info->u.synchronized.player)->UpdateConnectProgress(100);
    break;
  case GGPO_EVENTCODE_RUNNING:
    GGPOManager::Get().SetState(ConnectionState::Running);
    break;
  case GGPO_EVENTCODE_CONNECTION_INTERRUPTED:
    GGPOManager::Get().GetPlayer(info->u.connection_interrupted.player)->SetDisconnectionTimeout(SDL_GetTicks(), info->u.connection_interrupted.disconnect_timeout);
    break;
  case GGPO_EVENTCODE_CONNECTION_RESUMED:
    GGPOManager::Get().GetPlayer(info->u.connection_resumed.player)->SetConnectionState(ConnectionState::Running);
    break;
  case GGPO_EVENTCODE_DISCONNECTED_FROM_PEER:
    GGPOManager::Get().GetPlayer(info->u.disconnected.player)->SetConnectionState(ConnectionState::Disconnected);
    break;
  case GGPO_EVENTCODE_TIMESYNC:
    Sleep(1000 * info->u.timesync.frames_ahead / 60);
    break;
  }
  return true;
}
