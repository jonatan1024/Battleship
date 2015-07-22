#include "Client.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <netdb.h>
#include <sstream>
#include <errno.h>

CClient::CClient(CGame * game) : m_game(game), m_state(CLIENT_UNINITED) {

}

CClient::~CClient() {

}

bool CClient::Connect(const string & ipaddr) {
	struct addrinfo * ai;
	string ip = ipaddr.substr(0, ipaddr.find(':'));
	string port = ipaddr.substr(ipaddr.find(':') + 1);

	if(getaddrinfo(ip.c_str(), port.c_str(), NULL, &ai) != 0)
		throw CClientError();

	if(m_socket == -1) {
		m_socket = socket(ai->ai_family, SOCK_STREAM, 0);
		if(m_socket == -1) {
			freeaddrinfo(ai);
			throw CClientError();
		}
		SetBlocking(false);
	}
	int connected;
	connected = connect(m_socket, ai->ai_addr, ai->ai_addrlen);
	if(connected == -1 && errno == EINPROGRESS) {
		freeaddrinfo(ai);
		return false;
	}
	if(connected == -1 && errno != EISCONN) {
		close(m_socket);
		freeaddrinfo(ai);
		throw CClientError();
	}
	

	freeaddrinfo(ai);

	m_state = CLIENT_WFOR_CONFIG;
	return true;
}

void CClient::ClientLoop() {
	string msg;
	if(ReadMessage(msg)) {
		switch(m_state) {
		case CLIENT_WFOR_CONFIG:
			ReadConfig(msg);
			break;
		case CLIENT_WFOR_ROUND:
			if(msg == "ok") {
				//first round
				m_game->SetGameState(GAME_INGAME);
			}
			else {
				//next rounds
				ReadShotInfo(msg);
			}
			break;
		default:
			break;
		}
	}
}

void CClient::ReadConfig(const string & msg) {
	stringstream msgstr(msg);
	string version;
	msgstr >> version;
	if(version != GAME_VERSION)
		throw CClientError();

	int gridsize;
	msgstr >> gridsize;
	m_game->SetGridSize(gridsize);

	TTmpPlacement tmpp;
	tmpp.m_y = tmpp.m_x = (gridsize - SHIP_BBOX_SIZE) / 2;

	for(int i = 0; i < TOTAL_SHIPTYPES; i++) {
		int count;
		msgstr >> count;
		m_game->SetShipCount((shiptypes_t)i, count);

		tmpp.m_rem[i] = count;
	}

	m_state = CLIENT_WFOR_ROUND;
	if(m_game->GetGameState() == GAME_WAITING) {
		m_game->SetGameState(GAME_INPLACEMENT);
		m_game->SetStatus("Ship placement", SPOS_TOP_LEFT);
		m_game->SetStatus("Arrows - move. Space - rotate. Enter - place. Tab - skip. Esc - exit.", SPOS_BOTTOM);

		for(int i = 0; i < TOTAL_SHIPTYPES; i++) {
			if(tmpp.m_rem[i]) {
				tmpp.m_ship = (shiptypes_t)i;
				break;
			}
		}
		m_game->SetTmpPlcmnt(tmpp);
	}
}

void CClient::SendShot(int x, int y) {
	stringstream ss;
	ss << (y*m_game->GetGridSize() + x);
	SendMessage(ss.str());
	m_game->SetGameState(GAME_WAITING);
	m_game->SetStatus("Waiting for enemy's turn", SPOS_TOP_LEFT);
}

void CClient::ReadShotInfo(const string & msg) {
	stringstream ss(msg);
	string gameresult;
	ss >> gameresult;
	int gs = m_game->GetGridSize();
	for(int i = 1; i >= 0; i--) {
		int numsunk;
		ss >> numsunk;
		for(int j = 0; j < numsunk; j++) {
			int pos;
			int content;
			ss >> pos;
			ss >> content;
			m_game->GetGrid((gamegrids_t)i).SetFieldContent(pos % gs, pos / gs, (fieldcontents_t)content);
		}
	}
	if(gameresult == "ok") {
		m_game->SetGameState(GAME_INGAME);
	}
	else {
		m_game->SetGameState(GAME_GAMEOVER);
		m_game->SetStatus("Game is over, press Esc to return to menu.", SPOS_BOTTOM);
		if(gameresult == "won") {
			m_game->SetStatus("You won!", SPOS_TOP_LEFT);
		}
		else if(gameresult == "lost") {
			m_game->SetStatus("You lost!", SPOS_TOP_LEFT);
		}
		else {
			m_game->SetStatus("It's a draw!", SPOS_TOP_LEFT);
		}
	}
}
