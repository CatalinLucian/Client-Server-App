#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include "helpers.h"

void error(char *msg) {
	printf("%s\n", msg);
	exit(0);
}

int main(int argc, char const *argv[]) {
  int sockfd, ret;
  struct sockaddr_in serv_addr;
  char buffer[BUFLEN];

  // check if no of args is correct
  if (argc < 4) {
    error("Error: Too few arguments -> Usage: ./subscriber <ID_Client> <IP_Server> <Port_Server>");
  }

  // check for data to be introduced corectly
  if (strlen(argv[1]) > 10) {
    error("Error: ID is too long");
  }

	// open tcp socket
  sockfd = socket(PF_INET, SOCK_STREAM, 0);
  DIE(sockfd < 0, "socket");

  // fill server's address and port
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(atoi(argv[3]));
  ret = inet_aton(argv[2], &serv_addr.sin_addr);
  DIE(ret == 0, "inet_aton");


  ret = connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
  DIE(ret < 0, "connect");

  fd_set read_fds;
  fd_set tmp_fds;
  int fdmax;

  FD_ZERO(&read_fds);
  FD_ZERO(&tmp_fds);

  FD_SET(sockfd, &read_fds);
  // 0 is the descriptor for stdin
	FD_SET(0, &read_fds);
  fdmax = sockfd;
  int flag = 1;

	// deactivate Nagle algorithm for message truncation
	setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, (char *)&flag, sizeof(int));

	// send id to server
	int sent = send(sockfd, argv[1], strlen(argv[1]) + 1, 0);
	DIE(sent < 0, "sending");


  while (1) {
    tmp_fds = read_fds;
    ret = select(fdmax + 1, &tmp_fds, NULL, NULL, NULL);
    for (int i = 0; i <= fdmax; i++) {
      if (FD_ISSET(i, &tmp_fds)) {
				// client receives command from stdin
        if (i == 0) {
          memset(buffer, 0, BUFLEN);
          fgets(buffer, BUFLEN - 1, stdin);

					// client wants to disconnect from the server
          if (strncmp(buffer, "exit", 4) == 0) {
            goto exit;
          }

          // copy for checking the sent message
          char copy[BUFLEN];
          memset(copy, 0, BUFLEN);
          memcpy(&copy, buffer, BUFLEN - 1);

					// message checking is being done by the client to ease the server's
					// tasks which has to handle multiple clients at a time
          // the checking is used as a defensive programming tool
          char* token = strtok(copy, " "); // token = subscribe sau unsubscribe
          if (token == NULL) {
            error("Incorrect message");
          }
          if (strncmp(token, "unsubscribe", 11) == 0) {
            token = strtok(NULL, " "); // token = topic
            if (token == NULL) {
              error("No topic found");
            }
						printf("unsubscribed %s\n", token);
          } else if (strncmp(token, "subscribe", 9) == 0) {
            token = strtok(NULL, " "); // token = topic
						char topic[50];
						strcpy(topic, token);
            if (token == NULL) {
              error("No topic found");
            }
            token = strtok(NULL, " "); // token = sf
            if (token == NULL) {
              error("SF value should be specified");
            }
					  // in case sf is not zero or 1
            if (!((atoi(token) == 0) || atoi(token) == 1)) {
              error("SF should be either 0 or 1");
            }
						printf("subscribed %s\n", topic);
          } else {
            error("Please retry and enter accepted commands");
          }

          int toBeSent = send(sockfd, buffer, strlen(buffer) + 1, 0);
          DIE(toBeSent < 0, "Eroare la trimiterea mesajului");
        }
				// message from server was received
        if (i == sockfd) {
          memset(buffer, 0, BUFLEN);
          int toBeRecv = recv(i, buffer, sizeof(TtcpMsg), 0);
          DIE(toBeRecv < 0, "Eroare la primirea mesajului");
					if (toBeRecv == 0) {
						break;
					}
					// server sent the message
					// print necessary fields from teh message
					TtcpMsg* received = (TtcpMsg*)buffer;
          printf("%s:%hu - %s - %s - %s\n", received->ip, received->port,
					       received->topic, received->dataType, received->content);
        }
      }// isset
    }// for
  }

	exit:

  close(sockfd);
  return 0;
}
