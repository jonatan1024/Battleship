#pragma once
#include <curses.h>

/// Color pairs used by this program.
enum colorpairs_t{
	WHITE_ON_BLACK = 1,
	BLACK_ON_WHITE,
	WHITE_ON_BLUE,
	BLACK_ON_BLUE,
	RED_ON_BLUE,
	GREEN_ON_BLUE,
	BLUE_ON_RED,
	WHITE_ON_RED,
	TOTAL_COLORS,
};

/// Macro to initialize a color pair. If you want to add more pairs, insert them into enum above.
#define INIT_PAIR(foreground, background) init_pair(foreground ## _ON_ ## background, COLOR_ ## foreground, COLOR_ ## background)
