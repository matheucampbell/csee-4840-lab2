#include "keyhandler.h"

void update_position(int keycode, int mods, int* x){
	if (keycode == RIGHT_ARROW)
		(*x)++;
	else if (keycode == LEFT_ARROW)
		(*x)--;
}
