#ifdef _WIN32
#include "GameState.h"
#include <functional>

class GGPOMatch : public MatchBase
{
public:
  GGPOMatch(IInputHandler<SDL_Event>* localHandler, const std::string& remoteAddress, unsigned short port);

  virtual void ProcessRawInputs(SDL_Event* localInput) override;
  

  void StartGGPOSession();

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

protected:
  // GGPO Crap
  GGPOSession* ggpo;
  GGPOErrorCode result;
  GGPOSessionCallbacks cb;

  GGPOPlayer _ggpoP1, _ggpoP2;
  GGPOPlayerHandle _handles[2];

  LocalPlayer _player1;
  NetworkPlayer _player2;
};

#endif