/*********************************************/
/**************** TD SR03 n1 *****************/
/* Antoine Rondelet and Pierre-Louis Lacorte */
/*********************************************/

#include <sys/types.h>
#include <sys/socket.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>
#include <netinet/in.h>
#include <netdb.h>
#include "iniobj.h"

// handleServerResponse returns 0 in case of success and -1 in case of failure
int handleServerResponse(int sd) {
  int err;
  respHeader header;
  respMessage message;
  char *messageExtracted;

  err = recv(sd, &header, sizeof(respHeader), 0);
  if (err == -1) {
    return -1;
  }

  printf("[DEBUG] received header from the server: header = %d\n", header.number);
  for(int i=0; i<header.number; i++) {
    err = recv(sd, &message, sizeof(respMessage), 0);
    if (err == -1) {
      return -1;
    }

    printf("[DEBUG] received message from the server: %s\n", message.content);
  }

  return 0;
}


// initSocket returns -1 if an error occured and the socker descriptor in case of success
int initSocket(char* hostname, int port) {
	int sd, err;
	struct hostent * hp;
	struct sockaddr_in sin;

  sd = socket(AF_INET, SOCK_STREAM, 0);
  if (sd == -1) {
    return -1;
  }

	bzero(&sin, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_port = port;

	// Get the host ip from its name.
	hp = gethostbyname(hostname);
  if (hp == NULL) {
    perror("Could not resolve the hostname");
    exit(1);
  }

	bcopy(hp->h_addr, &sin.sin_addr, hp->h_length);
	sin.sin_family = hp->h_addrtype;
	sin.sin_port = htons(port);

	// Get an address and bind it
	err = connect(sd, (struct sockaddr *)&sin, sizeof(sin));
  if (err == -1) {
    perror("Could not initiate a connection on a socket");
    exit(1);
  }

  return sd;
}

int main(int argc, char *argv[]) {
  // Don't forget that the program itself is included in argv
  if (argc != 3) {
    printf("Wrong number of arguments\n");
    printf("Usage: ./%s <server-hostname> <server-port>\n", argv[0]);
    exit(1);
  }

	int sd, err;
	struct hostent * hp;
	struct sockaddr_in sin;
	obj message;


	// --- Set the socket --- //
  sd = initSocket(argv[1], atoi(argv[2]));
  if (sd == -1) {
    perror("Could not initialize the socket");
    exit(1);
  }

  // --- Send messages --- //
	int i;
	for (i=0; i<3; i++) {
		message = tabobjs[i];
    // Sleep to simulate a long exchange with the server
		sleep(1);
		err = send(sd, &message, sizeof(obj), 0);
    if (err == -1) {
      perror("Could not send the message from the socket");
      exit(1);
    }

		printf("Message sent: %s\n", message.description);
	}

  // --- Wait for the server's response --- //
  printf("Waiting for the server's response\n");
  err = handleServerResponse(sd);
  if (err == -1) {
    perror("Could not handle server response");
    exit(1);
  }

  printf("Reponse received\n");
  // Send "end" message to the server to indicate that the connection can be closed
  if(send(sd, &endConnection, sizeof(obj), 0) == -1) {
    perror("Could not notify end of connection to the server");
    exit(1);
  }

	printf("Connexion closed client side\n");
	return 0;
}
