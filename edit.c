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

int main()
{
  int err, col;

  struct usb_keyboard_packet packet;
  struct usb_keyboard_packet packet_l;  // Last packet
  int transferred;
  char keystate[12];
  int new_press;  // Newly pressed non-mod keys

  char* textbuf = (char*) malloc(BUFFER_SIZE * sizeof(char));
  textbuf[0] = '\0';
  // packet_l = {0, 0, 0};
  
  if ((err = fbopen()) != 0) {
    fprintf(stderr, "Error: Could not open framebuffer: %d\n", err);
    exit(1);
  }

  for (int row=0; row < SCREEN_ROWS; row++){
	for (int col=0; col<SCREEN_COLS; col++){
		fbputchar(' ', row, col, 255, 255, 255);
	}
  }

  for (int col=0; col<SCREEN_COLS; col++)
    fbputchar('-', TYPE_ROW_MIN-1, col, 255, 255, 255);

  fbputchar(cursor, TYPE_ROW_MIN, 0, 255, 255, 255);

  /* Draw rows of asterisks across the top and bottom of the screen */
  for (col = 0 ; col < 64 ; col++) {
    fbputchar('*', 0, col, 255, 255, 255);
    fbputchar('*', SCREEN_ROWS-1, col, 255, 255, 255);
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
      
      // Extract new presses from last packet and this packet
	   update_pressed(&new_press, packet.keycode, packet_l.keycode);
	   // Change cursor position if arrows clicked
	   update_position(new_press, packet.modifiers, textbuf, &curx, &cury);
	   // Parse letters if letters pressed
	   parse_letters(new_press, packet.modifiers, textbuf, &curx, &cury);
  	   // Check for backspace and enter
	   parse_entry(new_press, packet.modifiers, textbuf, &curx, &cury);

      if (curx != lastx || cury != lasty){
		  fbputchar(' ', lasty, lastx, 255, 255, 255);
	   } 

      fbclear(21, 22);
	   fbputlongs(textbuf, TYPE_ROW_MIN, 0, 2, SCREEN_COLS); 
	   fbputchar(cursor, cury, curx, 255, 255, 255);
	   
      packet_l = packet;
    }
  }

  return 0;
}

