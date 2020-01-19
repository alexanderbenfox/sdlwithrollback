#ifdef _WIN32

#include "GameState/GGPOGameState.h"
#include "Entity.h"
#include "Components/GameActor.h"

GGPOMatch::GGPOMatch(IInputHandler<SDL_Event>* localHandler, const std::string& remoteAddress, unsigned short port) :
  _player1(EntityCreation::CreateLocalPlayer(localHandler, 0)),
  _player2(EntityCreation::CreateNetworkPlayer(static_cast<IInputHandler<GGPOInput>*>(new GGPOInputHandler), 150)),
  MatchBase()
{
  StartGGPOSession();

  _ggpoP1.size = _ggpoP2.size = sizeof(GGPOPlayer);
  _ggpoP1.type = GGPO_PLAYERTYPE_LOCAL;
  _ggpoP2.type = GGPO_PLAYERTYPE_REMOTE;

  strcpy_s(_ggpoP2.u.remote.ip_address, remoteAddress.c_str());
  _ggpoP2.u.remote.port = port;

  GGPOErrorCode result = ggpo_add_player(ggpo, &_ggpoP1, &_handles[0]);
  result = ggpo_add_player(ggpo, &_ggpoP2, &_handles[1]);
}

void GGPOMatch::ProcessInputs(SDL_Event* localInput)
{
  InputState input[2];
  input[0] = _player1.input->HandleInput(localInput);

  GGPOInput inputPkg;
  inputPkg.session = ggpo;
  inputPkg.inputs = (InputState**)&input;
  inputPkg.handles = (GGPOPlayerHandle**)&_handles;

  _player2.input->HandleInput(&inputPkg);

  ICommand* p1Command = new InGameCommand(input[0]);
  ICommand* p2Command = new InGameCommand(input[1]);

  _player1.ExecuteInput(p1Command);
  _player2.ExecuteInput(p2Command);

  delete p1Command;
  delete p2Command;
}


void GGPOMatch::StartGGPOSession()
{
  //fill out the rest of the function pointers later...
  cb.begin_game = &GGPOMatch::BeginGame;
  cb.advance_frame = &GGPOMatch::AdvanceFrame;
  cb.load_game_state = &GGPOMatch::LoadGameState;
  cb.free_buffer = &GGPOMatch::FreeBuffer;
  cb.log_game_state = &GGPOMatch::LogGameState;
  cb.save_game_state = &GGPOMatch::SaveGameState;
  cb.on_event = &GGPOMatch::OnEvent;

  /* Start a new session */
  result = ggpo_start_session(&ggpo, &cb,
    "test",    // application name
    2,             // 2 players
    sizeof(InputState),   // size of an input packet
    8001);         // our local udp port
}

bool GGPOMatch::BeginGame(const char* game) { return true; }

bool GGPOMatch::SaveGameState(unsigned char** buffer, int* len, int* checksum, int frame) { return true; }

bool GGPOMatch::LoadGameState(unsigned char* buffer, int len) { return true; }

bool GGPOMatch::LogGameState(char* filename, unsigned char* buffer, int len) { return true; }

void GGPOMatch::FreeBuffer(void* buffer) {}

bool GGPOMatch::AdvanceFrame(int flags) { return true; }

bool GGPOMatch::OnEvent(GGPOEvent* info) { return true; }

#endif