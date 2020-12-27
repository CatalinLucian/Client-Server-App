#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <math.h>
#include "helpers.h"
#include "map.h"
#include "queue.h"

// init client list for a topic entry
TClientList* initClientList();

// find index of subscriber from the client list depending on his name passed
// as argument to the function
// if the client does not exist, -1 is returned
int findClient(char*, TClient**, int);

// find index of subscriber from the client list depending on his opened socket
// passed as argument to the function
// if the client does not exist, -1 is returned
int findClientBySocket(int, TClient**, int);

// check if client has 'store and forward activated' or not
int checkIfSF(char*, TClient*);

// create a TClient* struct
TClient* createClient(int, char*);

// add client to the list of clients
void addClient(TClientList*, TClient*);

// convert from message received from udp client to message for tcp client
int convertMessage(TudpMsg*, TtcpMsg*);

// send messages to those clients which reconnected after they disconnected but
// had 'store and forward' activated
int sendQueueMessages(TClient*);

void freeResources(TClientList*);
#endif
