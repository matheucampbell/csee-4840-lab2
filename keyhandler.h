#ifndef _KEYHANDLER_H
#define _KEYHANDLER_H

#include <libusb-1.0/libusb.h>

#define CURSOR 0x5f
#define RIGHT_ARROW 0x4f
#define LEFT_ARROW 0x50
#define UP_ARROW 0x52
#define DOWN_ARROW 0x51

#define ISLETTER(x) (0x04 <= x && x <= 0x1d)
 
#define SCREEN_ROWS 24
#define SCREEN_COLS 64
#define TYPE_ROW_MIN 21

void update_pressed(int*, uint8_t*, uint8_t*);
void update_position(int, int, char*, int*, int*);
void parse_letters(int, int, char*, int*, int*);
void parse_entry(int, int, char*, int*, int*);

#endif

