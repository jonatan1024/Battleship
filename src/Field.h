#pragma once

/// All possible contents of the field.
enum fieldcontents_t {
	FIELD_EMPTY,
	FIELD_EMPTY_HIT,
	FIELD_SHIP,
	FIELD_SHIP_HIT,
	FIELD_SHIP_SUNK,
	FIELD_UNKNOWN,
	TOTAL_FIELDCONTENTS
};

/// Fields, simple elements of a grid. You can Get/Set their content or Draw them on screen.
class CField {
public:
	CField();
	~CField();
	/// Draws this field on current location on screen
	void Draw() const;
	fieldcontents_t GetContent() const;
	void SetContent(fieldcontents_t c);
private:
	/// Contents of the field.
	fieldcontents_t m_content;
};

