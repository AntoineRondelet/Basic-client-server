# Simple Client/Server

Usage of the client and the server:

1. In one terminal run the server:

```bash
    $ ./serveur <server-port>
```

2. In another terminal run the client:

```bash
    $ ./client <server-hostname> <server-port>
```

**NB** Compile a single file in C:

```bash
    $ gcc yourFileName.c -W -o YourExecutableName
```

In order to test the server with plenty of clients without launching them manually, one might use the `start.sh` shel script.
This script launches as many clients as specified, on a given port. 
Example of use:

```bash
    $ ./start.sh 1234 6
```

This will launch 6 clients doing requests on `localhost:1234`.

## Note

1. If we try to launch multiple clients to communicate with the server in this exercise, the second client we try to launch won't be able to run. In fact:
When we start the server and when it receives a request, then the serveur (parent process) creates a child process (using the system call `fork`). This child process manipulates a socket different from the first one (`newsd = accept(sd, 0, 0)`). This socket is called a socket of service and the port associated is chosen by the system. That way a server is able to listen on a listening socket (Associated to the port 1234 for instance), and is able to handle each separate request using different file descriptors is child processes. 

This is what happens when a server handles a connection from a client:

```bash
  $ lsof -i // returns the files descriptors of all internet network files

serveur   70718 Antoine    3u  IPv4 XXXXXXXXXXXXXX      0t0  TCP *:mosaicsyssvc1 (LISTEN)
client    70897 Antoine    3u  IPv4 XXXXXXXXXXXXXX      0t0  TCP localhost:54020->localhost:mosaicsyssvc1 (ESTABLISHED)
serveur   70898 Antoine    4u  IPv4 XXXXXXXXXXXXXX      0t0  TCP localhost:mosaicsyssvc1->localhost:54020 (ESTABLISHED)
```

We clearly see the creation of a socket of service and the creation of the socket for the client. The process 70718 is (LISTENING) and represents the parent process using a socket for listening only, and delegates connection handling to chil processes.

## Protocol specs

In this exercice we had to choose some conventions to make our server and clients able to communicate with one another. These conventions define the protocol which clients and server speak.

Here are the specifications of our very simple protocol:

### Client's requests

Clients requests messages are represented with a structure `obj`:

```bash
typedef struct {
  char id[12];
  char description[24];
  int ii;
  int jj;
  double dd;
  int iqt;
} obj;
```

Each field of the structure represent an information in the message. However, a "special" field is worth taking a look at: `iqt`.
When this field is equal to -1, this means that the request is done/complete and that the client is now waiting for a response from the server.

### Server's responses

The server responses are composed by two parts:
1. The header: Adds useful "metadata" for the client. In our case the header contains only a field `number` which indicates the number of messages that the response will contain.

This field allows the client to either allocate the good amount of memory to store the response and/or not to wait indefinitely. The client knows exactly how many messages it is about to get and stops after all messages have been delivered.

2. The "body": Contains a succession of messages. Each of them have 2 fields. The first one `nbChars` indicates the numbers of characters in the messages (for the client to know exactly the good amount of bytes). The second field is the `content` containing the chunk of information carried by this message.


**Note** The client stops reading when it receives the good amount of messages

### Client's connection close

Once all messages are received by the client, the latter sends a "special" message to the server to indicate it that he wants to close the connection. This message is:

```bash
const obj endConnection = {"endConn", "end connection", 0, 0, 0, -1};
```


After reading this message the server and the client close the TCP connection. The exchange of information is finished.
