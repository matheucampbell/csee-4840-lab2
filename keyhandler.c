#include <stdio.h>
#include <string.h>
#include "keyhandler.h"

// Updates cursor upon receiving arrow key input
void update_position(int keycode, int mods, char* buf, int* x, int* y){
	if (keycode == RIGHT_ARROW && *x < strlen(buf))
		(*x)++;
	else if (keycode == LEFT_ARROW && *x > 0)
		(*x)--;
	else if (keycode == UP_ARROW && *y > TYPE_ROW_MIN)
		(*y)--;
	else if (keycode == DOWN_ARROW && *y < SCREEN_ROWS-2)
		(*y)++;
}

// Updates text buffer upon receiving text, then moves cursor
void parse_letters(int keycode, int mods, char* buf, int* x, int* y){
	int bufpos = *x;
	char* pp = &buf[bufpos]; // Partition pointer (where the cursor is)
	char c;
	
	if ((keycode | mods) == 0)
		return;

	// a through z (lowercase) 
	if (0x00 <= keycode && keycode <= 0x1c){
		c = keycode + 93;
		printf("%c received. Cursor now at %d.\n", c, bufpos+1);
		
		// Insert new letter at cursor position
		strcpy(pp+1, pp);
		buf[bufpos] = c;
		update_position(RIGHT_ARROW, mods, buf, x, y);
	}
	
}

// Checks for backspace and enter
void parse_entry(int keycode, int mods, char* buf, int* x, int* y){
	int bufpos = *x;
	
	// Backspace
	if (keycode == 0x2a){
		printf("BACKSPACE received. Cursor at %d.\n", bufpos-1);
		// shift current and all other characters left one.
		update_position(LEFT_ARROW, mods, buf, x, y);
	}
}
