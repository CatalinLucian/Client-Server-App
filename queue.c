#include "map.h"
#include "utils.h"
#include <stdlib.h>

TmessageNode* newMessage(char* message) {
  TmessageNode* aux = (TmessageNode*)calloc(1, sizeof(TmessageNode));
  aux->message = (char*)calloc(sizeof(TtcpMsg), sizeof(char));
  memcpy(aux->message, message, sizeof(TtcpMsg));
  aux->next = NULL;
  return aux;
}

TmessageQueue* createMessageQueue() {
  TmessageQueue* queue = (TmessageQueue*)calloc(1, sizeof(TmessageQueue));
  queue->front = NULL;
  queue->rear = NULL;
  return queue;
}

void enqueue(TmessageQueue* queue, char* message) {
  TmessageNode* aux = newMessage(message);
  if (queue->rear == NULL) {
    queue->front = aux;
    queue->rear = aux;
    return;
  }
  queue->rear->next = aux;
  queue->rear = aux;
}

void dequeue(TmessageQueue* queue) {
  if (queue->front == NULL) {
    return;
  }
  TmessageNode* aux = queue->front;
  queue->front = queue->front->next;

  if (queue->front == NULL) {
    queue->rear = NULL;
  }
  free(aux->message);
  free(aux);
}
