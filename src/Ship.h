#pragma once
#include <string>

using namespace std;

/// Size of a grid of a ship's shape.
#define SHIP_BBOX_SIZE 4

/// Ship types, there is a class for each one of them.
enum shiptypes_t {
	/// CShipSupercarrier
	SHIP_SUPERCARRIER,
	/// CShipBattleship
	SHIP_BATTLESHIP,
	/// CShipDestroyer
	SHIP_DESTROYER,
	/// CShipCorvette
	SHIP_CORVETTE,

	TOTAL_SHIPTYPES
};

/// Abstract Ship class and Ship factory.
class CShip {
public:
	virtual ~CShip();
	/// Ship factory. ( shipyard :) )
	/// @param type Type of the ship we want to build.
	/// @return Ship of specified type.
	static CShip * BuildShip(shiptypes_t type);
	/// @return Ship's shape in an 1D array of booleans. It should be interpreted as a 2D array with side of SHIP_BBOX_SIZE.
	virtual const bool * GetShipShape() const = 0;
	/// See GetShipShape() for more info.
	/// @param rot Rotation.
	const bool * GetRotatedShape(int rot) const;
	/// @return Ship's name.
	virtual const string & GetShipName() const = 0;
private:
	/// Rotates a shape 90 degrees in counterclockwise direction.
	void RotateShape(bool * shape) const;
};

