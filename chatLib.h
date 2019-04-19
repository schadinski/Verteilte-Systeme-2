#ifndef MYHEADER_H
#define MYHEADER_H

#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

# define MAXPEERS 50

 extern struct nodePeer* head;

typedef enum{
  DISCOVER,
  ANSWER,
  ENTRY,
  EXIT,
  MSG
}Typ;

struct chatPDU {
  Typ typ;
  char name[32];
  char msg[4096];
};

struct nodePeer {
  struct sockaddr_in addr;
  struct nodePeer* nextPeer;
};

void recvPeerMsg(int, struct nodePeer*);
void sendMsg(int, char[32], char*, struct sockaddr_in);
void sendEntry(int, char[32], struct sockaddr_in);
void sendExit(int, char [32], struct sockaddr_in);
void linkToChat(int, struct sockaddr_in*, unsigned int, struct nodePeer*);
void sendAnswer(int, struct nodePeer* , struct sockaddr_in);

void buildList(struct nodePeer*,struct sockaddr_in*);
int getListLength(struct nodePeer*);
void printList(struct nodePeer*);
void pushNode(struct nodePeer*, struct sockaddr_in);

#endif /* MYHEADER_H */
