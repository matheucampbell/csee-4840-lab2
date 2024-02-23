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

/* Update SERVER_HOST to be the IP address of
 * the chat server you are connecting to
 */
/* arthur.cs.columbia.edu */
#define SERVER_HOST "128.59.19.114"
#define SERVER_PORT 42000

#define BUFFER_SIZE 128
#define COL_NUM 64
#define ROW_NUM 24
/*
 * References:
 *
 * https://web.archive.org/web/20130307100215/http://beej.us/guide/bgnet/output/html/singlepage/bgnet.html
 *
 * http://www.thegeekstuff.com/2011/12/c-socket-programming/
 * 
 */

int sockfd; /* Socket file descriptor */

struct libusb_device_handle *keyboard;
uint8_t endpoint_address;

pthread_t network_thread;
void *network_thread_f(void *);
char key_trans(char *keyid);

int main()
{
  int err, col;

  struct sockaddr_in serv_addr;

  struct usb_keyboard_packet packet;
  int transferred;
  char keystate[12];
	char keyvalue;

  if ((err = fbopen()) != 0) {
    fprintf(stderr, "Error: Could not open framebuffer: %d\n", err);
    exit(1);
  }
	
	fbclear();	// Clear screen

  /* Draw rows of asterisks across the top and bottom of the screen */
  for (col = 0 ; col < 64 ; col++) {
    fbputchar('*', 0, col, 255, 255, 255);
    fbputchar('*', 23, col, 255, 255, 255);
    fbputchar('-', 20, col, 255, 255, 255);	// Devided line *
  }

  //fbputs("Hello CSEE 4840 World!", 4, 10);

  /* Open the keyboard */
  if ( (keyboard = openkeyboard(&endpoint_address)) == NULL ) {
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
    libusb_interrupt_transfer(keyboard, endpoint_address,
			      (unsigned char *) &packet, sizeof(packet),
			      &transferred, 0);
    if (transferred == sizeof(packet)) {
      sprintf(keystate, "%02x %02x %02x", packet.modifiers, packet.keycode[0],
	      packet.keycode[1]);
			keyvalue = key_trans(keystate);
      printf("%s\n", keystate);
      fbputs(keystate, 21, 0);
      fbputchar(keyvalue, 22, 0, 255, 255, 255);
      if (packet.keycode[0] == 0x29) { /* ESC pressed? */
	break;
      }
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
	int i, rows;
	rows = 1;
	char outStr[COL_NUM + 1];
  /* Receive data */
  while ( (n = read(sockfd, &recvBuf, BUFFER_SIZE - 1)) > 0 ) {
    recvBuf[n] = '\0';
    printf("%s", recvBuf);
		if (strlen(recvBuf) > 64) {
			while (strlen(recvBuf) > 64) {
				i = COL_NUM;
				do {
					i--;
					if (i == 0) {
						i = COL_NUM;
						break;
					}
				} while (recvBuf[i] != ' ');
				strncpy(outStr, recvBuf, i);
				fbputs(outStr, rows, 0);
				strcpy(recvBuf, recvBuf + i + 1);
				if (rows != 19) rows++;
				else fbscroll(1, 19, 1);
			}
  	} else {
				if (rows != 19) rows++;
				else fbscroll(1, 19, 1);
		}
		fbputs(recvBuf, rows, 0);
  }
	return NULL;
}

char key_trans(char *keyid)
{
	char symbol;
	int num[3]; 
	int i = 0;

	char *token = strtok(keyid, " ");
	while (token != NULL) {
		num[i] = (int)strtol(token, NULL, 16);
		token = strtok(NULL, " ");
		i++;
	}
	num[1] += 93;
	if (num[1] > 92 && num[1] <123) {
		symbol = (char)num[1];
	} else {
		symbol = 'x';
	}
	return symbol;
}
