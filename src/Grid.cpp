#include "Grid.h"
#include "colors.h"
#include <curses.h>
#include "Game.h"
#include "Ship.h"
#include <sstream>
#include <queue>

TTmpPlacement::TTmpPlacement() : m_ship(TOTAL_SHIPTYPES), m_x(0), m_y(0), m_rot(0) {
	for(int i = 0; i < TOTAL_SHIPTYPES; i++) {
		m_rem[i] = 0;
	}
}

CGrid::CGrid() : m_size(0) {
	for(int i = 0; i < TOTAL_SHIPTYPES; i++) {
		m_ships[i] = CShip::BuildShip((shiptypes_t)i);
	}
}

CGrid::CGrid(int size) : CGrid() {
	SetSize(size);
}

CGrid::~CGrid() {
	for(int i = 0; i < TOTAL_SHIPTYPES; i++)
		delete m_ships[i];
}

CGrid::CGrid(const CGrid& g) {
	m_size = g.m_size;
	m_fields = g.m_fields;
	for(int i = 0; i < TOTAL_SHIPTYPES; i++)
		m_ships[i] = CShip::BuildShip((shiptypes_t)i);
}

void CGrid::Clear() {
	m_fields.clear();
	m_fields.resize(m_size*m_size);
}

void CGrid::SetSize(int size) {
	m_size = size;
	m_fields.clear();
	m_fields.resize(size*size);
}

int CGrid::GetSize() const {
	return m_size;
}

void CGrid::SetUnknown() {
	for(CField & field : m_fields) {
		field.SetContent(FIELD_UNKNOWN);
	}
}

void CGrid::SetFieldContent(int x, int y, fieldcontents_t content) {
	m_fields[y*m_size + x].SetContent(content);
}

fieldcontents_t CGrid::GetFieldContent(int x, int y) const {
	return m_fields[y*m_size + x].GetContent();
}

bool CGrid::IsGridDefeated() const {
	for(const CField & field : m_fields) {
		if(field.GetContent() == FIELD_SHIP)
			return false;
	}
	return true;
}


void CGrid::Draw() const {
	int x = getcurx(stdscr);
	int y = getcury(stdscr);
	for(int row = 0; row < m_size; row++) {
		move(y + row, x);
		for(int col = 0; col < m_size; col++) {
			m_fields[row*m_size + col].Draw();
		}
	}
}

int CGrid::ShapeInGrid(const bool * shape, int x, int y) const {
	for(int i = 0; i < SHIP_BBOX_SIZE; i++) {
		for(int j = 0; j < SHIP_BBOX_SIZE; j++) {
			if(shape[i*SHIP_BBOX_SIZE + j]) {
				if(j + x < 0)
					return 1;
				if(j + x >= m_size)
					return -1;
				if(i + y < 0)
					return 2;
				if(i + y >= m_size)
					return -2;
			}
		}
	}
	return 0;
}

void CGrid::FixPlacement(TTmpPlacement& tmpp) const {
	const bool * shape = m_ships[tmpp.m_ship]->GetRotatedShape(tmpp.m_rot);
	while(int fix = ShapeInGrid(shape, tmpp.m_x, tmpp.m_y)) {
		if(fix == 1 || fix == -1) {
			tmpp.m_x += fix;
		}
		if(fix == 2 || fix == -2) {
			tmpp.m_y += fix / 2;
		}
	}
}

void CGrid::DrawPlacement(const TTmpPlacement& tmpp) const {
	const bool * shape = m_ships[tmpp.m_ship]->GetRotatedShape(tmpp.m_rot);
	int orgx = getcurx(stdscr);
	int orgy = getcury(stdscr);
	if(CanPlace(tmpp)) {
		attrset(COLOR_PAIR(GREEN_ON_BLUE) | A_BOLD);
	}
	else {
		attrset(COLOR_PAIR(RED_ON_BLUE));
	}
	for(int row = 0; row < SHIP_BBOX_SIZE; row++) {
		for(int col = 0; col < SHIP_BBOX_SIZE; col++) {
			if(shape[row*SHIP_BBOX_SIZE + col]) {
				move(orgy + tmpp.m_y + row, orgx + tmpp.m_x + col);
				addch('#');
			}
		}
	}
	attrset(COLOR_PAIR(WHITE_ON_BLACK));
}

bool CGrid::CanPlace(const TTmpPlacement& tmpp) const {
	TTmpPlacement fixed = tmpp;
	FixPlacement(fixed);
	if(fixed.m_x != tmpp.m_x || fixed.m_y != tmpp.m_y)
		return false;
	const bool * shape = m_ships[tmpp.m_ship]->GetRotatedShape(tmpp.m_rot);
	for(int i = 0; i < SHIP_BBOX_SIZE; i++) {
		for(int j = 0; j < SHIP_BBOX_SIZE; j++) {
			if(shape[i*SHIP_BBOX_SIZE + j]) {
				int x = tmpp.m_x + j;
				int y = tmpp.m_y + i;
				if(m_fields[y * m_size + x].GetContent() != FIELD_EMPTY) {
					return false;
				}
				if((x - 1 >= 0) && m_fields[y * m_size + x - 1].GetContent() > FIELD_EMPTY_HIT)
					return false;
				if((x + 1 < m_size) && m_fields[y * m_size + x + 1].GetContent() > FIELD_EMPTY_HIT)
					return false;
				if((y - 1 >= 0) && m_fields[(y - 1) * m_size + x].GetContent() > FIELD_EMPTY_HIT)
					return false;
				if((y + 1 < m_size) && m_fields[(y + 1) * m_size + x].GetContent() > FIELD_EMPTY_HIT)
					return false;
			}
		}
	}
	return true;
}

void CGrid::Place(const TTmpPlacement& tmpp) {
	const bool * shape = m_ships[tmpp.m_ship]->GetRotatedShape(tmpp.m_rot);
	for(int i = 0; i < SHIP_BBOX_SIZE; i++) {
		for(int j = 0; j < SHIP_BBOX_SIZE; j++) {
			if(shape[i*SHIP_BBOX_SIZE + j]) {
				int x = tmpp.m_x + j;
				int y = tmpp.m_y + i;
				m_fields[y * m_size + x].SetContent(FIELD_SHIP);
			}
		}
	}
}

string CGrid::Serialize() const {
	stringstream ss;
	ss << m_size << endl;
	for(int i = 0; i < m_size*m_size; i++) {
		ss << m_fields[i].GetContent() << " ";
	}
	return ss.str();
}

void CGrid::Deserialize(const string & grid) {
	stringstream ss(grid);
	int size;
	ss >> size;
	SetSize(size);
	for(int i = 0; i < m_size*m_size; i++) {
		int content;
		ss >> content;
		m_fields[i].SetContent((fieldcontents_t)content);
	}
}


bool CGrid::CanShoot(int x, int y) const {
	return m_fields[m_size*y + x].GetContent() == FIELD_UNKNOWN;
}

bool CGrid::GetSunk(int x, int y, set<int> & sunk) const {
	//DFS
	queue<int> q;
	q.push(y*m_size + x);
	while(!q.empty()) {
		int pos = q.front();
		q.pop();
		fieldcontents_t content = m_fields[pos].GetContent();
		switch(content) {
		case FIELD_SHIP:
		{
			sunk.clear();
			sunk.insert(y*m_size + x);
			return false;
			break;
		}
		case FIELD_SHIP_HIT:
		{
			if(sunk.find(pos) != sunk.end())
				break;
			sunk.insert(pos);
			int newx = pos % m_size;
			int newy = pos / m_size;
			if(newx - 1 >= 0)
				q.push(newy*m_size + newx - 1);
			if(newx + 1 < m_size)
				q.push(newy*m_size + newx + 1);
			if(newy - 1 >= 0)
				q.push((newy - 1)*m_size + newx);
			if(newy + 1 < m_size)
				q.push((newy + 1)*m_size + newx);
			break;
		}
		default:
			break;
		}
	}
	return true;
}

void CGrid::Shoot(TShotInfo& info) {
	CField & targetfield = m_fields[info.m_pos];
	fieldcontents_t content = targetfield.GetContent();
	info.m_content = FIELD_UNKNOWN;
	info.m_defeat = false;
	info.m_sunk.clear();
	if(content == FIELD_SHIP) {
		targetfield.SetContent(FIELD_SHIP_HIT);
		int y = info.m_pos / m_size;
		int x = info.m_pos % m_size;
		if(GetSunk(x, y, info.m_sunk)) {
			for(int pos : info.m_sunk) {
				m_fields[pos].SetContent(FIELD_SHIP_SUNK);
			}
		}
		info.m_content = targetfield.GetContent();
		info.m_defeat = IsGridDefeated();
	}
	else {
		targetfield.SetContent(FIELD_EMPTY_HIT);
		info.m_content = FIELD_EMPTY_HIT;
		info.m_sunk.insert(info.m_pos);
	}
}
