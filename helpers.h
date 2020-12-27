#ifndef _HELPERS_H
#define _HELPERS_H 1

#include <stdio.h>
#include <stdlib.h>
#include "queue.h"

#define DIE(assertion, call_description)	\
	do {									\
		if (assertion) {					\
			fprintf(stderr, "(%s, %d): ",	\
					__FILE__, __LINE__);	\
			perror(call_description);		\
			exit(EXIT_FAILURE);				\
		}									\
	} while(0)

#define BUFLEN 1501 // max size of message
#define MAX_CLIENTS	150	// max no of connected clients
#define MAX_TOPICS 50 // max no of topics
#define MAX_SUBSCRIPTIONS 100 // max no of subscriptions for a client
#define MAX_MESSAGES 1000 // max no of message for 'store and forward'

struct client;
struct subscription;
struct tcpMsg;
struct udpMsg;
struct listOfClients;

// struct for a subscription
typedef struct subscription {
  char topic[50];
  int sf;
}TSubscription;

// the message sent to a tcp client
typedef struct tcpMsg {
  char ip[16];
  unsigned short int port;
  char topic[51];
  char dataType[11];
  char content[1501];
}TtcpMsg;

// the message received from a UDP client
typedef struct udpMsg {
  char topic[50];
  uint8_t dataType;
  char content[1501];
}TudpMsg;

// subscriber connected to server
typedef struct client {
  char id[10];
  TSubscription subscriptions[MAX_SUBSCRIPTIONS];
	int nr_subscriptions;
  TmessageQueue* messages;
	int is_connected;
  int socket;
}TClient;

typedef struct listOfClients {
	TClient** clients;
	int numOfClients;
}TClientList;



#endif
