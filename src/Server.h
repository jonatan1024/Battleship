#pragma once
#include <string>
#include "Game.h"
#include "Socket.h"
using namespace std;

/// Server error exception.
class CServerError : public CSocketError {};

/// States in which server can be in.
enum serverstates_t {
	SERVER_UNINITED,
	SERVER_ACCEPTING,
	SERVER_FRESH,
	SERVER_INPLACEMENT,
	SERVER_INGAME,
	SERVER_DONE,
	TOTAL_SERVERSTATES
};

/// Server, handles communication between Clients.
class CServer : public CSocket {
public:
	CServer(CGame * game);
	~CServer();
	/// @param ipaddr IP address and port of the server.
	///	@throws CServerError On connection failure.
	void Start(const string & ipaddr);
	/// Main Server loop, delegates current job to proper functions.
	void ServerLoop();
private:
	/// Wait for clients to connect.
	/// Then send them configs.
	void AcceptClients();
	/// Send game configurations to clients.
	void SendConfig();
	/// Receive fleet placements from clients.
	/// When are all placements received, start the game.
	void ReadPlacement();
	/// Receive clients' targets.
	/// When are all targets received, start next round.
	void ReadShot();
	/// Pointer to the game we're playing.
	CGame * m_game;
	/// Sockets for communication with clients.
	CSocket m_clients[2];
	/// Grid for each client.
	CGrid m_grids[2];
	/// Info about client's last shot.
	TShotInfo m_lastshot[2];
	/// Current state of the server.
	serverstates_t m_state;
};

