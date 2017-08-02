/*********************************************/
/**************** TD SR03 n1 *****************/
/* Antoine Rondelet and Pierre-Louis Lacorte */
/*********************************************/

#include "defobj.h"

#define tablen 3

const obj tabobjs[tablen] = {
	{"ident_o1", "description_o1", 11, 12, 10.2345, 1},
	{"ident_o2", "description_o2", 21, 22, 20.2345, 1},
	{"ident_o3", "description_o3", 31, 32, 30.2345, -1},
};


const obj endConnection = {"endConn", "end connection", 0, 0, 0, -1};
