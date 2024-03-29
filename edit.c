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

/* Update SERVER_HOST to be the IP address of
 * the chat server you are connecting to
 */
/* arthur.cs.columbia.edu */
#define SERVER_HOST "128.59.19.114"
#define SERVER_PORT 42000

#define BUFFER_SIZE 256

/*
 * References:
 *
 * https://web.archive.org/web/20130307100215/http://beej.us/guide/bgnet/output/html/singlepage/bgnet.html
 *
 * http://www.thegeekstuff.com/2011/12/c-socket-programming/
 * 
 */

int sockfd; /* Socket file descriptor */

pthread_t network_thread;
void *network_thread_f(void *);

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

  struct sockaddr_in serv_addr;
  struct usb_keyboard_packet packet;
  struct usb_keyboard_packet packet_l = {.modifiers = 0, .reserved = 0, .keycode = {(uint8_t) 0}};  // Last packet

  int transferred;
  char keystate[12];
  int new_press;  // Newly pressed non-mod keys

  char* textbuf = (char*) malloc(BUFFER_SIZE * sizeof(char));
  textbuf[0] = '\0';
  
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

  fbcursor(TYPE_ROW_MIN, 0);

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

  /* Create a TCP communications socket */
  if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) {
    fprintf(stderr, "Error: Could not create socket\n");
    exit(1);
  }

  /* Get the server address */
  memset(&serv_addr, 0, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(SERVER_PORT);
  if ( inet_pton(AF_INET, SERVER_HOST, &serv_addr.sin_addr) <= 0) {
    fprintf(stderr, "Error: Could not convert host IP \"%s\"\n", SERVER_HOST);
    exit(1);
  }

  /* Connect the socket to the server */
  if ( connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
    fprintf(stderr, "Error: connect() failed.  Is the server running?\n");
    exit(1);
  }

  /* Start the network thread */
  pthread_create(&network_thread, NULL, network_thread_f, NULL);   
  
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
      // fbputs(keystate, 6, 0);
      if (packet.keycode[0] == 0x29) { /* ESC pressed? */
	 	  free(textbuf);
		  break;
      }
      
      // Extract new presses from last packet and this packet
      update_pressed(&new_press, packet.keycode, packet_l.keycode);
	   // Change cursor position if arrows clicked
	   update_position(new_press, packet.modifiers, textbuf, &curx, &cury);
	   // Parse letters if letters pressed
	   parse_symbols(new_press, packet.modifiers, textbuf, &curx, &cury);

      if (curx != lastx || cury != lasty){
		  fbputchar(' ', lasty, lastx, 255, 255, 255);
	   } 
		
		// Checks for enter
		if (new_press == ENTER){
		  fbtype(21, 22, textbuf);
		  write(sockfd, textbuf, strlen(textbuf));
		  curx = 0;
		  cury = TYPE_ROW_MIN;
		  textbuf[0] = '\0';
		}
      
		fbclear(21, 22);
	   fbputlongs(textbuf, TYPE_ROW_MIN, 0, 2, SCREEN_COLS); 
	  	fbcursor(cury, curx);

      packet_l = packet;	
    }
  }
  
  /* Terminate the network thread */
  pthread_cancel(network_thread);

  /* Wait for the network thread to finish */
  pthread_join(network_thread, NULL);

  return 0;
}

void *network_thread_f(void *ignored)
{
  char recvBuf[BUFFER_SIZE];
  int n;
  /* Receive data */
  while ( (n = read(sockfd, &recvBuf, BUFFER_SIZE - 1)) > 0 ) {
    recvBuf[n] = '\0';
    printf("%s\n", recvBuf);
		fbinput(1, 19, recvBuf);
  }
	return NULL;
}

