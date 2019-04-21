// Hinweise: 

// * Hier gibt es sicherlich optimierungsmöglichkeiten, z.B. sind 
//   alle drei send-Funktionen sehr ähnlich.

#include "chatLib.h"

 struct nodePeer* head;

// setup memory, filter msg type and output
void recvPeerMsg(int fd, struct nodePeer* head)
{
  //printf("recvfrom\n");
  struct chatPDU* pCurrMsg = malloc(sizeof(struct chatPDU));
  struct sockaddr_in* peerAddr = malloc(sizeof(struct sockaddr_in));
  unsigned int peerAddrlen;
  int recvBytes;
    
  peerAddrlen = sizeof(*peerAddr);
  recvBytes = recvfrom(fd, (struct chatPDU*)pCurrMsg, sizeof(*pCurrMsg), 0, (struct sockaddr*)peerAddr, &peerAddrlen);
  if(recvBytes < 0)
  {
    printf("recvfrom in recvMsg\n");
    perror("recvfrom:");
  }
  
  int strLen = strlen(pCurrMsg->name);
  if (pCurrMsg->name[strLen-1] == '\n')
  {
    pCurrMsg->name[strLen-1] = 0;
  }
  
  switch(pCurrMsg->typ)
  {
    case DISCOVER:  sendAnswer(fd, head, *peerAddr);
                    break;                
    case ENTRY:     printf("%s ist dem Chat beigetreten\n", pCurrMsg->name);
                    break;
    case MSG: 	    printf("%s: %s", pCurrMsg->name, pCurrMsg->msg);
                    break;
    case EXIT:      printf("%s hat den Chat verlassen\n", pCurrMsg->name);
		    removeNodeByAddr(head, *peerAddr);
                    break;
    default: 	    printf("Error: got message without typ\n");
  }
  free(pCurrMsg);
  free(peerAddr);
}

//####################################################################################

// setup memory and PDU, send
void sendMsg(int fd, char nickname[32], char* buf2, struct sockaddr_in peerAddr)
{
  //printf("send\n");
  int sendbytes;
  struct chatPDU* pCurrMsg = malloc(sizeof(struct chatPDU));
  
  strncpy(pCurrMsg->msg, buf2, 4096);
  strncpy(pCurrMsg->name, nickname, 32);
  pCurrMsg->typ = MSG;
  sendbytes = sendto(fd, (const struct chatPDU*)pCurrMsg, sizeof(*pCurrMsg), 0, (struct sockaddr*)&peerAddr, sizeof(peerAddr));
  if(sendbytes < 0)
  {
    perror("sendMsg sendto:");
  }
  free(pCurrMsg);
}

//####################################################################################

// setup memory and PDU, send
void sendEntry(int fd, char nickname[32], struct sockaddr_in peerAddr)
{
  //printf("entry\n");
  int sendbytes;
  struct chatPDU* pEntryMsg = (struct chatPDU*)malloc(sizeof(struct chatPDU));
    
  pEntryMsg->typ = ENTRY;
  strncpy(pEntryMsg->name, nickname, 32);
  
  sendbytes = sendto(fd, (const struct chatPDU*)pEntryMsg, sizeof(*pEntryMsg), 0, (struct sockaddr*)&peerAddr, sizeof(peerAddr));
  if(sendbytes < 0)
  {
    perror("sendEntry sendto:");
  }
  free(pEntryMsg);
}

//####################################################################################

// setup memory and PDU, send
void sendExit(int fd, char nickname[13], struct sockaddr_in peerAddr)
{
  //printf("exit\n");
  int sendbytes;
  struct chatPDU* pExitMsg = malloc(sizeof(struct chatPDU));
  pExitMsg->typ = EXIT;
  strncpy(pExitMsg->name, nickname, 32);
  sendbytes = sendto(fd, (const struct chatPDU*)pExitMsg, sizeof(*pExitMsg), 0, (struct sockaddr*)&peerAddr, sizeof(peerAddr));
  if(sendbytes < 0)
  {
    perror("sendExit sendto:");
  }
  free(pExitMsg);
}

//####################################################################################

//send discover msg, recv answer from friend, build list of all peers
void linkToChat(int fd, struct sockaddr_in* pFriendAddr, unsigned int localPort, struct nodePeer* head)
{
  //printf("start linktochat\n");
  int sendbytes;
  
  //build pdu
  struct chatPDU* pDiscoverMsg = (struct chatPDU*)malloc(sizeof(struct chatPDU));
  pDiscoverMsg->typ = DISCOVER;
  
  // send discover msg to friend
  sendbytes = sendto(fd, (const struct chatPDU*)pDiscoverMsg, sizeof(*pDiscoverMsg), 0, (struct sockaddr*)pFriendAddr, sizeof(*pFriendAddr));
  if(sendbytes < 0)
  {
    perror("sendDiscover sendto:");
  }
  
  // wait for answer
  unsigned int friendAddrLen; 
  int recvBytes;
  struct chatPDU* pAnswerMsg = (struct chatPDU*)malloc(sizeof(struct chatPDU));

  recvBytes = recvfrom(fd, (struct chatPDU*)pAnswerMsg, sizeof(*pAnswerMsg), 0, (struct sockaddr*)pFriendAddr, &friendAddrLen);
  if(recvBytes < 0)
  {
    printf("revcfrom in Linktochat\n");
    perror("recvfrom:");
  }
  
  struct sockaddr_in* allAddrs = (struct sockaddr_in*) malloc(sizeof(*pAnswerMsg->msg));  
  
  switch(pAnswerMsg->typ)
  {
    case ANSWER:    allAddrs = (struct sockaddr_in*)pAnswerMsg->msg;		   
		    buildList(head, allAddrs);
                    break;                
    default: 	    printf("Error: got message without typ answer\n");
                    break;
  }
  
  free(pDiscoverMsg);
  free(pAnswerMsg);
}

//####################################################################################

void sendAnswer(int fd, struct nodePeer* head, struct sockaddr_in newPeerAddr)
{ 
  //build PDU
  struct nodePeer currNode;
  currNode = *head;
  
  int length = getListLength(head);
  struct sockaddr_in allAddr[length];
  int i= 0;
  //copy all addresses in array
  for(i = 0; i<length;i++)
  {
    currNode = *currNode.nextPeer;
    //copy
    allAddr[i] = currNode.addr;
  }
  
  //Build answer PDU
  struct chatPDU* pAnswerMsg = malloc(sizeof(struct chatPDU));
  memcpy(&pAnswerMsg->msg,(char*)allAddr, (getListLength(head) * sizeof(struct sockaddr_in)) );
  pAnswerMsg->typ = ANSWER;
  
  //send PDU
  int sendbytes = sendto(fd, (const struct chatPDU*)pAnswerMsg, sizeof(*pAnswerMsg), 0, (struct sockaddr*)&newPeerAddr, sizeof(newPeerAddr));
  if(sendbytes < 0)
  {
    perror("sendAnswer sendto:");
  }
  
  //add new Peer to local list
  pushNode(head, newPeerAddr);
  
  free(pAnswerMsg);
}

//####################################################################################

//build new peerNode List with all the sockaddr_in
void buildList(struct nodePeer* head, struct sockaddr_in* allAddrs)
{  
  while(allAddrs->sin_family == AF_INET)
  {
    pushNode(head, *allAddrs);
    allAddrs++;
  }
}

//####################################################################################

int getListLength(struct nodePeer* head)
{
 int ret = 0;
 struct nodePeer* tmp;
 tmp = head;
 while(tmp->nextPeer != NULL)
 {
  ret++;
  tmp = tmp->nextPeer;
 }
 return ret; 
}

//######################################################################################

// Add node at the end of the list
void pushNode(struct nodePeer* head, struct sockaddr_in data)
{  
  struct nodePeer* current = head;
  while(current->nextPeer != NULL)
  {
    current = current->nextPeer;
  }
  
  current->nextPeer = malloc(sizeof(struct nodePeer));
  current->nextPeer->addr = data;
  current->nextPeer->nextPeer = NULL;
}

//######################################################################################

void removeNodeByAddr(struct nodePeer* head, struct sockaddr_in addrToRemove)
{
 struct nodePeer* tmp;
 struct nodePeer* current = head;
 
 //get IP to remove 
 char* ipToRemove = inet_ntoa(addrToRemove.sin_addr);
 printf("ip to remove %s\n", ipToRemove);
 char* currIP = (char*)malloc(sizeof(struct in_addr));
 
 printList(head);
 while(current->nextPeer != NULL)
 {
  //get current IP
   currIP = inet_ntoa(current->nextPeer->addr.sin_addr);
   printf("currIP is %s\n", currIP);
  //compare current IP with IP to remove
  if(current->nextPeer->addr.sin_addr.s_addr != addrToRemove.sin_addr.s_addr )
  {
    //if node to remove is the last in list
     if(current->nextPeer->nextPeer == NULL)
     {
//       printf("remove last node\n");
      current->nextPeer = NULL;
     }
     else
     {
    //  printf("remove ip, not last node\n");
    //remove this node from list
    current->nextPeer = current->nextPeer->nextPeer;
     }
    printf("removed ip %s\n", currIP);
  }
  current = current->nextPeer;
 }
}

//######################################################################################

void printList(struct nodePeer* head)
{
  int counter = 0;
  struct nodePeer* current = head;
  while(current->nextPeer != NULL)
  {
    current = current->nextPeer;
    counter++;
    printf("Ip no %d is %s\n", counter, inet_ntoa(current->addr.sin_addr) );
  }
}

  