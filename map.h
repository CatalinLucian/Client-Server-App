#ifndef MAP_H
#define MAP_H

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

// struct which represents an entry in the mapping topic->subscribed_clients
typedef struct topicEntry {
	char topic[50];
	TClientList* clientList;
}TtopicEntry;

// topic->subscribed_clients map
typedef struct map {
	int size;
	TtopicEntry** topicsAndClients;
}TtopicToClients;

// creates the map
TtopicToClients* createMap();

// search a specific topic
int searchTopic(TtopicToClients*, char*);

// create and add a new topic
void addTopicEntry(TtopicToClients*, char*);

// add new client to topic entry
// add subscribe to client subscriptions
// return 1 for succes, 0 for failure
int subscribe(TtopicToClients*, TClient*, char*, int, TClientList*);

// deletes client from topic entry
// adeletes subscribe from client subscriptions
// return 1 for succes, 0 for failure
int unsubscribe(TtopicToClients*, TClient*, char*, TClientList*);
#endif
