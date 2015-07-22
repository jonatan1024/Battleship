#pragma once
#include "Client.h"
#include "Grid.h"

/// Maximum amount of tries for random fleet placement.
#define MAX_AI_PLACEMENT_TRIES 64

/// Abstract AI class, parent for all AI implementations.
class CAI : public CClient {
public:
	/// @param game Game to connect to.
	CAI(CGame * game);
	virtual ~CAI();
	/// Main AI loop, delegates current job to proper functions.
	void AILoop();
protected:
	/// Enemy's grid.
	CGrid m_grid;
	/// Read hit data and store thim into grid.
	/// @param msg Network message.
	virtual void ReadShotInfo(const string& msg);
	/// Deciede how to arrange the fleet.
	virtual void DecidePlacement() = 0;
	/// Decide where to shoot.
	/// @param x Output x coordinate.
	/// @param y Output y coordinate.
	virtual void DecideShot(int & x, int & y) const = 0;
};

/// Easy AI, shoots completely random.
class CEasyAI : public CAI {
public:
	CEasyAI(CGame * game);
private:
	/// Arrange ships randomly.
	/// @param horizontal True if we want to restrict ships' rotation to horizontal.
	/// @return true if the placement fits into the grid.
	bool TryRandomPlacement(bool horizontal = false);
protected:
	/// Find first placement that fits into the grid.
	/// Tries MAX_AI_PLACEMENT_TRIES times. If it fails, it uses Game's CanPlaceAll function to place the ships.
	void DecidePlacement();
	/// Randomly selects a field to shoot to.
	/// @param x Output x coordinate.
	/// @param y Output y coordinate.
	void DecideShot(int & x, int & y) const;
};

/// Medium AI, shoots random and tries to sink ships that was hit but not sunk yet.
class CMediumAI : public CEasyAI {
public:
	CMediumAI(CGame * game);
protected:
	/// If there is a ship that has been hit but not yet sunk, use NeighbourShot to sink it.
	/// Otherwise use Easy's DecideShot to find a target.
	/// @param x Output x coordinate.
	/// @param y Output y coordinate.
	void DecideShot(int & x, int & y) const;
private:
	/// Finds a target near specified coordinates.
	/// @param x I/O x coordinate.
	/// @param y I/O y coordinate.
	/// @return true if there is a suitable target around x and y.
	bool NeighbourShot(int & x, int & y) const;
};

/// Hard AI, uses more clever techniques for selecting a target.
class CHardAI : public CMediumAI {
public:
	CHardAI(CGame * game);
	~CHardAI();
protected:
	/// Reads additional info.
	/// Guesses what kind of ship has been sunk.
	void ReadShotInfo(const string& msg);
	/// Finds a field that has the biggest probability of hiding an enemy's ship.
	/// @param x Output x coordinate.
	/// @param y Output y coordinate.
	void DecideShot(int & x, int & y) const;
private:
	/// Ships, shapes
	CShip * m_ships[TOTAL_SHIPTYPES];
	/// Remaining enemy's ships.
	int m_remaining[TOTAL_SHIPTYPES];
	/// Finds a list (vector) of fields that has the biggest probability of hiding an enemy's ship.
	/// @param sinkfirst If there are hit (but not sunk) ships, target them first.
	/// @return Vector of positions.
	vector<int> GetBestTargets(bool sinkfirst) const;
	void AddPlaceProb(const TTmpPlacement & tmpp, int * probs, bool sinkfirst) const;
};
