#pragma once
#include "Menu.h"
#include <string>
#include "Ship.h"
#include "Grid.h"

using namespace std;

/// Game version string to avoid possible incompatibilities in network communication.
#define GAME_VERSION "submit"

/// Game types - PvAI / PvP.
enum gametypes_t {
	/// CClient & CServer & CAI
	GT_PLAYER_VS_AI,
	/// CClient & CServer
	GT_PVP_SERVER,
	/// CClient
	GT_PVP_CLIENT,
	TOTAL_GAMETYPES
};

/// Levels of A. Inteligence.
enum ailevels_t {
	/// CEasyAI
	AIL_EASY,
	/// CMediumAI
	AIL_MEDIUM,
	/// CHardAI
	AIL_HARD,
	TOTAL_AILEVELS
};

/// States in which game can be in.
enum gamestates_t {
	GAME_UNINITED,
	GAME_INMENU,
	GAME_WAITING,
	GAME_INPLACEMENT,
	GAME_INGAME,
	GAME_INERROR,
	GAME_GAMEOVER,
	TOTAL_GAMESTATES
};

/// Status bars.
enum statuspos_t {
	SPOS_TOP_LEFT,
	SPOS_TOP_RIGHT,
	SPOS_BOTTOM,
	TOTAL_STATUSPOS
};

/// Game grids.
enum gamegrids_t {
	GG_LEFT,
	GG_RIGHT,
	TOTAL_GAMEGRIDS
};

#define GRIDSIZE_MAX 18
#define GRIDSIZE_MIN 6

/// Defualt localhost address and port
#define DEFAULT_IPADDR "127.0.0.1:2015"
#define DEFAULT_GRIDSIZE 10
/// Default amount of each ship type.
#define DEFAULT_SHIPCOUNT 2

class CServer;
class CClient;
class CAI;

/// Game, main class that manages everything.
class CGame {
public:
	CGame();
	~CGame();
	/// Inits the game and opens menu.
	void Init();
	/// Main game loop, runs all other loops (menu, server, client, ai, ...).
	/// @return False when exitting.
	bool GameLoop();
	/// Stars a new game, starts server/client/ai according to the configuration.
	void Start();
	/// Sets a message into specified status bar.
	void SetStatus(const string & status, statuspos_t pos);
	/// Puts game into error state and sets an error message.
	void SetError(const string & status);
	/// Tries to place the fleet into game's grid.
	/// @return True on success.
	bool CanPlaceAll();
	/// Tries to place the fleet into a grid.
	/// @param grid Grid to insert the fleet into.
	/// @return True on success.
	bool CanPlaceAll(CGrid& grid) const;

	gametypes_t GetGameType() const;
	void SetGameType(gametypes_t gt);
	ailevels_t GetAILevel() const;
	void SetAILevel(ailevels_t ail);
	string GetIPAddr() const;
	void SetIPAddr(string ip);
	int GetGridSize() const;
	void SetGridSize(int gs);
	int GetShipCount(shiptypes_t type) const;
	void SetShipCount(shiptypes_t type, int count);
	gamestates_t GetGameState() const;
	void SetGameState(gamestates_t state);
	void SetTmpPlcmnt(TTmpPlacement p);
	CGrid & GetGrid(gamegrids_t c);
	const CGrid & GetGrid(gamegrids_t c) const;
private:
	/// Draws status bars.
	void DrawStatus() const;
	/// Disconnects and closes active game, returns to menu.
	/// @param error True on error-disconnect (not invoked by player).
	void Disconnect(bool error = false);
	/// Ship placement loop.
	/// @param c Keycode.
	void HandlePlacement(int c);
	/// Turn (target selecting and shooting) loop.
	/// @param c Keycode.
	void HandleTurn(int c);

	gamestates_t m_gamestate;
	CMenu m_menu;
	/// Stats bars
	string m_status[TOTAL_STATUSPOS];
	gametypes_t m_gametype;
	ailevels_t m_ailevel;
	string m_ipaddr;
	int m_gridsize;
	/// Amount of ships of each type.
	int m_shipcount[TOTAL_SHIPTYPES];
	CServer * m_server;
	CClient * m_client;
	CAI * m_ai;
	CGrid m_grids[TOTAL_GAMEGRIDS];
	/// Temporary placement - used when placing ships and selecting targets.
	TTmpPlacement m_tmpplacement;
};

