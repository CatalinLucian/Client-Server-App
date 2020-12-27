#ifndef QUEUE_H
#define QUEUE_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <math.h>

typedef struct messageNode {
	char* message;
	struct messageNode* next;
}TmessageNode;

typedef struct messageQueue {
	TmessageNode* front;
	TmessageNode* rear;
}TmessageQueue;


// create new message
TmessageNode* newMessage(char*);

// create queue message
TmessageQueue* createMessageQueue();

// add message to queue
void enqueue(TmessageQueue*, char*);

// delete message from queue
void dequeue(TmessageQueue*);

#endif
