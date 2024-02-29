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
	char sendbuf[161] = "";
	char input[161] = "";
	char keyvalue[2];
	int length, cursor = 0;

  if ((err = fbopen()) != 0) {
    fprintf(stderr, "Error: Could not open framebuffer: %d\n", err);
    exit(1);
  }
	
	fbclear(0, 23);	// Clear screen

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
			keyvalue[0] = key_trans(keystate);
			keyvalue[1] = '\0';
			// My code
			//
			length = strlen(sendbuf);
			if (length < 64 * 2 + 32) {
      	if (keyvalue[0] == '\n') {
					write(sockfd, sendbuf, strlen(sendbuf));
					sendbuf[0] = '\0';
					input[0] = '\0';
					fbclear(21, 22);
				} else if (keyvalue[0] == (char)8) {
					strcpy(sendbuf + length + cursor - 1, sendbuf + length + cursor);
				} else if (keyvalue[0] == (char)17) {
					if (cursor < 0)
						cursor++;
				} else if (keyvalue[0] == (char)18) {
					if (cursor > -length)
						cursor--;
				} else if (keyvalue[0] == (char)19) {
					if (cursor < -63)
						cursor += 64;
				} else if (keyvalue[0] == (char)20) {
					if (cursor > -length + 63)
						cursor -= 64;
				} else if (keyvalue[0] != (char)0) {
					if (cursor == 0)
						strcat(sendbuf, keyvalue);
					else	
						strncpy(sendbuf + length + cursor, keyvalue, 1);
				}
			} else if (keyvalue[0] == '\n') {
					write(sockfd, sendbuf, strlen(sendbuf));
					sendbuf[0] = '\0';
					input[0] = '\0';
					fbclear(21, 22);
			} else if (keyvalue[0] == (char)8) {
					sendbuf[length + cursor - 1] = '\0';
			} else if (keyvalue[0] == (char)17) {
					if (cursor < 0)
						cursor++;
			} else if (keyvalue[0] == (char)18) {
					if (cursor > -length)
						cursor--;
			} else if (keyvalue[0] == (char)19) {
					if (cursor < -63)
						cursor += 64;
			} else if (keyvalue[0] == (char)20) {
					if (cursor > -length + 63)
						cursor -= 64;
			} 

			length = strlen(sendbuf);
			int num = cursor + length;	
			int rw = 21 + num / 64;	
			int cl = num % 64;
			int up = 0;
			if (length >= 128) {
				if (num <= 64) {
					up = 1;
				} else if (num < 128) {
					if (!up)
						rw--;
				} else {
					strcpy(input, sendbuf + 64);
					fbtype(21, 22, input);	
					up = 0;
					rw--;
				}
			} else {
				strncpy(input, sendbuf, 128);
				input[128] = '\0';
				fbtype(21, 22, input);
			}
			if (up) {
				strncpy(input, sendbuf, 128);
				input[128] = '\0';
				fbtype(21, 22, input);
			}
			fbcursor(rw, cl);

			printf("%s\n", sendbuf);

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
  /* Receive data */
  while ( (n = read(sockfd, &recvBuf, BUFFER_SIZE - 1)) > 0 ) {
    recvBuf[n] = '\0';
    printf("%s\n", recvBuf);
		fbinput(1, 19, recvBuf);
  }
	return NULL;
}

char key_trans(char *keyid)
{
	char symbol;
	int num[3]; 
	int i = 0;
	static int temp = 0;

	char *token = strtok(keyid, " ");
	while (token != NULL) {
		num[i] = (int)strtol(token, NULL, 16);
		token = strtok(NULL, " ");
		i++;
	}
	if (temp != num[1])
		temp = num[1];
	else
		temp = num[2];
	printf("KEYS:%d, %d, %d\n", num[0], num[1], num[2]);
	if (temp >= 4  && temp <= 29) {
		symbol = (char)(temp + 93);
	} else if (temp >= 30  && temp <= 38) {
		symbol = (char)(temp + 19);
	} else if (temp == 39) {
		symbol = '0';
	}	else if (temp == 44) {
		symbol = ' ';
	} else if (temp == 40) {
		symbol = '\n';
	} else if (temp == 42) {
		symbol = 8;
	} else if (temp >= 79 && temp <= 82) {
		symbol = (char)(temp - 62);
	} else if (temp == 0) {
		symbol = 0;
	} 
	
	return symbol;
}
