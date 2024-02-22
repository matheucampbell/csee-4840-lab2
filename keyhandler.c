#include "keyhandler.h"

const int SCREEN_ROWS = 24;
const int SCREEN_COLS = 64;
const int TYPE_ROW_MIN = 18;

void update_position(int keycode, int mods, int* x, int* y){
	if (keycode == RIGHT_ARROW && *x < SCREEN_COLS-1)
		(*x)++;
	else if (keycode == LEFT_ARROW && *x > 0)
		(*x)--;
	else if (keycode == UP_ARROW && *y > TYPE_ROW_MIN)
		(*y)--;
	else if (keycode == DOWN_ARROW && *y < SCREEN_ROWS-1)
		(*y)++;
}
