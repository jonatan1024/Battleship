#include "Server.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <netdb.h>
#include <sstream>
#include "Ship.h"
#include <signal.h>

CServer::CServer(CGame * game) : m_game(game), m_state(SERVER_UNINITED) {
	m_lastshot[0].m_pos = -1;
	m_lastshot[1].m_pos = -1;
}

CServer::~CServer() {
}

void CServer::Start(const string & ipaddr) {

	struct addrinfo * ai;
	string ip = ipaddr.substr(0, ipaddr.find(':'));
	string port = ipaddr.substr(ipaddr.find(':') + 1);

	if(getaddrinfo(ip.c_str(), port.c_str(), NULL, &ai) != 0) {
		throw CServerError();
	}

	m_socket = socket(ai->ai_family, SOCK_STREAM, 0);

	if(m_socket == -1) {
		freeaddrinfo(ai);
		throw CServerError();
	}

	int reuse = 1;
	if(setsockopt(m_socket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
		close(m_socket);
		freeaddrinfo(ai);
		throw CServerError();
	}

	if(bind(m_socket, ai->ai_addr, ai->ai_addrlen) != 0) {
		close(m_socket);
		freeaddrinfo(ai);
		throw CServerError();
	}

	if(listen(m_socket, 10) != 0) {
		close(m_socket);
		freeaddrinfo(ai);
		throw CServerError();
	}
	freeaddrinfo(ai);

	SetBlocking(false);

	m_state = SERVER_ACCEPTING;
}

void CServer::AcceptClients() {
	int done = 0;
	for(int i = 0; i < 2; i++) {
		if(m_clients[i]) {
			done++;
			continue;
		}
		struct sockaddr addr;
		socklen_t addrLen = sizeof(addr);

		m_clients[i] = accept(m_socket, &addr, &addrLen);
		m_clients[i].SetBlocking(false);
	}
	if(done == 2)
		m_state = SERVER_FRESH;
}

void CServer::ReadPlacement() {
	int done = 0;
	for(int i = 0; i < 2; i++) {
		if(m_grids[i].GetSize() == m_game->GetGridSize()) {
			done++;
			continue;
		}
		string grid;
		if(m_clients[i].ReadMessage(grid)) {
			m_grids[i].Deserialize(grid);
		}
	}
	if(done == 2) {
		m_state = SERVER_INGAME;
		m_clients[0].SendMessage("ok");
		m_clients[1].SendMessage("ok");
	}
}

void CServer::ServerLoop() {
	switch(m_state) {
	case SERVER_ACCEPTING:
	{
		AcceptClients();
		break;
	}
	case SERVER_FRESH:
	{
		SendConfig();
		break;
	}
	case SERVER_INPLACEMENT:
	{
		ReadPlacement();
		break;
	}
	case SERVER_INGAME:
	{
		ReadShot();
		break;
	}
	default:
		break;
	}
}

void CServer::SendConfig() {
	stringstream msg;
	msg << GAME_VERSION << endl;
	msg << m_game->GetGridSize() << endl;
	for(int i = 0; i < TOTAL_SHIPTYPES; i++) {
		msg << m_game->GetShipCount((shiptypes_t)i) << " ";
	}
	msg << endl;
	for(int i = 0; i < 2; i++) {
		m_clients[i].SendMessage(msg.str());
	}
	m_state = SERVER_INPLACEMENT;
}

void CServer::ReadShot() {
	int done = 0;
	for(int i = 0; i < 2; i++) {
		if(m_lastshot[i].m_pos >= 0) {
			done++;
			continue;
		}
		string msg;
		if(!m_clients[i].ReadMessage(msg))
			continue;
		stringstream ss(msg);
		int pos;
		ss >> pos;
		int i2 = 1 - i;
		if(pos >= m_grids[i2].GetSize()*m_grids[i2].GetSize())
			continue;
		m_lastshot[i].m_pos = pos;
		m_grids[i2].Shoot(m_lastshot[i]);
	}
	if(done == 2) {
		stringstream ss[2];
		bool gameover = false;
		if(m_lastshot[0].m_defeat && m_lastshot[1].m_defeat) {
			ss[0] << "draw" << endl;
			ss[1] << "draw" << endl;
			gameover = true;
		}else if(m_lastshot[0].m_defeat) {
			ss[0] << "won" << endl;
			ss[1] << "lost" << endl;
			gameover = true;
		}
		else if(m_lastshot[1].m_defeat) {
			ss[0] << "lost" << endl;
			ss[1] << "won" << endl;
			gameover = true;
		}
		else {
			ss[0] << "ok" << endl;
			ss[1] << "ok" << endl;
		}
		for(int i = 0; i < 2; i++) {
			ss[i] << m_lastshot[i].m_sunk.size() << endl;
			for(int sunkPos : m_lastshot[i].m_sunk) {
				ss[i] << sunkPos << " " << m_lastshot[i].m_content << endl;
			}

			int i2 = 1 - i;

			ss[i] << m_lastshot[i2].m_sunk.size() << endl;
			for(int sunkPos : m_lastshot[i2].m_sunk) {
				ss[i] << sunkPos << " " << m_lastshot[i2].m_content << endl;
			}

			m_clients[i].SendMessage(ss[i].str());
		}
		if(gameover) {
			m_state = SERVER_DONE;
		}
		m_lastshot[0].m_pos = -1;
		m_lastshot[1].m_pos = -1;
	}
}
