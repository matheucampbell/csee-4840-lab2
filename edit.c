/*
 *
 * CSEE 4840 Lab 2 for 2019
 *
 * Name/UNI: Please Changeto Yourname (pcy2301)
 */

#include "fbputchar.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "usbkeyboard.h"
#include <pthread.h>
#include "keyhandler.h"

#define BUFFER_SIZE 128

/*
 * References:
 *
 * https://web.archive.org/web/20130307100215/http://beej.us/guide/bgnet/output/html/singlepage/bgnet.html
 *
 * http://www.thegeekstuff.com/2011/12/c-socket-programming/
 * 
 */

struct libusb_device_handle *keyboard;
uint8_t endpoint_address;

// Cursor initialization and positioning
char cursor = CURSOR;
int lastx, lasty;
int curx = 0;
int cury = TYPE_ROW_MIN;

// Text buffer constants
int BUFF_SIZE = SCREEN_COLS;

int main()
{
  int err, col;

  struct usb_keyboard_packet packet;
  int transferred;
  char keystate[12];

  char* textbuf = (char*) malloc(64 * sizeof(char));
  
  if ((err = fbopen()) != 0) {
    fprintf(stderr, "Error: Could not open framebuffer: %d\n", err);
    exit(1);
  }

  for (int row=0; row < SCREEN_ROWS; row++){
	for (int col=0; col<SCREEN_COLS; col++){
		fbputchar(' ', row, col);
	}
  }

  for (int col=0; col<SCREEN_COLS; col++)
    fbputchar('-', TYPE_ROW_MIN-1, col);

  fbputchar(cursor, TYPE_ROW_MIN, 0);

  /* Draw rows of asterisks across the top and bottom of the screen */
  for (col = 0 ; col < 64 ; col++) {
    fbputchar('*', 0, col);
    fbputchar('*', SCREEN_ROWS-1, col);
  }

  /* Open the keyboard */
  if ((keyboard = openkeyboard(&endpoint_address)) == NULL ) {
    fprintf(stderr, "Did not find a keyboard\n");
    exit(1);
  }
    
  /* Look for and handle keypresses */
  for (;;) {
	 // Save previous cursor position before proceeding
	lastx = curx;
	lasty = cury;

    libusb_interrupt_transfer(keyboard, endpoint_address,
			      (unsigned char *) &packet, sizeof(packet),
			      &transferred, 0);
    if (transferred == sizeof(packet)) {
      sprintf(keystate, "%02x %02x %02x", packet.modifiers, packet.keycode[0],
	      packet.keycode[1]);
      printf("%s\n", keystate);
      fbputs(keystate, 6, 0);
      if (packet.keycode[0] == 0x29) { /* ESC pressed? */
	 		free(textbuf);
			break;
      }
	
	  // Change cursor position	if arrows clicked
	  update_position(packet.keycode[0], packet.modifiers, textbuf, &curx, &cury);
	  // Parse letters if letters pressed
	  parse_letters(packet.keycode[0], packet.modifiers, textbuf, &curx, &cury);

	  // Render cursor and remove last cursor
      if (curx != lastx || cury != lasty){
		fbputchar(' ', lasty, lastx);
		fbputchar(cursor, cury, curx);
	  }

	  // Null terminate text buffer and print to screen
	  fbputs(textbuf, TYPE_ROW_MIN, 0);
	}
  }

  return 0;
}

