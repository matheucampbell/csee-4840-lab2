void update_position(char* keystate, int* x){
	if (*keystate[0:3] == RIGHT_ARROW)
		*x++;
	else if (*keystate[0:3] == LEFT_ARROW)
		*y--;
}
