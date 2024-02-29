#ifndef _KEYHANDLER_H
#define _KEYHANDLER_H

#include <libusb-1.0/libusb.h>

#define CURSOR 0x5f
#define RIGHT_ARROW 0x4f
#define LEFT_ARROW 0x50
#define UP_ARROW 0x52
#define DOWN_ARROW 0x51

#define ISLETTER(x) (0x04 <= x && x <= 0x1d)
#define LET_OFF 0x5d
#define LET_OFF_SH 0x3d

#define ISDIGIT(x) (0x1e <= x && x <= 0x26)
#define DIG_OFF 0x12
#define DIG_OFF_SH 0x03

#define ISZERO(x) (x == 0x27)
#define ZERO_OFF 0x12
#define ZERO_OFF_SH 0x01

#define SHIFT_MOD 0x02
 
#define SCREEN_ROWS 24
#define SCREEN_COLS 64
#define TYPE_ROW_MIN 21

void update_pressed(int*, uint8_t*, uint8_t*);
void update_position(int, int, char*, int*, int*);
void parse_letters(int, int, char*, int*, int*);
void parse_entry(int, int, char*, int*, int*);

#endif

