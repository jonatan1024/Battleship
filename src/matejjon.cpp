#include <curses.h>
#include "Game.h"
#include "colors.h"
#include "Server.h"
#include <stdlib.h>
#include <time.h>

void initCurses() {
	initscr();
	clear();
	noecho();
	set_escdelay(0);
	cbreak();
	keypad(stdscr, true);
	curs_set(0);
	timeout(100);
	start_color();
	INIT_PAIR(WHITE, BLACK);
	INIT_PAIR(BLACK, WHITE);
	INIT_PAIR(WHITE, BLUE);
	INIT_PAIR(BLACK, BLUE);
	INIT_PAIR(RED, BLUE);
	INIT_PAIR(GREEN, BLUE);
	INIT_PAIR(BLUE, RED);
	INIT_PAIR(WHITE, RED);
	srand(time(0));
}

void stopCurses() {
	endwin();
}

int main() {
	initCurses();
	CGame game;
	game.Init();
	while(game.GameLoop()) {}
	stopCurses();
	return 0;
}
