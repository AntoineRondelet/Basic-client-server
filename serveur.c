/*********************************************/
/**************** TD SR03 n1 *****************/
/* Antoine Rondelet and Pierre-Louis Lacorte */
/*********************************************/


#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>

#include <netinet/in.h>
#include <strings.h>
#include <signal.h>

#include "iniobj.h"

#define MAX_LENGTH_RESPONSE 10
#define BACKLOG 5

// Define the type bool
typedef int bool;
#define true 1
#define false 0

int nbConnections;

bool isEndConnection(obj message) {
  if (*message.id == *endConnection.id && *message.description == *endConnection.description && message.ii == endConnection.ii && message.jj == endConnection.jj && message.dd == endConnection.dd && message.iqt == endConnection.iqt) {
    printf("[CLIENT] End connection received\n");
    return true;
  }

  return false;
}

void SIGCHLD_handler(int sig) {
  pid_t processID;

  while (nbConnections) {
    // Use WNOHANG to indicate that the call should not blockif there are no processes
    // that wih to report the status, in that case the returned value of waitpid is 0
    processID = waitpid((pid_t)-1, NULL, WNOHANG);
    if (processID == -1) {
      perror("Could not wait child process to finish");
      exit(1);
    }

    // No child to wait, so we exit
    if (processID == 0) {
        break;
    } else {
        nbConnections--;
    }
  }
}

// handleClientConn returns -1 if an error occured and 0 if the exchange with the client is finished
int handleClientConn(int sd) {
  int err;
	obj message;
  respHeader header;

  srand(time(NULL));
  while(1) {
    err = recv(sd, &message, sizeof(obj),0);
    if (err == -1) {
      return -1;
    }

    printf("[Client] Received (description): %s\n", message.description);

    // message.iqt = -1 indicates that the request from the client is complete. We can respond now.
    if (message.iqt == -1) {
      printf("[Client] End of the request: server builds response\n");

      // Generate a random number of messages composing the response
      // (to prevent the server from sending the same answer to all clients)
      int randomNumber = rand();
      header.number = (randomNumber % MAX_LENGTH_RESPONSE) + 1;

      // Send header of the respnse back to the client to inform it how many messages are coming
      printf("[DEBUG] send header response to client\n");
      err = send(sd, &header, sizeof(respHeader), 0);
      if (err == -1) {
        return -1;
      }

      printf("[DEBUG] send %d messages back to the client\n", header.number);
      respMessage messageResponse;
      for (int i=0; i<header.number; i++) {
        // Sleep to simulate a long exchange with the client
        sleep(1);
        err = sprintf(messageResponse.content, "Message number %d in the response", i+1);
        if (err == -1) {
          return -1;
        }

        messageResponse.nbChars = sizeof(messageResponse.content) / sizeof(char);

        printf("[DEBUG] send message to client\n");
        send(sd, &messageResponse, sizeof(respMessage), 0);
      }

      // End of the response from the server
      return 0;
    }
  }
}

// initSocket returns -1 if an error occured and the socker descriptor in case of success
int initSocket(int port) {
	int sd;
	struct sockaddr_in sin;
	int err;

	// ---- Set the socket ---- //
  // Options:
  // Domain: Defines a group of sockets with which a given process will be able to communicate.
  // AF_INET: Allows process located on different machines on the same network to communicate (TCP-IP)
  // Type of socket:
  // SOCK_STREAM: Data stream in connected mode (TCP)
  // Protocol:
  // 0 corresponds to the internet protocol (it is always going to be 0, and was just added as a previsions for the future)
  sd = socket(AF_INET, SOCK_STREAM, 0);
  if (sd == -1) {
    return -1;
  }

	// ---- Initialize the structures with the zero values (always do it manually in C) ---- //
	bzero(&sin, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_port = port;

  // ---- Bind a name to a socket ---- //
  // sd = Socket descriptor
	err = bind(sd, (struct sockaddr *)&sin, sizeof(sin));
  if (err == -1) {
    return -1;
  }

	// ---- Listen up to BACKLOG connections ---- //
  // Backlog defines the maximum length for the queue of pending connections
	err = listen(sd, BACKLOG);
	if (err == -1) {
    return -1;
	}

  return sd;
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    printf("Wrong number of arguments\n");
    printf("Usage: ./%s <server-port>\n", argv[0]);
    exit(1);
  }

	int sd, err, port;
  pid_t npid;
  obj message;

  // The port number should be positive
  if (atoi(argv[1]) < 0) {
    perror("Wrong port number. The port should be a positive number");
    exit(1);
  }

  // --- Convert the port value between host and network byte order --- //
  // Paramount if a process is communicating with another process running on a different machine (not the same hardware and conventions from one machine to another)
  port = htons(atoi(argv[1]));
  sd = initSocket(port);
  if (sd == -1) {
    perror("Could not initiliaze the socket");
    exit(1);
  }

  // Create handler for SIGCHLD signals
  struct sigaction sigchld_received;
  sigchld_received.sa_handler = &SIGCHLD_handler;
  sigchld_received.sa_flags = SA_RESTART;

  err = sigaction(SIGCHLD, &sigchld_received, NULL);
  if (err == -1) {
    perror("Could not handle signal SIGCHLD");
    exit(1);
  }

  // We use an infinite loop since the matter of a server is to never stop and serve clients.
  while(1) {
    printf("Server (PID=%d) listening on %d\n", getpid(), ntohs(port));

    // --- Accept a connection on a socket --- //
    // Extracts the first connection request on the queue of pending connections
    // This waits for incoming connections and creates a service socket once a connection is handled
    int newsd;
    newsd = accept(sd, NULL, NULL);
    if (newsd == -1) {
      // Do not throw an error: Wait for incoming connections and do not stop the server
      continue;
    }

    // --- Create child process to handle the connection --- //
    npid = fork();
    if(npid == -1) {
      perror("Could not handle request because cannot create child process");
      continue;
    }

    // --- Child process --- //
    if (npid == 0) {
      printf("Handling client connection in process %d using the socket of service n°%d\n", (int)getpid(), newsd);

      // Close the listening socket since the memory is shared when we create a child with fork
      // The child process only reads on the socket of service
      close(sd);

      err = handleClientConn(newsd);
      if (err == -1) {
        perror("Could not handle the client request properly");
        exit(1);
      }

      // --- Read if the client wants to close the TCP connection --- //
      err = recv(newsd, &message, sizeof(obj),0);
      if (err == -1) {
        return -1;
      }
      if (!isEndConnection(message)) {
        printf("Connection stays open...");
      }

      exit(0);
    }

    // We close the descriptor of the socket of service since this is only used in the child process
    close(newsd);
    nbConnections++;
    printf("Number of connections: %d\n", nbConnections);
 }

	return 0;
}
