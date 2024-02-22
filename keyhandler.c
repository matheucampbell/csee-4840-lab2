#include "keyhandler.h"

// Updates cursor upon receiving arrow key input
void update_position(int keycode, int mods, int* x, int* y){
	if (keycode == RIGHT_ARROW && *x < SCREEN_COLS-1)
		(*x)++;
	else if (keycode == LEFT_ARROW && *x > 0)
		(*x)--;
	else if (keycode == UP_ARROW && *y > TYPE_ROW_MIN)
		(*y)--;
	else if (keycode == DOWN_ARROW && *y < SCREEN_ROWS-2)
		(*y)++;
}

// Updates text buffer upon receiving text, then moves cursor
void parse_letters(int keycode, int mods, char** buf, int* x, int* y){
	int bufpos = *x;
	char c;
	if (0x00 <= keycode  && keycode <= 0x1c){
		c = keycode + 94;
		*buf[bufpos] = c;
	}
	
	update_position(RIGHT_ARROW, mods, x, y);
}
