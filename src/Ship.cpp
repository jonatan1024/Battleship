#include "Ship.h"

CShip::~CShip() {

}

class CShipSupercarrier : public CShip {
	virtual const bool * GetShipShape() const {
		const static bool shape[] = {
			0, 0, 0, 0,
			0, 1, 1, 0,
			1, 1, 1, 1,
			0, 0, 0, 0,
		};
		return shape;
	}

	virtual const string & GetShipName() const {
		const static string name("Supercarrier");
		return name;
	}
};

class CShipBattleship : public CShip {
	virtual const bool * GetShipShape() const {
		const static bool shape[] = {
			0, 0, 0, 0,
			0, 0, 1, 0,
			0, 1, 1, 1,
			0, 0, 0, 0,
		};
		return shape;
	}

	virtual const string & GetShipName() const {
		const static string name("Battleship");
		return name;
	}
};

class CShipDestroyer : public CShip {
	virtual const bool * GetShipShape() const {
		const static bool shape[] = {
			0, 0, 0, 0,
			0, 0, 0, 0,
			1, 1, 1, 1,
			0, 0, 0, 0,
		};
		return shape;
	}

	virtual const string & GetShipName() const {
		const static string name("Destroyer");
		return name;
	}
};

class CShipCorvette : public CShip {
	virtual const bool * GetShipShape() const {
		const static bool shape[] = {
			0, 0, 0, 0,
			0, 0, 0, 0,
			0, 1, 1, 0,
			0, 0, 0, 0,
		};
		return shape;
	}

	virtual const string & GetShipName() const {
		const static string name("Corvette");
		return name;
	}
};

CShip * CShip::BuildShip(shiptypes_t type) {
	switch(type) {
	case SHIP_SUPERCARRIER:
		return new CShipSupercarrier();
		break;
	case SHIP_BATTLESHIP:
		return new CShipBattleship();
		break;
	case SHIP_DESTROYER:
		return new CShipDestroyer();
		break;
	case SHIP_CORVETTE:
		return new CShipCorvette();
		break;
	default:
		break;
	}
	return NULL;
}

const bool * CShip::GetRotatedShape(int rot) const {
	static bool shape[SHIP_BBOX_SIZE*SHIP_BBOX_SIZE];
	for(int i = 0; i < SHIP_BBOX_SIZE*SHIP_BBOX_SIZE; i++) {
		shape[i] = GetShipShape()[i];
	}
	for(int i = 0; i < rot; i++) {
		RotateShape(shape);
	}
	return shape;
}

void CShip::RotateShape(bool * shape) const {
	bool newshape[SHIP_BBOX_SIZE*SHIP_BBOX_SIZE];
	for(int y = 0; y < SHIP_BBOX_SIZE; y++) {
		for(int x = 0; x < SHIP_BBOX_SIZE; x++) {
			int newy = SHIP_BBOX_SIZE - x - 1;
			int newx = y;
			newshape[newy * SHIP_BBOX_SIZE + newx] = shape[y*SHIP_BBOX_SIZE + x];
		}
	}
	for(int i = 0; i < SHIP_BBOX_SIZE*SHIP_BBOX_SIZE; i++) {
		shape[i] = newshape[i];
	}
}
