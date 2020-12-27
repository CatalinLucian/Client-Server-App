#include "map.h"
#include "utils.h"
#include <stdlib.h>


TtopicToClients* createMap() {
  TtopicToClients* map = (TtopicToClients*)calloc(1, sizeof(TtopicToClients));
  DIE(map == NULL, "calloc error");
  map->size = 0;
  map->topicsAndClients = (TtopicEntry**)calloc(MAX_TOPICS, sizeof(TtopicEntry*));
  DIE(map->topicsAndClients == NULL, "calloc error");
  return map;
}

int searchTopic(TtopicToClients* map, char* topic) {
  for (int i = 0; i < map->size; i++) {
    if (strcmp(map->topicsAndClients[i]->topic, topic) == 0) {
      return i;
    }
  }
  return -1;
}

void addTopicEntry(TtopicToClients* map, char* topic) {
  // there can be at max MAX_TOPICS topics in the map
  if (map->size == MAX_TOPICS) {
    printf("Too many topics\n");
    return;
  }

  // if the topic is there already
  if (searchTopic(map, topic) != -1) {
    return;
  }

  TtopicEntry* entry = (TtopicEntry*)calloc(1, sizeof(TtopicEntry));
  entry->clientList = initClientList();
  strcpy(entry->topic, topic);
  int n = map->size;
  map->topicsAndClients[n] = entry;
  map->size++;
}

int subscribe(TtopicToClients* map, TClient* client, char* topic, int sf, TClientList* list) {

  // check if the client is connected to the server
  int total = list->numOfClients;
  int p = findClient(client->id, list->clients, total);
  if (p == -1) {
    printf("Client is not connected to the server\n");
    return 0;
  }

  // check if the topic exists
  int pos = searchTopic(map, topic);
  if (pos == -1) {
    printf("Nonexistent topic\n");
    return 0;
  }
  int numOfClients = map->topicsAndClients[pos]->clientList->numOfClients;

  if (numOfClients == MAX_CLIENTS) {
    printf("Can't subscribe to this topic\n");
    return 0;
  }

  if (findClient(client->id, map->topicsAndClients[pos]->clientList->clients, numOfClients) != -1) {
    printf("Has already subscribed to this topic\n");
    return 0;
  }

  // update client's subscriptions
  int nr_subscriptions = list->clients[p]->nr_subscriptions;
  strcpy(list->clients[p]->subscriptions[nr_subscriptions].topic, topic);
  list->clients[p]->subscriptions[nr_subscriptions].sf = sf;
  list->clients[p]->nr_subscriptions++;

  // add client to topic->subscribed_clients map
  map->topicsAndClients[pos]->clientList->clients[numOfClients] = client;
  map->topicsAndClients[pos]->clientList->numOfClients++;

  return 1;
}

int unsubscribe(TtopicToClients* map, TClient* client, char* topic, TClientList* list) {
  // check if client is connected
  int total = list->numOfClients;
  int p = findClient(client->id, list->clients, total);
  if (p == -1) {
    printf("Client is not connected to the server\n");
    return 0;
  }

  // check if topic exists
  int pos = searchTopic(map, topic);
  if (pos == -1) {
    printf("Nonexistent topic\n");
    return 0;
  }

  int numOfClients = map->topicsAndClients[pos]->clientList->numOfClients;
  int pos2 = findClient(client->id, map->topicsAndClients[pos]->clientList->clients, numOfClients);

  if (pos2 == -1) {
    printf("You are not subscribed to this topic\n");
    return 0;
  }

  // update client's subscriptions
  int idx = -1;
  int nr_subscriptions = list->clients[p]->nr_subscriptions;
  for (int i = 0; i < nr_subscriptions; i++) {
    if (strcmp(list->clients[p]->subscriptions[i].topic, topic) == 0) {
      idx = i;
    }
  }
  for (int i = idx; i < nr_subscriptions; i++) {
    list->clients[p]->subscriptions[i] = list->clients[p]->subscriptions[i + 1];
  }
  list->clients[p]->nr_subscriptions--;

  // delete client from topic->subscribed_clients map
  for (int i = pos2; i < numOfClients; i++) {
    map->topicsAndClients[pos]->clientList->clients[i] =
                    map->topicsAndClients[pos]->clientList->clients[i + 1];
  }
  map->topicsAndClients[pos]->clientList->numOfClients--;
  return 1;
}
