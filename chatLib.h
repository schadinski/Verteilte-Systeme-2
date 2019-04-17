#ifndef MYHEADER_H
#define MYHEADER_H

#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

/*
static unsigned short ports[10] = { 1050,
				    1100,
				    1150,
				    1200,
				    1250,
				    1300,
				    1350,
				    1400,
				    1450,
				    1500 };
				    */
# define MAXPEERS 50

//extern struct sockaddr_in* allPeerAddrs;

//192.168.178.21 = suseVM, home
//192.168.178.62 = suseVM HP Laptop
//10.0.2.15 = suseVM, eth0
//192.168.178.81 = Ubuntu Fujitsu Laptop
/*static char* ips[MAXPEERS] = { "192.168.178.21",
  			"192.168.178.62",
  //			"10.0.2.15",
			"192.168.178.81" };
*/

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

void recvPeerMsg(int, struct sockaddr_in*);
void sendMsg(int, char[32], char*, struct sockaddr_in);
void sendEntry(int, char[32], struct sockaddr_in);
void sendExit(int, char [32], struct sockaddr_in);
struct sockaddr_in* linkToChat(int, struct sockaddr_in*, unsigned int);
void sendAnswer(int, struct sockaddr_in*, struct sockaddr_in);

size_t getNoOfPeers(struct sockaddr_in*);

#endif /* MYHEADER_H */
