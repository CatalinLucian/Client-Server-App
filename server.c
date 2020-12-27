#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <math.h>
#include "map.h"
#include "utils.h"

int main(int argc, char const *argv[]) {
  int sockfd, newsockfd, portno;
	char buffer[BUFLEN];
	struct sockaddr_in serv_addr, udp_addr, tcpCli;
  struct sockaddr_in from_udp = { 0 };
  socklen_t len_udp = sizeof(from_udp);
	int ret;
	socklen_t clilen = sizeof(struct sockaddr_in);

	fd_set read_fds;
	fd_set tmp_fds;
	int fdmax;

  DIE(argc < 2, "Specify wanted port please");


  FD_ZERO(&read_fds);
  FD_ZERO(&tmp_fds);

  // open tcp socket
  sockfd = socket(PF_INET, SOCK_STREAM, 0);
  DIE(sockfd < 0, "tcp socket error");

  int flag_delay = 1;

  // open udp socket
  int sockUdp = socket(PF_INET, SOCK_DGRAM, 0);
  DIE(sockUdp < 0, "udp socket error");

  portno = atoi(argv[1]);
  DIE(portno <= 1024, "port not available");

  memset((char *) &serv_addr, 0, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(portno);
  serv_addr.sin_addr.s_addr = INADDR_ANY;

  ret = bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(struct sockaddr));
  DIE(ret < 0, "bind");

  udp_addr.sin_family = AF_INET;
  udp_addr.sin_port = htons(portno);
  udp_addr.sin_addr.s_addr = INADDR_ANY;


  ret = bind(sockUdp, (struct sockaddr *) &udp_addr, sizeof(struct sockaddr));
  DIE(ret < 0, "bind");

  // there can be at max MAX_CLIENTS connected at the same time
  ret = listen(sockfd, MAX_CLIENTS);
  DIE(ret < 0, "listen");

  // add tcp and udp sockets in reading descriptors
  FD_SET(sockfd, &read_fds);
  FD_SET(sockUdp, &read_fds);
  FD_SET(0, &read_fds);

  if (sockfd > sockUdp) {
    fdmax = sockfd;
  } else fdmax = sockUdp;

  // create mapping between topics and subscribers of that topic
  TtopicToClients* map = createMap();

  // list which keeps track of connected/disconnected clients since the server
  // started
  TClientList* list = initClientList();

  // these topics are static
  // additional topics will be added after the UDP client send the message to
  // the server
  // the subscriber can't subscribe to a topic (except for those statically linked)
  // unless the UDP client snet the first message on that topic
  addTopicEntry(map, "a_non_negative_int");
  addTopicEntry(map, "a_negative_int");
  addTopicEntry(map, "a_larger_value");
  addTopicEntry(map, "a_large_negative_value");
  addTopicEntry(map, "that_is_small_short_real");
  addTopicEntry(map, "that_is_big_short_real");
  addTopicEntry(map, "that_is_integer_short_real");
  addTopicEntry(map, "float_seventeen");
  addTopicEntry(map, "float_minus_seventeen");
  addTopicEntry(map, "a_strange_float");
  addTopicEntry(map, "a_negative_strange_float");
  addTopicEntry(map, "a_subunitary_float");
  addTopicEntry(map, "a_negative_subunitary_float");
  addTopicEntry(map, "ana_string_announce");
  addTopicEntry(map, "huge_string");
  addTopicEntry(map, "topic_a");
  addTopicEntry(map, "topic_b");
  addTopicEntry(map, "topic_c");

  while(1) {
    tmp_fds = read_fds;
    ret = select(fdmax + 1, &tmp_fds, NULL, NULL, NULL);
    if (ret < 0) {
      printf("select error\n");
      goto exit;
    }
    for (int i = 0; i <= fdmax; i++) {
      if (FD_ISSET(i, &tmp_fds)) {
        // server receives command from stdin
        if (i == 0) {
          memset(buffer, 0, BUFLEN);
          fgets(buffer, BUFLEN - 1, stdin);

          if (strncmp(buffer, "exit", 4) == 0) {
            // end all connections and free all memory
            goto exit;
          } else {
            printf("-exit- is the only accepted command");
          }
        } else if (i == sockUdp) { // server received UDP message
          memset(buffer, 0, BUFLEN);
          int received = recvfrom(sockUdp, buffer, BUFLEN, 0,
                            (struct sockaddr*)&from_udp, &len_udp);
          if (received < 0) {
            printf("recv error\n");
            goto exit;
          }

          // UDP message is of type udpMsg and it is converted from char*
          TudpMsg* udpMsg = (TudpMsg*)buffer;

          // construction of the message for the subscriber follows

          TtcpMsg toBeSent = {};
          // decode UDP message
          convertMessage(udpMsg, &toBeSent);
          toBeSent.port = ntohs(from_udp.sin_port);
          strcpy(toBeSent.ip, inet_ntoa(from_udp.sin_addr));

          // add the topic to the topic list
          addTopicEntry(map, udpMsg->topic);
          int topicPos = searchTopic(map, udpMsg->topic);
          if (topicPos == -1) {
            continue;
          }
          int numOfClients =
                      map->topicsAndClients[topicPos]->clientList->numOfClients;
          for (int j = 0; j < numOfClients; j++) {
            TClient* current = map->topicsAndClients[topicPos]->clientList->clients[j];
            // send decoded message to every client subscribed to that topic
            if (current->is_connected == 1) {
              int sock = current->socket;
              int sent = send(sock, (char*)&toBeSent, sizeof(TtcpMsg), 0);
              if (sent < 0) {
                printf("Eroare la trimitere\n");
                goto exit;
              }
            } else {
              // enqueue messages from topic for disconnected clients with
              // 'store and forward' activated
              if (checkIfSF(udpMsg->topic, current) == 1) {
                enqueue(current->messages, (char*)&toBeSent);
              }
            }
          }



        } else if (i == sockfd) { // tcp connection request
          newsockfd = accept(i, (struct sockaddr*)&tcpCli, &clilen);
          if (newsockfd < 0) {
            printf("Conexiunea nu se poate realiza");
            goto exit;
          }

          // deactivate Nagle algorithm for message truncation
          setsockopt(newsockfd, IPPROTO_TCP, TCP_NODELAY,
                   (char *)&flag_delay, sizeof(int));

          FD_SET(newsockfd, &read_fds);
          if (newsockfd > fdmax) {
            fdmax = newsockfd;
          }

          memset(buffer, 0, BUFLEN - 1);
          // receive subscriber id
          int received = recv(newsockfd, buffer, BUFLEN - 1, 0);
          if (received < 0) {
            printf("recv error");
            goto exit;
          }
          // add new client
          TClient* newClient = createClient(newsockfd, buffer);
          addClient(list, newClient);

          printf("New client %s connected from %s:%d\n", buffer,
                           inet_ntoa(tcpCli.sin_addr), ntohs(tcpCli.sin_port));

        } else { // command from subscriber
          memset(buffer, 0, BUFLEN - 1);
          int received = recv(i, buffer, BUFLEN - 1, 0);
          if (received < 0) {
            printf("recv error");
            goto exit;
          }

          if (received == 0) { // client has disconnected
            int pos = findClientBySocket(i, list->clients, list->numOfClients);
            printf("Client %s disconnected\n", list->clients[pos]->id);

            FD_CLR(list->clients[pos]->socket, &read_fds);
            // update max descriptor
            for (int k = fdmax; k > 2; k--) {
              if (FD_ISSET(k, &read_fds)) {
                fdmax = k;
                break;
              }
            }
            close(list->clients[pos]->socket);
            list->clients[pos]->is_connected = 0;
            list->clients[pos]->socket = -1; // socket becomes inactive
          } else {
            // client subscribes to a topic
            if (strncmp(buffer, "subscribe", 9) == 0) {
              int pos = findClientBySocket(i, list->clients, list->numOfClients);
              char* topic = strtok(buffer, " ");
              // find that topic
              topic = strtok(NULL, " ");
              if (topic == NULL) {
                printf("Eroare");
                goto exit;
              }
              // find if 'store and forward is activated'
              char* sf = strtok(NULL, " ");
              if (sf == NULL) {
                printf("Eroare");
                goto exit;
              }
              subscribe(map, list->clients[pos], topic, atoi(sf), list);
            } else if (strncmp(buffer, "unsubscribe", 11) == 0) {
              int pos = findClientBySocket(i, list->clients, list->numOfClients);
              char* topic = strtok(buffer, " \n");
              topic = strtok(NULL, " \n");
              if (topic == NULL) {
                printf("Eroare la primirea mesajului");
                goto exit;
              }
              unsubscribe(map, list->clients[pos], topic, list);
            }
          }
        }
      }
    }
  }

  exit:
    // close all sockets
    for (int i = 1; i <= fdmax; i++) {
      if (FD_ISSET(i, &read_fds)) {
        close(i);
      }
    }
    // deallocate all memory
    freeResources(list);
    for (int i = 0; i < map->size; i++) {
      free(map->topicsAndClients[i]->clientList->clients);
      free(map->topicsAndClients[i]->clientList);
      free(map->topicsAndClients[i]);
    }
    free(map->topicsAndClients);
    free(map);
  return 0;
}
