#ifndef _KEYHANDLER_H
#define _KEYHANDLER_H

#define CURSOR 0x23
#define RIGHT_ARROW 0x4f
#define LEFT_ARROW 0x50
#define UP_ARROW 0x52
#define DOWN_ARROW 0x51
 
#define SCREEN_ROWS 24
#define SCREEN_COLS 64
#define TYPE_ROW_MIN 18

void update_position(int, int, int*, int*);

#endif

