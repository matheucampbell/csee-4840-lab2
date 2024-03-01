#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libusb-1.0/libusb.h>
#include "keyhandler.h"

#define BUFFER_SIZE 128

// USB codes of symbols with shift variants (except letters and 0-9)
int sh_usb[11] = {45, 46, 47, 48, 49, 51, 52, 53, 54, 55, 56};
int nsh_ascii[11] = {45, 61, 91, 93, 92, 59, 39, 96, 44, 46, 47}; // non-shifted equiv
int sh_ascii[11] = {95, 43, 123, 125, 124, 58, 34, 126, 60, 62, 63}; // shifted equiv
int sh_digits[11] = {3, 33, 3, 3, 3, 59, 2, 5, 2, 2};  // offset for shifted digits

// Updates new_press and stores newly pressed keys
void update_pressed(int* new_p, uint8_t* new, uint8_t* old){
	*new_p = 0;
	if (new[0] != 0 && new[0] != old[0] && new[0] != old[1]) 
		*new_p = new[0];
	if (new[1] != 0 && new[1] != old[0] && new[1] != old[1]) 
		*new_p = new[1];
}

// Updates cursor upon receiving arrow key input
void update_position(int pressed, int mods, char* buf, int* x, int* y){
	int hpos = (*y - TYPE_ROW_MIN)*SCREEN_COLS + *x;
	if (pressed == RIGHT_ARROW && hpos <= strlen(buf) - 1){
		if (*x < SCREEN_COLS - 1){
			(*x)++;
		} else if (*x == SCREEN_COLS - 1 && *y < SCREEN_ROWS - 2){
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
	else if (pressed == UP_ARROW && *y > TYPE_ROW_MIN && hpos - SCREEN_COLS >= 0){
		(*y)--;
	}
	else if (pressed == DOWN_ARROW && *y < SCREEN_ROWS-2 && hpos + SCREEN_COLS < strlen(buf)){
		(*y)++;
	}
}

// Updates text buffer upon receiving text, then moves cursor
void parse_symbols(int keycode, int mods, char* buf, int* x, int* y){
	int bufpos = (*y - TYPE_ROW_MIN)*SCREEN_COLS + *x;
	char* pp = &buf[bufpos]; // Partition pointer (where the cursor is)
	char* tmp = (char*) malloc(BUFFER_SIZE*sizeof(char*)); // Stores pp until the end for strcpy
	char c = 0;
	
	if ((keycode | mods) == 0 || strlen(buf) == (SCREEN_ROWS-TYPE_ROW_MIN-1)*SCREEN_COLS)
		return;

	if (ISLETTER(keycode))
		c = ISSHIFT(mods) ? keycode + LETT_OFF_SH: keycode + LETT_OFF;
	else if (ISDIGIT(keycode) && !ISSHIFT(mods))
		c = keycode + DIG_OFF;
	else if (ISDIGIT(keycode) && ISSHIFT(mods))
		c = keycode + sh_digits[keycode - 30];
	else if (ISZERO(keycode))
		c = ISSHIFT(mods) ? keycode + ZERO_OFF_SH : keycode + ZERO_OFF;
	else if (ISSPACE(keycode))
		c = SPACE;
	else if (ISTAB(keycode))
		c = TAB;
	else {
		for (int i = 0; i < 11; i++){
			if (keycode == sh_usb[i])
				c = ISSHIFT(mods) ? sh_ascii[i] : nsh_ascii[i];
		}
	}
	
	if (c != 0 && strlen(buf) < SCREEN_COLS*2 - 1){	
		printf("%c received. Cursor now at %d.\n", c, bufpos+1);
		// Insert new letter at cursor position
		strcpy(tmp, pp);
		strcpy(pp+1, tmp);
		buf[bufpos] = c;
		
		update_position(RIGHT_ARROW, 0, buf, x, y);
	} else if (keycode == BACKSPACE && bufpos > 0) {
		printf("BACKSPACE received. Cursor at %d.\n", bufpos-1);
		
		strcpy(tmp, pp);
		strcpy(pp-1, tmp);
		
		update_position(LEFT_ARROW, mods, buf, x, y);
	}
	
	free(tmp);
}

