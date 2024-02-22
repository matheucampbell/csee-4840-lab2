#include "keyhandler.h"

void update_position(int keycode, int mods, int* x, int* y){
	if (keycode == RIGHT_ARROW)
		(*x)++;
	else if (keycode == LEFT_ARROW)
		(*x)--;
	else if (keycode == UP_ARROW)
		(*y)--;
	else if (keycode == DOWN_ARROW)
		(*y)++;
}
