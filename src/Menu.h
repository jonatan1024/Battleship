#pragma once
#include <vector>
#include "Ship.h"
#include "Grid.h"

#define KEY_ESC 0x1B
#define KEY_BSPACE 0x7F
#define KEY_NLINE 0x0A
#define KEY_TAB 0x09
#define KEY_SPACE 0x20

class CGame;
/// Menu, UI for Game configuration.
class CMenu {
public:
	CMenu();
	~CMenu();
	/// Main Menu loop, handles movement through the menu and other stuff.
	/// @param c Keycode.
	/// @return False on exitting.
	bool MenuLoop(int c);
	/// Initialize and link to the Game
	void Init(CGame * game);
	void Open();
private:
	void Draw() const;
	/// Modifies selected property by diff
	/// @param diff Value to add to the property.
	void ModifyProperty(int diff);
	/// Inserts a char into IP address field.
	void InsertIP(char c);
	/// Removes a char from IP address field.
	/// @param backspace Remove the char on the left of the cursor.
	void RemoveIP(bool backspace);
	CGame * m_game;
	/// Position, selected property.
	int m_pos;
	/// Cursor in IP address field.
	int m_hpos;
	/// Ships, shapes, names
	CShip * m_ships[TOTAL_SHIPTYPES];
};

