#include "keyhandler.h"

int SCREEN_ROWS = 24;
int SCREEN_COLS = 63;
int TYPE_ROW_MIN = 18;

void update_position(int keycode, int mods, int* x, int* y){
	if (keycode == RIGHT_ARROW && *x < SCREEN_ROWS-1)
		(*x)++;
	else if (keycode == LEFT_ARROW && *x > 0)
		(*x)--;
	else if (keycode == UP_ARROW && *y > 0)
		(*y)--;
	else if (keycode == DOWN_ARROW && *y < SCREEN_COLS-1)
		(*y)++;
}
