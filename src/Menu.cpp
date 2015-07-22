#include "Menu.h"
#include <curses.h>
#include "colors.h"
#include "Game.h"
#include <string>
#include <map>
#include "Ship.h"
#include "Server.h"

using namespace std;

enum properties_t {
	PROPERTY_GAMETYPE,
	PROPERTY_IPADDR_OR_AILEVEL,
	TOTAL_PROPERTIES
};

enum exproperties_t {
	PROPERTY_GIRDSIZE = TOTAL_PROPERTIES,
	TOTAL_EXPROPERTIES
};

CMenu::CMenu() : m_pos(0), m_hpos(0) {
	for(int i = 0; i < TOTAL_SHIPTYPES; i++) {
		m_ships[i] = CShip::BuildShip((shiptypes_t)i);
	}
}


CMenu::~CMenu() {
	for(int i = 0; i < TOTAL_SHIPTYPES; i++)
		delete m_ships[i];
}

void CMenu::Init(CGame * game) {
	m_game = game;
	m_hpos = game->GetIPAddr().length();
	Draw();
}

void CMenu::Open() {
	m_game->SetGameState(GAME_INMENU);
	m_game->SetStatus("Main menu", SPOS_TOP_LEFT);
	m_game->SetStatus("Use arrows to move and change settings. Enter to start. Esc to exit", SPOS_BOTTOM);
}

void CMenu::Draw() const {
	int y = 2;
	move(y + m_pos, 0);
	addch('>');

	const char * game_types[] = {
		"Player vs. AI",
		"Player vs. Player - Server",
		"Player vs. Player - Client"};
	gametypes_t gametype = m_game->GetGameType();
	mvprintw(y, 2, "Game type: %s", game_types[gametype]);
	y++;

	const char * ai_levels[] = {
		"Easy",
		"Normal",
		"Hard"};
	if(gametype == GT_PLAYER_VS_AI) {
		mvprintw(y, 2, "AI difficulty: %s", ai_levels[m_game->GetAILevel()]);
	}
	else {
		mvprintw(y, 2, "IP address and port of the server: %s", m_game->GetIPAddr().c_str());
		move(y, getcurx(stdscr) + m_hpos - m_game->GetIPAddr().length());
		attrset(COLOR_PAIR(BLACK_ON_WHITE));
		addch((char)winch(stdscr));
		attrset(COLOR_PAIR(WHITE_ON_BLACK));
	}
	y++;
	//you don't need more options as a client
	if(gametype == GT_PVP_CLIENT)
		return;

	mvprintw(y, 2, "Grid size: %dx%d", m_game->GetGridSize(), m_game->GetGridSize());
	y++;

	for(int i = 0; i < TOTAL_SHIPTYPES; i++) {
		mvprintw(y, 2, "%s: %dx", m_ships[i]->GetShipName().c_str(), m_game->GetShipCount((shiptypes_t)i));
		y++;
	}

	int shiptype = m_pos - TOTAL_EXPROPERTIES;
	if(shiptype < 0 || shiptype >= TOTAL_SHIPTYPES)
		return;

	mvprintw(y, 2, "Ship preview:");
	y += 2;
	CGrid preview(4);
	preview.Clear();
	TTmpPlacement tmpp;
	tmpp.m_ship = (shiptypes_t)shiptype;
	preview.Place(tmpp);
	move(y, 2);
	preview.Draw();
}

inline void mod_clamp(int & i, int min, int max) {
	if(max <= min)
		return;
	while(i < min) {
		i += max - min;
	}
	while(i >= max) {
		i -= max - min;
	}
}

void CMenu::ModifyProperty(int diff) {
	switch(m_pos) {
	case PROPERTY_GAMETYPE:
	{
		//Game type
		int gt = m_game->GetGameType() + diff;
		mod_clamp(gt, 0, TOTAL_GAMETYPES);
		m_game->SetGameType((gametypes_t)gt);
		break;
	}
	case PROPERTY_IPADDR_OR_AILEVEL:
	{
		if(m_game->GetGameType() == GT_PLAYER_VS_AI) {
			//AI difficulty
			int ail = m_game->GetAILevel() + diff;
			mod_clamp(ail, 0, TOTAL_AILEVELS);
			m_game->SetAILevel((ailevels_t)ail);
		}
		else {
			//IP address and port
			int iplen = m_game->GetIPAddr().length();
			m_hpos += diff;
			if(m_hpos < 0)
				m_hpos = 0;
			if(m_hpos > iplen)
				m_hpos = iplen;
		}
		break;
	}
	case PROPERTY_GIRDSIZE:
	{
		int gridsize = m_game->GetGridSize() + diff;
		if(gridsize < GRIDSIZE_MIN)
			gridsize = GRIDSIZE_MIN;
		if(gridsize > GRIDSIZE_MAX)
			gridsize = GRIDSIZE_MAX;
		m_game->SetGridSize(gridsize);
		if(!m_game->CanPlaceAll())
			m_game->SetGridSize(gridsize - diff);
		break;
	}
	default:
	{
		int st = m_pos - TOTAL_EXPROPERTIES;
		if(st < 0 || st >= TOTAL_SHIPTYPES)
			return;
		shiptypes_t type = (shiptypes_t)st;
		int count = m_game->GetShipCount(type) + diff;
		if(count < 0)
			return;
		m_game->SetShipCount(type, count);
		int totalcount = 0;
		for(int i = 0; i < TOTAL_SHIPTYPES; i++)
			totalcount += m_game->GetShipCount((shiptypes_t)i);
		if(!totalcount || !m_game->CanPlaceAll())
			m_game->SetShipCount(type, count - diff);
	}
	}
}

void CMenu::InsertIP(char c) {
	string ipaddr = m_game->GetIPAddr();
	m_game->SetIPAddr(ipaddr.substr(0, m_hpos) + c + ipaddr.substr(m_hpos));
	m_hpos++;
}

void CMenu::RemoveIP(bool backspace) {
	string ipaddr = m_game->GetIPAddr();
	if(backspace) {
		if(m_hpos) {
			m_game->SetIPAddr(ipaddr.substr(0, m_hpos - 1) + ipaddr.substr(m_hpos));
			m_hpos--;
		}
	}
	else {
		if(m_hpos < (int)ipaddr.length()) {
			m_game->SetIPAddr(ipaddr.substr(0, m_hpos) + ipaddr.substr(m_hpos + 1));
		}
	}
}

bool CMenu::MenuLoop(int c) {
	switch(c) {
	case KEY_ESC:
		return false;
	case KEY_ENTER:
	case KEY_NLINE:
		m_game->Start();
		return true;
	case KEY_DOWN:
		if(m_game->GetGameType() == GT_PVP_CLIENT) {
			if(m_pos < TOTAL_PROPERTIES - 1)
				m_pos++;
		}
		else {
			if(m_pos < (TOTAL_EXPROPERTIES + TOTAL_SHIPTYPES - 1))
				m_pos++;
		}
		break;
	case KEY_UP:
		if(m_pos > 0)
			m_pos--;
		break;
	case KEY_RIGHT:
		ModifyProperty(1);
		break;
	case KEY_LEFT:
		ModifyProperty(-1);
		break;
	case KEY_DC:	//delete
		if(m_game->GetGameType() != GT_PLAYER_VS_AI && m_pos == PROPERTY_IPADDR_OR_AILEVEL)
			RemoveIP(false);
		break;
	case KEY_BACKSPACE:
	case KEY_BSPACE:
		if(m_game->GetGameType() != GT_PLAYER_VS_AI && m_pos == PROPERTY_IPADDR_OR_AILEVEL)
			RemoveIP(true);
		break;
	default:
		if(m_game->GetGameType() != GT_PLAYER_VS_AI && m_pos == PROPERTY_IPADDR_OR_AILEVEL
			&& !((c < '0' || c > '9') && c != '.' && c != ':')) {
			InsertIP(c);
		}
	}
	Draw();
	return true;
}
