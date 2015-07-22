#pragma once
#include <vector>
#include "Field.h"
#include "Ship.h"
#include <set>

using namespace std;

/// I/O structure for storing hit info.
struct TShotInfo {
	/// (in) Position to shoot at.
	int m_pos;
	/// (out) What content we hit?
	fieldcontents_t m_content;
	/// (out) What fields we hit?
	set<int> m_sunk;
	/// (out) Have we defeated this grid?
	bool m_defeat;
};

/// Structure for storing teporary placement of a ship.
struct TTmpPlacement {
	TTmpPlacement();
	/// ship type
	shiptypes_t m_ship;
	/// position
	int m_x;
	/// position
	int m_y;
	/// How much of each ship type is left to place.
	int m_rem[TOTAL_SHIPTYPES];
	/// rotation
	int m_rot;
};

/// Grid, 2D array of Fields.
class CGrid {
public:
	CGrid();
	/// Create a square grid.
	/// @param size Length of an edge.
	CGrid(int size);
	~CGrid();
	/// Grid copy.
	CGrid(const CGrid&);
	CGrid& operator=(const CGrid&) = delete;

	/// Clears the grid.
	/// Content of all the fields is set to EMPTY.
	void Clear();
	/// Clears the grid and set a new size.
	void SetSize(int size);
	int GetSize() const;
	/// Mark all fields' content as UNKNOWN.
	void SetUnknown();
	/// Draw the grid at current position.
	void Draw() const;
	/// If the tmpp is off the grid, put it back in.
	/// @param tmpp Temporary placement to fix.
	void FixPlacement(TTmpPlacement& tmpp) const;
	/// Draw the temp placement as it was already placed into the grid.
	/// If the placement is ok, it's in green, otherwise it's in red.
	/// @param tmpp Temporary placement to draw.
	void DrawPlacement(const TTmpPlacement& tmpp) const;
	/// @param tmpp Temporary placement to try.
	/// @return True if the placement is ok to place.
	bool CanPlace(const TTmpPlacement& tmpp) const;
	/// Apply the placement to the grid.
	/// @param tmpp Temporary placement to apply.
	void Place(const TTmpPlacement& tmpp);
	/// Serialize the grid into a string.
	/// @return Grid as a string.
	string Serialize() const;
	/// Deserialize the string into a grid and apply it.
	/// @param grid Grid as a string.
	void Deserialize(const string & grid);
	/// @return True if the target field's content is UNKNOWN.
	bool CanShoot(int x, int y) const;
	/// See TShotInfo for more info.
	/// @paramt info I/O structure.
	void Shoot(TShotInfo& info);
	void SetFieldContent(int x, int y, fieldcontents_t content);
	fieldcontents_t GetFieldContent(int x, int y) const;
private:
	bool GetSunk(int x, int y, set<int> & sunk) const;
	int ShapeInGrid(const bool * shape, int x, int y) const;
	bool IsGridDefeated() const;
	/// Length of the grid's edge
	int m_size;
	/// Fields stored in 1D "vector"
	vector<CField> m_fields;
	/// Ships, shapes.
	CShip * m_ships[TOTAL_SHIPTYPES];
};

