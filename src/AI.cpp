#include "AI.h"
#include <stdlib.h>
#include <time.h>
#include <sstream>
#include <string.h>

inline float RandomFloat() {
	return (float)rand() / RAND_MAX;
}

inline int RandomRange(int min, int max) {
	return min + ((rand()) % (max - min));
}

inline bool RandomBool() {
	return (rand() % 2 == 0);
}

CAI::CAI(CGame * game) : CClient(game) {
	//we can rely on values from game
}


CAI::~CAI() {

}

void CAI::AILoop() {
	string msg;
	if(ReadMessage(msg)) {
		switch(m_state) {
		case CLIENT_WFOR_CONFIG:
			//we don't care about the config
			//we can rely on values from game
			m_grid.SetSize(m_game->GetGridSize());
			//we're here only for the placement
			DecidePlacement();
			SendMessage(m_grid.Serialize());
			m_grid.Clear();
			m_grid.SetUnknown();
			m_state = CLIENT_WFOR_ROUND;
			break;
		case CLIENT_WFOR_ROUND:
			if(msg != "ok") {
				//excluding first round
				ReadShotInfo(msg);
			}
			//decide shot
			int x, y;
			DecideShot(x, y);
			SendShot(x, y);
			break;
		default:
			break;
		}
	}
}

void CAI::ReadShotInfo(const string & msg) {
	stringstream ss(msg);
	//we don't care about the game result
	string gameresult;
	ss >> gameresult;
	int gs = m_game->GetGridSize();
	int numsunk;
	ss >> numsunk;
	for(int j = 0; j < numsunk; j++) {
		int pos;
		int content;
		ss >> pos;
		ss >> content;
		m_grid.SetFieldContent(pos % gs, pos / gs, (fieldcontents_t)content);
	}
	//we don't care about his hits either
}

CEasyAI::CEasyAI(CGame * game) : CAI(game) {

}

bool CEasyAI::TryRandomPlacement(bool horizontal) {
	m_grid.Clear();
	vector<shiptypes_t> ships;
	for(int i = 0; i < TOTAL_SHIPTYPES; i++) {
		int count = m_game->GetShipCount((shiptypes_t)i);
		for(int j = 0; j < count; j++) {
			ships.push_back((shiptypes_t)i);
		}
	}
	while(!ships.empty()) {
		int pick = RandomRange(0, ships.size());
		shiptypes_t st = ships[pick];
		ships.erase(ships.begin() + pick);

		TTmpPlacement tmpp;
		tmpp.m_ship = st;
		vector<TTmpPlacement> placements;
		int gs = m_game->GetGridSize();
		for(tmpp.m_y = -SHIP_BBOX_SIZE; tmpp.m_y < gs; tmpp.m_y++)
			for(tmpp.m_x = -SHIP_BBOX_SIZE; tmpp.m_x < gs; tmpp.m_x++)
				for(tmpp.m_rot = 0; tmpp.m_rot < 4; tmpp.m_rot += (horizontal ? 2 : 1))
					if(m_grid.CanPlace(tmpp))
						placements.push_back(tmpp);
		if(placements.empty())
			return false;
		m_grid.Place(placements[RandomRange(0, placements.size())]);
	}
	return true;
}

void CEasyAI::DecidePlacement() {
	for(int i = 0; i < MAX_AI_PLACEMENT_TRIES / 2; i++)
		if(TryRandomPlacement())
			return;
	for(int i = 0; i < MAX_AI_PLACEMENT_TRIES / 2; i++)
		if(TryRandomPlacement(true))
			return;
	m_game->CanPlaceAll(m_grid);
}

void CEasyAI::DecideShot(int & io_x, int & io_y) const {
	int gs = m_grid.GetSize();
	vector<int> possibs;
	for(int y = 0; y < gs; y++)
		for(int x = 0; x < gs; x++)
			if(m_grid.CanShoot(x, y))
				possibs.push_back(y*gs + x);
	if(possibs.empty()) {
		io_x = 0;
		io_y = 0;
		return;
	}
	int pos = possibs[RandomRange(0, possibs.size())];
	io_x = pos % gs;
	io_y = pos / gs;
}

CMediumAI::CMediumAI(CGame * game) : CEasyAI(game) {

}

void CMediumAI::DecideShot(int & io_x, int & io_y) const {
	int gs = m_grid.GetSize();
	for(int y = 0; y < gs; y++)
		for(int x = 0; x < gs; x++)
			if(m_grid.GetFieldContent(x, y) == FIELD_SHIP_HIT) {
				io_x = x;
				io_y = y;
				if(NeighbourShot(io_x, io_y))
					return;
			}

	CEasyAI::DecideShot(io_x, io_y);
}

bool CMediumAI::NeighbourShot(int & x, int & y) const {
	int gs = m_grid.GetSize();
	vector<int> possibs;
	if(x - 1 >= 0 && m_grid.CanShoot(x - 1, y))
		possibs.push_back(y*gs + x - 1);
	if(x + 1 < gs && m_grid.CanShoot(x + 1, y))
		possibs.push_back(y*gs + x + 1);
	if(y - 1 >= 0 && m_grid.CanShoot(x, y - 1))
		possibs.push_back((y - 1)*gs + x);
	if(y + 1 < gs && m_grid.CanShoot(x, y + 1))
		possibs.push_back((y + 1)*gs + x);

	if(possibs.empty()) {
		return false;
	}

	int pos = possibs[RandomRange(0, possibs.size())];
	x = pos % gs;
	y = pos / gs;
	return true;
}

void CHardAI::ReadShotInfo(const string & msg) {
	CAI::ReadShotInfo(msg);
	//we're here to dig some extra info
	stringstream ss(msg);
	string gr;
	ss >> gr;
	int gs = m_game->GetGridSize();
	int numsunk;
	ss >> numsunk;
	//we're here for sunk ships
	if(numsunk <= 1)
		return;
	shiptypes_t sunktype = TOTAL_SHIPTYPES;
	switch(numsunk) {
	case 2:
		sunktype = SHIP_CORVETTE;
		break;
	case 6:
		sunktype = SHIP_SUPERCARRIER;
		break;
	case 4:
	{
		int x, y;
		for(int i = 0; i < numsunk; i++) {
			int pos, content;
			ss >> pos;
			ss >> content;
			int nx = pos % gs;
			int ny = pos / gs;
			if(i == 0) {
				x = nx;
				y = ny;
			}
			if(x != nx) {
				x = -1;
			}
			if(y != ny) {
				y = -1;
			}
		}
		if(x == -1 && y == -1)
			sunktype = SHIP_BATTLESHIP;
		else
			sunktype = SHIP_DESTROYER;
		break;
	}
	default:
		break;
	}
	if(sunktype < TOTAL_SHIPTYPES) {
		m_remaining[sunktype]--;
	}
}

CHardAI::CHardAI(CGame * game) : CMediumAI(game) {
	for(int i = 0; i < TOTAL_SHIPTYPES; i++) {
		m_ships[i] = CShip::BuildShip((shiptypes_t)i);
		m_remaining[i] = game->GetShipCount((shiptypes_t)i);
	}
}

CHardAI::~CHardAI() {
	for(int i = 0; i < TOTAL_SHIPTYPES; i++)
		delete m_ships[i];
}

void CHardAI::DecideShot(int & io_x, int & io_y) const {
	int gs = m_grid.GetSize();
	vector<int> besttargets = GetBestTargets(true);
	if(besttargets.empty()) {
		CMediumAI::DecideShot(io_x, io_y);
		if(!m_grid.CanShoot(io_x, io_y))
			gs = 0;
		return;
	}
	int pick = RandomRange(0, besttargets.size());
	io_x = besttargets[pick] % gs;
	io_y = besttargets[pick] / gs;
	if(!m_grid.CanShoot(io_x, io_y))
		gs = 0;
}

vector<int> CHardAI::GetBestTargets(bool sinkfirst) const {
	int gs = m_grid.GetSize();
	CGrid tmpgrid = m_grid;
	bool sinkready = false;
	for(int y = 0; y < gs; y++)
		for(int x = 0; x < gs; x++) {
			fieldcontents_t content = tmpgrid.GetFieldContent(x, y);
			if(content == FIELD_UNKNOWN)
				tmpgrid.SetFieldContent(x, y, FIELD_EMPTY);
			else if(sinkfirst && content == FIELD_SHIP_HIT) {
				tmpgrid.SetFieldContent(x, y, FIELD_EMPTY);
				sinkready = true;
			}
		}
	if(sinkfirst && !sinkready) {
		return GetBestTargets(false);
	}

	int * probs = new int[gs*gs];
	memset(probs, 0, sizeof(*probs)*gs*gs);
	TTmpPlacement tmpp;
	for(int st = 0; st < TOTAL_SHIPTYPES; st++) {
		tmpp.m_ship = (shiptypes_t)st;
		if(!m_remaining[tmpp.m_ship])
			continue;
		for(tmpp.m_y = -SHIP_BBOX_SIZE; tmpp.m_y < gs; tmpp.m_y++)
			for(tmpp.m_x = -SHIP_BBOX_SIZE; tmpp.m_x < gs; tmpp.m_x++)
				for(tmpp.m_rot = 0; tmpp.m_rot < 4; tmpp.m_rot++) {
					if(!tmpgrid.CanPlace(tmpp))
						continue;
					AddPlaceProb(tmpp, probs, sinkfirst);
				}
	}
	vector<int> bestprobs;
	int maxprob = 1;
	for(int i = 0; i < gs*gs; i++) {
		if(probs[i] > maxprob) {
			maxprob = probs[i];
			bestprobs.clear();
		}
		if(probs[i] == maxprob) {
			bestprobs.push_back(i);
		}
	}
	delete[] probs;
	if(sinkfirst && bestprobs.empty())
		return GetBestTargets(false);
	return bestprobs;
}

void CHardAI::AddPlaceProb(const TTmpPlacement & tmpp, int * probs, bool sinkfirst) const {
	int gs = m_grid.GetSize();
	const bool * shape = m_ships[tmpp.m_ship]->GetRotatedShape(tmpp.m_rot);
	int weight = m_remaining[tmpp.m_ship];
	int multiplier = 1;
	if(sinkfirst) {
		multiplier = 0;
		for(int row = 0; row < SHIP_BBOX_SIZE; row++) {
			for(int col = 0; col < SHIP_BBOX_SIZE; col++) {
				int y = row + tmpp.m_y;
				int x = col + tmpp.m_x;
				if(shape[row*SHIP_BBOX_SIZE + col] && m_grid.GetFieldContent(x, y) == FIELD_SHIP_HIT) {
					multiplier = multiplier * 2 + 1;
				}
			}
		}
	}
	for(int row = 0; row < SHIP_BBOX_SIZE; row++) {
		for(int col = 0; col < SHIP_BBOX_SIZE; col++) {
			if(shape[row*SHIP_BBOX_SIZE + col]) {
				int y = row + tmpp.m_y;
				int x = col + tmpp.m_x;
				if(m_grid.CanShoot(x, y))
					probs[y*gs + x] += weight*multiplier;
			}
		}
	}
}
