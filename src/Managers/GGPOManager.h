#pragma once
#ifdef _WIN32
#include <GGPO/ggponet.h>
#include <vector>
#include <string_view>

#include "Core/InputState.h"

#define NUM_PLAYERS 2

/*
* Simple checksum function stolen from wikipedia:
*
*   http://en.wikipedia.org/wiki/Fletcher%27s_checksum
*/

int fletcher32_checksum(short* data, size_t len);

enum class ConnectionState
{
  Connecting = 0,
  Synchronizing,
  Running,
  Disconnected,
  Disconnecting
};

class GGPOManager
{
public:

  struct Player
  {
    void SetConnectionState(ConnectionState nState);
    void SetDisconnectionTimeout(int when, int timeout);
    void UpdateConnectProgress(int p);

    GGPOPlayerType type;
    GGPOPlayerHandle handle;
    ConnectionState state;
    int progress = 0;
    int disconnectionTimeout = 0;
    int disconnectionStart = 0;

  };

  static GGPOManager& Get()
  {
    static GGPOManager instance;
    return instance;
  }

  //!
  GGPOPlayer CreateLocalPlayer();
  GGPOPlayer CreateRemotePlayer(std::string_view ip, unsigned short port);

  //! Call ggpo_start_session with local udp port 8001
  void BeginSession(GGPOPlayer* players);
  //! Destroys session object and sets InMatch status to false
  void ExitSession();
  //! Gets a player by handle
  Player* GetPlayer(GGPOPlayerHandle handle);
  //! Sets state of all players to a certain connection state
  void SetState(ConnectionState state);

  //! used by ggpo advance frame callback to sync inputs and advance frame
  void CallUpdate();
  //! used by game manager to notify GGPO state that frame has been updated
  void NotifyAdvanceFrame();
  //! returns true if frame should advance. localInput should be an array of NUM_PLAYERS length
  bool SyncInputs(InputState* input);
  //! amount of time in milliseconds (i think) that GGPO.net can spend doing work this cycle
  void Idle(int time) { ggpo_idle(_session, time); }
  //! returns true if we are in a ggpo match
  bool InMatch() const { return _playingOnline; }
  //!
  GGPONetworkStats GetPlayerStats(int index);

private:

  //! Sets up callbacks
  GGPOManager();
  //! Calls exit
  ~GGPOManager();


  GGPOSession* _session;
  GGPOSessionCallbacks _callbacks;

  Player _playerInfo[NUM_PLAYERS];
  GGPOPlayerHandle _localPlayerHandle;

  bool _playingOnline = false;
  int _localPlayerIndex = 0;
  int _remotePlayerIndex = 0;

};


/*begin_game callback - This callback has been deprecated.You must
* implement it, but should ignore the 'game' parameter.
*/
static bool BeginGame(const char* game);

/*
 * save_game_state - The client should allocate a buffer, copy the
 * entire contents of the current game state into it, and copy the
 * length into the *len parameter.  Optionally, the client can compute
 * a checksum of the data and store it in the *checksum argument.
 */
static bool SaveGameState(unsigned char** buffer, int* len, int* checksum, int frame);

/*
 * load_game_state - GGPO.net will call this function at the beginning
 * of a rollback.  The buffer and len parameters contain a previously
 * saved state returned from the save_game_state function.  The client
 * should make the current game state match the state contained in the
 * buffer.
 */
static bool LoadGameState(unsigned char* buffer, int len);

/*
 * log_game_state - Used in diagnostic testing.  The client should use
 * the ggpo_log function to write the contents of the specified save
 * state in a human readible form.
 */
static bool LogGameState(char* filename, unsigned char* buffer, int len);

/*
 * free_buffer - Frees a game state allocated in save_game_state.  You
 * should deallocate the memory contained in the buffer.
 */
static void FreeBuffer(void* buffer);

/*
 * advance_frame - Called during a rollback.  You should advance your game
 * state by exactly one frame.  Before each frame, call ggpo_synchronize_input
 * to retrieve the inputs you should use for that frame.  After each frame,
 * you should call ggpo_advance_frame to notify GGPO.net that you're
 * finished.
 *
 * The flags parameter is reserved.  It can safely be ignored at this time.
 */
static bool AdvanceFrame(int flags);

/*
 * on_event - Notification that something has happened.  See the GGPOEventCode
 * structure above for more information.
 */
static bool OnEvent(GGPOEvent* info);

#endif // _WIN32
