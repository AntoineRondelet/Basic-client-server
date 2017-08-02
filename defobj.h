/*********************************************/
/**************** TD SR03 n1 *****************/
/* Antoine Rondelet and Pierre-Louis Lacorte */
/*********************************************/

#define MAXSIZE 100
#define NBMAX 100

typedef struct {
	char id[12];
	char description[24];
	int ii;
	int jj;
	double dd;
  // We add another field to the structure to indicate the end of the
  // message stream from the client to the server (we define our proper protocol)
	int iqt;
} obj;

typedef struct {
  int number; // Indicate the number of messages in the response from the server
} respHeader;

typedef struct {
	int nbChars; // Number of characters in the string (part of the response)
	char content[MAXSIZE];
} respMessage;

typedef struct {
  char messages[NBMAX];
} response;

