#include "Game.h"
#include <curses.h>
#include "colors.h"
#include "Server.h"
#include "Client.h"
#include "AI.h"

CGame::CGame() : m_gamestate(GAME_UNINITED), m_gametype(GT_PLAYER_VS_AI), m_ailevel(AIL_MEDIUM),
m_ipaddr(DEFAULT_IPADDR), m_gridsize(DEFAULT_GRIDSIZE),
m_server(NULL), m_client(NULL), m_ai(NULL) {
	for(int i = 0; i < TOTAL_SHIPTYPES; i++) {
		m_shipcount[i] = DEFAULT_SHIPCOUNT;
	}
}


CGame::~CGame() {
	delete m_server;
	delete m_client;
	delete m_ai;
}

void CGame::Init() {
	SetStatus("<< BATTLESHIP ]:==---", SPOS_TOP_RIGHT);
	m_menu.Init(this);
	m_menu.Open();
	DrawStatus();
	refresh();
}

void CGame::Start() {
	if(m_gametype == GT_PLAYER_VS_AI)
		m_ipaddr = DEFAULT_IPADDR;
	//start a server
	if(m_gametype == GT_PLAYER_VS_AI || m_gametype == GT_PVP_SERVER) {
		delete m_server;
		m_server = new CServer(this);
		try {
			m_server->Start(m_ipaddr);
		}
		catch(CServerError) {
			delete m_server;
			m_server = NULL;
			SetError("Couldn't start server, press any key to continue.");
			return;
		}
	}
	//create an ai
	if(m_gametype == GT_PLAYER_VS_AI) {
		delete m_ai;
		switch(m_ailevel) {
		case AIL_EASY:
			m_ai = new CEasyAI(this);
			break;
		case AIL_MEDIUM:
			m_ai = new CMediumAI(this);
			break;
		case AIL_HARD:
			m_ai = new CHardAI(this);
			break;
		default:
			break;
		}
		try {
			while(!m_ai->Connect(m_ipaddr)) {}
		}
		catch(CClientError) {
			delete m_ai;
			m_ai = NULL;
			SetError("AI couldn't connect, press any key to continue.");
			return;
		}
	}
	//connect player
	{
		delete m_client;
		m_client = new CClient(this);
		try {
			while(!m_client->Connect(m_ipaddr)) {}
		}
		catch(CClientError) {
			delete m_client;
			m_client = NULL;
			SetError("Couldn't connect, press any key to continue.");
			return;
		}
	}
	SetStatus("Connecting...", SPOS_TOP_LEFT);
	SetStatus("Press ESC to disconnect.", SPOS_BOTTOM);
	m_gamestate = GAME_WAITING;
}

void CGame::SetStatus(const string & status, statuspos_t pos) {
	m_status[pos] = status;
}

void CGame::SetError(const string & error) {
	erase();
	SetStatus("Error!", SPOS_TOP_LEFT);
	SetStatus(error, SPOS_BOTTOM);
	m_gamestate = GAME_INERROR;
}

void CGame::DrawStatus() const {
	attrset(COLOR_PAIR(WHITE_ON_BLUE));
	int height = getmaxy(stdscr);
	int width = getmaxx(stdscr);
	for(int y = 0; y < height; y += height - 1) {
		move(y, 0);
		for(int x = 0; x < width; x++) {
			addch(' ');
		}
		if(y == 0) {
			mvprintw(y, 1, "%s", m_status[SPOS_TOP_LEFT].c_str());
			attron(A_BOLD);
			mvprintw(y, (width - m_status[SPOS_TOP_RIGHT].length()) - 1, "%s", m_status[SPOS_TOP_RIGHT].c_str());
			attroff(A_BOLD);
		}
		else {
			mvprintw(y, (width - m_status[SPOS_BOTTOM].length()) / 2, "%s", m_status[SPOS_BOTTOM].c_str());
		}
	}
	move(0, 0);
	attrset(COLOR_PAIR(WHITE_ON_BLACK));
}

void CGame::Disconnect(bool error) {
	delete m_server;
	m_server = NULL;
	delete m_client;
	m_client = NULL;
	delete m_ai;
	m_ai = NULL;
	if(error) {
		SetError("Connection failed!");
	}
	else {
		m_menu.Open();
	}
}

void CGame::HandlePlacement(int c) {
	int sumships = 0;
	int sumrem = 0;
	for(int i = 0; i < TOTAL_SHIPTYPES; i++) {
		sumships += m_shipcount[i];
		sumrem += m_tmpplacement.m_rem[i];
	}
	if(sumrem == 0) {
		try {
			m_client->SendMessage(m_grids[GG_LEFT].Serialize());
		}
		catch(CSocketError) {
			Disconnect(true);
			return;
		}
		m_gamestate = GAME_WAITING;
		SetStatus("Awaiting placement", SPOS_TOP_LEFT);
		SetStatus("Waiting for enemy's placement. Press Esc to disconnect.", SPOS_BOTTOM);
		return;
	}
	switch(c) {
	case KEY_ESC:
		//exit
		Disconnect();
		break;
	case KEY_SPACE:
		//rotate
		m_tmpplacement.m_rot = (m_tmpplacement.m_rot + 1) % 4;
		break;
	case KEY_ENTER:
	case KEY_NLINE:
		//place
		if(m_grids[GG_LEFT].CanPlace(m_tmpplacement)) {
			m_grids[GG_LEFT].Place(m_tmpplacement);
			m_tmpplacement.m_rem[m_tmpplacement.m_ship]--;
			sumrem--;
		}
		else {
			break;
		}
		//FALL THROUGH!
	case KEY_TAB:
		//skip
		if(!sumrem)
			break;
		do {
			m_tmpplacement.m_ship = (shiptypes_t)((m_tmpplacement.m_ship + 1) % TOTAL_SHIPTYPES);
		} while(!m_tmpplacement.m_rem[m_tmpplacement.m_ship]);
		break;
	case KEY_UP:
		m_tmpplacement.m_y--;
		break;
	case KEY_DOWN:
		m_tmpplacement.m_y++;
		break;
	case KEY_LEFT:
		m_tmpplacement.m_x--;
		break;
	case KEY_RIGHT:
		m_tmpplacement.m_x++;
		break;
	default:
		break;
	}
	m_grids[GG_LEFT].FixPlacement(m_tmpplacement);
	move(2, 1);
	m_grids[GG_LEFT].Draw();
	move(2, 1);
	m_grids[GG_LEFT].DrawPlacement(m_tmpplacement);
}

bool CGame::GameLoop() {
	try {
		if(m_client) {
			m_client->ClientLoop();
		}
		if(m_server)
			m_server->ServerLoop();
		if(m_ai)
			m_ai->AILoop();
	}
	catch(CSocketError) {
		Disconnect(true);
	}

	int c = wgetch(stdscr);
	if(m_gamestate != GAME_WAITING)
		erase();

	switch(m_gamestate) {
	case GAME_INMENU:
		if(!m_menu.MenuLoop(c))
			return false;
		break;
	case GAME_WAITING:
		if(c == KEY_ESC) {
			Disconnect();
		}
		break;
	case GAME_INERROR:
		if(c != ERR)
			m_menu.Open();
		break;
	case GAME_INPLACEMENT:
		HandlePlacement(c);
		break;
	case GAME_GAMEOVER:
		if(c == KEY_ESC) {
			Disconnect();
		}
		else {
			c = ERR;
		}
		//FALL THROUGH
	case GAME_INGAME:
		HandleTurn(c);
		break;
	default:
		break;
	}
	DrawStatus();
	refresh();
	return true;
}

bool CGame::CanPlaceAll() {
	return CanPlaceAll(m_grids[GG_LEFT]);
}

bool CGame::CanPlaceAll(CGrid& grid) const {
	grid.SetSize(m_gridsize);
	TTmpPlacement tmpp;
	int remsum = 0;
	for(int i = 0; i < TOTAL_SHIPTYPES; i++) {
		tmpp.m_rem[i] = m_shipcount[i];
		remsum += tmpp.m_rem[i];
	}
	tmpp.m_ship = (shiptypes_t)0;
	bool superbreak = false;
	while(remsum) {
		while(!tmpp.m_rem[tmpp.m_ship])
			tmpp.m_ship = (shiptypes_t)(tmpp.m_ship + 1);
		for(tmpp.m_y = -SHIP_BBOX_SIZE; tmpp.m_y < m_gridsize; tmpp.m_y++) {
			for(tmpp.m_x = -SHIP_BBOX_SIZE; tmpp.m_x < m_gridsize; tmpp.m_x++) {
				if(grid.CanPlace(tmpp)) {
					grid.Place(tmpp);
					tmpp.m_rem[tmpp.m_ship]--;
					remsum--;
					superbreak = true;
					break;
				}
			}
			if(superbreak)
				break;
		}
		if(!superbreak)
			return false;
		superbreak = false;
	}
	return true;
}

void CGame::HandleTurn(int c) {
	if(m_gamestate == GAME_INGAME) {
		SetStatus("It's your turn.", SPOS_TOP_LEFT);
		SetStatus("Use arrows to move, space to shoot. Press Esc to disconnect.", SPOS_BOTTOM);
	}

	TTmpPlacement& gpos = m_tmpplacement;

	switch(c) {
	case KEY_ESC:
		//exit
		Disconnect();
		break;
	case KEY_SPACE:
		if(m_grids[GG_RIGHT].CanShoot(gpos.m_x, gpos.m_y)) {
			try {
				m_client->SendShot(gpos.m_x, gpos.m_y);
			}
			catch(CSocketError) {
				Disconnect(true);
				return;
			}
		}
		break;
	case KEY_UP:
		gpos.m_y--;
		break;
	case KEY_DOWN:
		gpos.m_y++;
		break;
	case KEY_LEFT:
		gpos.m_x--;
		break;
	case KEY_RIGHT:
		gpos.m_x++;
		break;
	default:
		break;
	}
	if(gpos.m_x < 0)
		gpos.m_x = 0;
	if(gpos.m_x >= m_gridsize)
		gpos.m_x = m_gridsize - 1;
	if(gpos.m_y < 0)
		gpos.m_y = 0;
	if(gpos.m_y >= m_gridsize)
		gpos.m_y = m_gridsize - 1;

	move(2, 1);
	m_grids[GG_LEFT].Draw();
	int orgy = 2;
	int orgx = m_gridsize + 2;
	move(orgy, orgx);
	m_grids[GG_RIGHT].Draw();
	move(orgy + gpos.m_y, orgx + gpos.m_x);
	char under = winch(stdscr);
	if(under != '?') {
		attrset(COLOR_PAIR(WHITE_ON_RED));
	}
	else {
		attrset(COLOR_PAIR(BLUE_ON_RED));
	}
	addch(under);
	attrset(COLOR_PAIR(WHITE_ON_BLACK));
}

gametypes_t CGame::GetGameType() const {
	return m_gametype;
}

void CGame::SetGameType(gametypes_t gt) {
	m_gametype = gt;
}

ailevels_t CGame::GetAILevel() const {
	return m_ailevel;
}

void CGame::SetAILevel(ailevels_t ail) {
	m_ailevel = ail;
}

string CGame::GetIPAddr() const {
	return m_ipaddr;
}

void CGame::SetIPAddr(string ip) {
	m_ipaddr = ip;
}

int CGame::GetGridSize() const {
	return m_gridsize;
}

void CGame::SetGridSize(int gs) {
	m_gridsize = gs;
	m_grids[GG_LEFT].SetSize(gs);
	m_grids[GG_RIGHT].SetSize(gs);
	m_grids[GG_RIGHT].SetUnknown();
}

int CGame::GetShipCount(shiptypes_t type) const {
	return m_shipcount[type];
}

void CGame::SetShipCount(shiptypes_t type, int count) {
	m_shipcount[type] = count;
}

gamestates_t CGame::GetGameState() const {
	return m_gamestate;
}

void CGame::SetGameState(gamestates_t state) {
	m_gamestate = state;
}

void CGame::SetTmpPlcmnt(TTmpPlacement p) {
	m_tmpplacement = p;
}

CGrid & CGame::GetGrid(gamegrids_t c) {
	return m_grids[c];
}

const CGrid & CGame::GetGrid(gamegrids_t c) const {
	return m_grids[c];
}
