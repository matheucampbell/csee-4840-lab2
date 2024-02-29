#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libusb-1.0/libusb.h>
#include "keyhandler.h"

#define BUFFER_SIZE 128

static int presscheck(int* presses, int target){
	return (presses[0] == target || presses[1] == target);
}

// Updates new_press and stores newly pressed keys
void update_pressed(int* new_p, uint8_t* new, uint8_t* old){
	*new_p = 0;
	if (new[0] != old[0] && new[0] != old[1]): *new_p = new[0];
	if (new[1] != old[0] && new[1] != old[1]): *new_p = new[1];
}

// Updates cursor upon receiving arrow key input
void update_position(int pressed, int mods, char* buf, int* x, int* y){
	int hpos = (*y - TYPE_ROW_MIN)*SCREEN_COLS + *x;
	if (pressed == RIGHT_ARROW && hpos < strlen(buf)-1){ 
		if (*x < SCREEN_COLS)
			(*x)++;
		if (*x == SCREEN_COLS && *y < SCREEN_ROWS - 2){
			*x = 0;
			(*y)++;
		}
	}
	else if (pressed == LEFT_ARROW && hpos > 0){
		if (*x > 0)
			(*x)--;
		if (*x == 0 && (*y - TYPE_ROW_MIN) > 0){ // not in first row
			*x = SCREEN_COLS-1;
			(*y)--;
		}
	}
	else if (pressed == UP_ARROW && *y > TYPE_ROW_MIN && hpos - SCREEN_COLS < strlen(buf)){
		(*y)--;
	}
	else if (presses == DOWN_ARROW && *y < SCREEN_ROWS-2 && hpos + SCREEN_COLS > 0){
		(*y)++;
	}
}

// Updates text buffer upon receiving text, then moves cursor
void parse_letters(int keycode_a, int keycode_b, int mods, char* buf, int* x, int* y){
	int bufpos = (*y - TYPE_ROW_MIN)*SCREEN_COLS + *x;
	char* pp = &buf[bufpos]; // Partition pointer (where the cursor is)
	char* tmp = (char*) malloc(BUFFER_SIZE*sizeof(char*)); // Stores pp until the end for strcpy
	char c;
	
	if ((keycode_a | mods) == 0 || strlen(buf) == (SCREEN_ROWS-TYPE_ROW_MIN-1)*SCREEN_COLS)
		return;

	// a through z (upper and lowercase) 
	if (0x00 < keycode_a && keycode_a <= 0x1c){
		c = mods == 0x02 ? keycode_a + 61: keycode_a + 93;
		printf("%c received. Cursor now at %d.\n", c, bufpos+1);
		
		// Insert new letter at cursor position
		strcpy(tmp, pp);
		strcpy(pp+1, tmp);
		buf[bufpos] = c;
		
		// update_position(RIGHT_ARROW, 0, mods, buf, x, y);
	}
	
	free(tmp);
}

// Checks for backspace and enter
void parse_entry(int keycode_a, int keycode_b, int mods, char* buf, int* x, int* y){
	int bufpos = (*y - TYPE_ROW_MIN)*SCREEN_COLS + *x;
	char* pp = &buf[bufpos];
	char* tmp = (char*) malloc(BUFFER_SIZE*sizeof(char*));
	
	// Backspace
	if (keycode_a == 0x2a && bufpos > 0){
		printf("BACKSPACE received. Cursor at %d.\n", bufpos-1);
		
		strcpy(tmp, pp);
		strcpy(pp-1, tmp);
		
		// update_position(LEFT_ARROW, 0, mods, buf, x, y);
	}

	free(tmp);
}
