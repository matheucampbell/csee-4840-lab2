#ifndef _KEYHANDLER_H
#define _KEYHANDLER_H

#define CURSOR 0x23
#define RIGHT_ARROW 0x4f
#define LEFT_ARROW 0x50
#define UP_ARROW 0x52
#define DOWN_ARROW 0x51

// Below defined in keyhandler.c
extern const int SCREEN_ROWS;
extern const int SCREEN_COLS;
extern const int TYPE_ROW_MIN;

void update_position(int, int, int*, int*);

#endif

