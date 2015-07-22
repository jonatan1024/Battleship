#include "Field.h"
#include "colors.h"
#include <curses.h>

CField::CField() : m_content(FIELD_EMPTY) {
}


CField::~CField() {
}

void CField::Draw() const {

	switch(m_content) {
	case FIELD_EMPTY:
		attrset(COLOR_PAIR(BLACK_ON_BLUE));
		addch('~');
		break;
	case FIELD_EMPTY_HIT:
		attrset(COLOR_PAIR(RED_ON_BLUE));
		addch('~');
		break;
	case FIELD_SHIP:
		attrset(COLOR_PAIR(WHITE_ON_BLUE) | A_BOLD);
		addch('#');
		break;
	case FIELD_SHIP_HIT:
		attrset(COLOR_PAIR(RED_ON_BLUE) | A_BOLD);
		addch('@');
		break;
	case FIELD_SHIP_SUNK:
		attrset(COLOR_PAIR(RED_ON_BLUE));
		addch('#');
		break;
	case FIELD_UNKNOWN:
		attrset(COLOR_PAIR(BLACK_ON_BLUE));
		addch('?');
		break;
	default:
		break;
	}
	attrset(COLOR_PAIR(WHITE_ON_BLACK));
}

fieldcontents_t CField::GetContent() const {
	return m_content;
}
void CField::SetContent(fieldcontents_t c) {
	m_content = c;
}
