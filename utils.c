#include "utils.h"
#include "queue.h"
#include "map.h"

TClientList* initClientList() {
  TClientList* clientList = (TClientList*)calloc(1, sizeof(TClientList));
  DIE(clientList == NULL, "calloc error");
  clientList->clients = (TClient**)calloc(MAX_CLIENTS, sizeof(TClient*));
  DIE(clientList->clients == NULL, "calloc error");
  clientList->numOfClients = 0;
  return clientList;
}

TClient* createClient(int sock, char* id) {
  TClient* client = (TClient*)calloc(1, sizeof(TClient));
  DIE(client == NULL, "calloc");
  client->messages = createMessageQueue();
  client->is_connected = 1;
  client->nr_subscriptions = 0;
  client->socket = sock;
  strcpy(client->id, id);
  return client;
}

void addClient(TClientList* clientList, TClient* client) {
  int pos = findClient(client->id, clientList->clients, clientList->numOfClients);
  // if clients did connect to server in the past then his queue messages are
  // being sent and his corresponding fiels are being updated
  if (pos != -1) {
    clientList->clients[pos]->is_connected = 1;
    clientList->clients[pos]->socket = client->socket;
    sendQueueMessages(clientList->clients[pos]);
    free(client->messages);
    free(client);
    return;
  }
  // else the server add him to list of connected clients
  clientList->clients[clientList->numOfClients] = client;
  clientList->numOfClients++;
}

// find client by id
// returns his position
// -1 if it does not exist
int findClient(char* id, TClient** clients, int numOfClients) {
  if (id != NULL) {
    for (int i = 0; i < numOfClients; i++) {
      if (strcmp(id, clients[i]->id) == 0) {
        return i;
      }
    }
  }
  return -1;
}

// find client by socket
int findClientBySocket(int socket, TClient** clients, int numOfClients) {
  for (int i = 0; i < numOfClients; i++) {
    if (clients[i]->socket == socket) {
      return i;
    }
  }
  return -1;
}

// check if 'store and forward' is activated
int checkIfSF(char* topic, TClient* client) {
  for (int i = 0; i < client->nr_subscriptions; i++) {
    if (strcmp(client->subscriptions[i].topic, topic) == 0) {
      if (client->subscriptions[i].sf == 1) {
        return 1;
      } else return 0;
    }
  }
  return -1;
}

int convertMessage(TudpMsg* udpMsg, TtcpMsg* toBeSent) {
  if (udpMsg->dataType > 3) {
    printf("Data type is not recognized\n");
    return 0;
  }

  if (udpMsg->dataType == 0) {
    memcpy(&(toBeSent->dataType), "INT", 3);
    toBeSent->dataType[3] = '\0';
    uint32_t n;
    memcpy(&n, &(udpMsg->content[1]), 4); // copy 4 bytes in n
    n = ntohl(n); // then prelucrate it
    if (udpMsg->content[0] == 1) {
      n *= -1;
    }
    sprintf(toBeSent->content, "%d", n); // add to tcp message

  } else if (udpMsg->dataType == 1) {
    memcpy(&(toBeSent->dataType), "SHORT_REAL", 10);
    toBeSent->dataType[10] = '\0';
    uint16_t n;
    memcpy(&n, &(udpMsg->content[0]), 2);
    n = ntohs(n);
    float x = n / 100.0;
    sprintf(toBeSent->content, "%.2f", x);

  } else if (udpMsg->dataType == 2) {
    memcpy(&(toBeSent->dataType), "FLOAT", 5);
    toBeSent->dataType[5] = '\0';
    uint32_t n;
    memcpy(&n, &(udpMsg->content[1]), 4);
    n = ntohl(n);
    int power = udpMsg->content[5];
    float x = ((float)n) / pow(10, power);
    if (udpMsg->content[0] == 1) {
      x *= -1;
    }
    sprintf(toBeSent->content, "%.4lf", x);

  } else {
    memcpy(&(toBeSent->dataType), "STRING", 6);
    toBeSent->dataType[6] = '\0';
    strcpy(toBeSent->content, udpMsg->content);
  }

  // copy the topic
  sprintf(toBeSent->topic, "%s", udpMsg->topic);
  toBeSent->topic[50] = '\0';
  return 1;
}

int sendQueueMessages(TClient* client) {
  // send all messages from queue
  while (client->messages->front != NULL) {
    char* message = client->messages->front->message;
    int sent = send(client->socket, message, sizeof(TtcpMsg), 0);
    DIE(sent < 0, "Eroare la trimitere");
    dequeue(client->messages);
  }
  return 1;
}

// deallocate memory
void freeResources(TClientList* list) {
  for (int i = list->numOfClients - 1; i >= 0; i--) {
    while (list->clients[i]->messages->front != NULL) {
      dequeue(list->clients[i]->messages);
    }
    free(list->clients[i]->messages);
    free(list->clients[i]);
  }
  free(list->clients);
  free(list);
}
