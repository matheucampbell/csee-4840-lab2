#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "keyhandler.h"

#define BUFFER_SIZE 128

// Updates cursor upon receiving arrow key input
void update_position(int keycode, int mods, char* buf, int* x, int* y){
	int hpos = (*y - TYPE_ROW_MIN)*SCREEN_COLS + *x;
	if (keycode == RIGHT_ARROW && hpos < strlen(buf)-1){ 
		if (*x < SCREEN_COLS)
			(*x)++;
		if (*x == SCREEN_COLS && *y < SCREEN_ROWS - 2){
			*x = 0;
			(*y)++;
		}
	}
	else if (keycode == LEFT_ARROW && hpos > 0){
		if (*x > 0)
			(*x)--;
		if (*x == 0 && (*y - TYPE_ROW_MIN) > 0){ // not in first row
			*x = SCREEN_COLS-1;
			(*y)--;
		}
	}
	else if (keycode == UP_ARROW && *y > TYPE_ROW_MIN && hpos - SCREEN_COLS < strlen(buf)){
		(*y)--;
	}
	else if (keycode == DOWN_ARROW && *y < SCREEN_ROWS-2 && hpos + SCREEN_COLS > 0){
		(*y)++;
	}
}

// Updates text buffer upon receiving text, then moves cursor
void parse_letters(int keycode, int mods, char* buf, int* x, int* y){
	int bufpos = (*y - TYPE_ROW_MIN)*SCREEN_COLS + *x;
	char* pp = &buf[bufpos]; // Partition pointer (where the cursor is)
	char* tmp = (char*) malloc(BUFFER_SIZE*sizeof(char*)); // Stores pp until the end for strcpy
	char c;
	
	if ((keycode | mods) == 0 || strlen(buf) == (SCREEN_ROWS-TYPE_ROW_MIN-1)*SCREEN_COLS)
		return;

	// a through z (lowercase) 
	if (0x00 <= keycode && keycode <= 0x1c){
		c = mods == 0x02 ? keycode + 61: keycode + 93;
		printf("%c received. Cursor now at %d.\n", c, bufpos+1);
		
		// Insert new letter at cursor position
		strcpy(tmp, pp);
		strcpy(pp+1, tmp);
		buf[bufpos] = c;
		
		update_position(RIGHT_ARROW, mods, buf, x, y);
	}
	
	free(tmp);
}

// Checks for backspace and enter
void parse_entry(int keycode, int mods, char* buf, int* x, int* y){
	int bufpos = (*y - TYPE_ROW_MIN)*SCREEN_COLS + *x;
	char* pp = &buf[bufpos];
	char* tmp = (char*) malloc(BUFFER_SIZE*sizeof(char*));
	
	// Backspace
	if (keycode == 0x2a && bufpos > 0){
		printf("BACKSPACE received. Cursor at %d.\n", bufpos-1);
		
		strcpy(tmp, pp);
		strcpy(pp-1, tmp);
		
		update_position(LEFT_ARROW, mods, buf, x, y);
	}

	free(tmp);
}
