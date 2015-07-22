#pragma once
#include "Game.h"
#include <string>
#include "Socket.h"
#include "Grid.h"
using namespace std;

/// Client error exception.
class CClientError : public CSocketError {};

/// States in which client can be in.
enum clientstates_t {
	CLIENT_UNINITED,
	CLIENT_OK,
	CLIENT_WFOR_CONFIG,
	CLIENT_WFOR_ROUND,
	CLIENT_WFOR_HINFO,
	TOTAL_CLIENTSTATES
};

/// Client, a layer between Socket and Game.
class CClient : public CSocket {
public:
	CClient(CGame * game);
	~CClient();
	/// Connects into a game.
	/// @param ipaddr Address to connect to.
	/// @throws CClientError On connection failure.
	/// @return True when connection is ready.
	bool Connect(const string & ipaddr);
	/// Main Client loop, delegates current job to proper functions.
	void ClientLoop();
	/// Sends a message containing position of client's target.
	/// @param x Target's coordinate.
	/// @param y Target's coordinate.
	void SendShot(int x, int y);
private:
	/// Read and apply game settings.
	void ReadConfig(const string & msg);
	/// Read hit information and store it into Game's grids.
	void ReadShotInfo(const string & msg);
protected:
	/// Pointer to the game we're connected to.
	CGame * m_game;
	/// Current state of the client.
	clientstates_t m_state;
};

