// Hinweise: 

// * Hier gibt es sicherlich optimierungsmöglichkeiten, z.B. sind 
//   alle drei send-Funktionen sehr ähnlich.

#include "chatLib.h"

// setup memory, filter msg type and output
void recvPeerMsg(int fd, struct nodePeer* head)
{
//  printf("recvfrom\n");
  struct chatPDU* pCurrMsg = malloc(sizeof(struct chatPDU));
  struct sockaddr_in* peerAddr = malloc(sizeof(struct sockaddr_in));
  unsigned int peerAddrlen;
  int recvBytes;
    
  peerAddrlen = sizeof(*peerAddr);
  recvBytes = recvfrom(fd, (struct chatPDU*)pCurrMsg, sizeof(*pCurrMsg), 0, (struct sockaddr*)peerAddr, &peerAddrlen);
  if(recvBytes < 0)
  {
    perror("recvfrom:");
  }
  
  //TODO: ist das nach Anpassung noch nötig/sinnvoll?
  // cut of \n from name 
  int strLen = strlen(pCurrMsg->name);
  if (pCurrMsg->name[strLen-1] == '\n')
  {
    pCurrMsg->name[strLen-1] = 0;
  }
  
  //printf("typ is %d, name is %s, msg is %s", pCurrMsg->typ, pCurrMsg->name, pCurrMsg->msg);
  switch(pCurrMsg->typ)
  {
    case DISCOVER:  sendAnswer(fd, head, *peerAddr);
                    break;                
    case ENTRY:     printf("%s ist dem Chat beigetreten\n", pCurrMsg->name);
		    // TODO: add new addr to allPeerAddrs
		    //size_t noOfPeers = getNoOfPeers(allPeerAddrs);
// 		    allPeerAddrs[noOfPeers+1] = *peerAddr;
                    break;
    case MSG: 	    printf("%s: %s", pCurrMsg->name, pCurrMsg->msg);
                    break;
    case EXIT:      printf("%s hat den Chat verlassen\n", pCurrMsg->name);
                    break;
    default: 	    printf("Error: got message without typ\n");
  }
  //printf("recv %d bytes\n", recvBytes);
  free(pCurrMsg);
}

// setup memory and PDU, send
void sendMsg(int fd, char nickname[32], char* buf2, struct sockaddr_in peerAddr)
{
  printf("send\n");
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
  printf("SENDMsg: send %d bytes\n", sendbytes);
  free(pCurrMsg);
}

// setup memory and PDU, send
void sendEntry(int fd, char nickname[32], struct sockaddr_in peerAddr)
{
  printf("entry\n");
  int sendbytes;
  struct chatPDU* pEntryMsg = (struct chatPDU*)malloc(sizeof(struct chatPDU));
    
  pEntryMsg->typ = ENTRY;
  strncpy(pEntryMsg->name, nickname, 32);
  
  sendbytes = sendto(fd, (const struct chatPDU*)pEntryMsg, sizeof(*pEntryMsg), 0, (struct sockaddr*)&peerAddr, sizeof(peerAddr));
  if(sendbytes < 0)
  {
    perror("sendEntry sendto:");
  }
  printf("Entry: send %d bytes\n", sendbytes);
  free(pEntryMsg);
}

// setup memory and PDU, send
void sendExit(int fd, char nickname[13], struct sockaddr_in peerAddr)
{
  printf("exit\n");
  int sendbytes;
  struct chatPDU* pExitMsg = malloc(sizeof(struct chatPDU));
  pExitMsg->typ = EXIT;
  strncpy(pExitMsg->name, nickname, 32);
  sendbytes = sendto(fd, (const struct chatPDU*)pExitMsg, sizeof(*pExitMsg), 0, (struct sockaddr*)&peerAddr, sizeof(peerAddr));
  if(sendbytes < 0)
  {
    perror("sendExit sendto:");
  }
  printf("Exit: send %d bytes\n", sendbytes);
  free(pExitMsg);
}

//send discover msg, recv answer from friend, build list of all peers
struct nodePeer* linkToChat(int fd, struct sockaddr_in* pFriendAddr, unsigned int localPort, struct nodePeer* head)
{
  printf("start linktochat\n");
  int sendbytes;
  
  //build pdu
  struct chatPDU* pDiscoverMsg = (struct chatPDU*)malloc(sizeof(struct chatPDU));
    
  pDiscoverMsg->typ = DISCOVER;
  printf("after build pdu\n");

  // send discover msg to friend
  sendbytes = sendto(fd, (const struct chatPDU*)pDiscoverMsg, sizeof(*pDiscoverMsg), 0, (struct sockaddr*)pFriendAddr, sizeof(*pFriendAddr));
  if(sendbytes < 0)
  {
    perror("sendDiscover sendto:");
  }
  printf("linktochat: send %d bytes\n", sendbytes);
  
  // wait for answer
  unsigned int friendAddrLen; 
  int recvBytes;
  struct chatPDU* pAnswerMsg = (struct chatPDU*)malloc(sizeof(struct chatPDU));

  recvBytes = recvfrom(fd, (struct chatPDU*)pAnswerMsg, sizeof(*pAnswerMsg), 0, (struct sockaddr*)pFriendAddr, &friendAddrLen);
  if(recvBytes < 0)
  {
    perror("recvfrom:");
  }
  printf("recvbytes in linktochat %d\n", recvBytes);
  
  struct sockaddr_in* allAddrs = (struct sockaddr_in*) malloc(sizeof(*pAnswerMsg->msg));  
  
  switch(pAnswerMsg->typ)
  {
    case ANSWER:    // pAnswerMsg->msg; enthält sockaddr_in als char[] bzw char*
		    allAddrs = (struct sockaddr_in*)pAnswerMsg->msg;
		    //printf("linktochat: port received is : %d\n", ntohs(allAddrs[0].sin_port) );
		    //printf("linktochat: addr recewived is: %s\n", inet_ntoa(allAddrs[0].sin_addr));
		   
		    return buildList(head, allAddrs);
                    break;                

    default: 	    printf("Error: got message without typ answer\n");
                    return NULL;
  }
  


  free(pDiscoverMsg);
  free(pAnswerMsg);
  free(allAddrs);
}

void sendAnswer(int fd, struct nodePeer* head, struct sockaddr_in newPeerAddr)
{
  printf("send answer\n");
  
  //printf("sendAnswer: send port %d\nsendAnswer: send addr %s\n",
//	 ntohs(allPeerAddrs[0].sin_port), inet_ntoa(allPeerAddrs[0].sin_addr));
  
  //build PDU
  
  //TODO copiere alle struct sockaddr_in in neue var(typ struct sockaddr_in[]) und versende diese
  struct nodePeer currNode;
  currNode.nextPeer = head->nextPeer;
  
  struct sockaddr_in allAddr[MAXPEERS];
  
  int i = 0;
  while(currNode.nextPeer != NULL)
  {
    //copy
    allAddr[i] = currNode.addr;
    currNode = *currNode.nextPeer;
  }
  
  struct chatPDU* pAnswerMsg = malloc(sizeof(struct chatPDU));
  memcpy(&pAnswerMsg->msg,allAddr, sizeof(allAddr) );
  pAnswerMsg->typ = ANSWER;
  
  //debug
  //struct sockaddr_in* addresses = (struct sockaddr_in*)malloc(sizeof(*allPeerAddrs));
  //addresses = (struct sockaddr_in*)pAnswerMsg->msg;
  //printf("sendAnswer no of peers: %d\n", getNoOfPeers(addresses) );
  
  //printf("sendAnswer after cast: send port %d\nsendAnswer after cast: send addr %s\n", 
//	 ntohs(addresses->sin_port), inet_ntoa(addresses->sin_addr));
  //debug end
  
  //send PDU
  int sendbytes = sendto(fd, (const struct chatPDU*)pAnswerMsg, sizeof(*pAnswerMsg), 0, (struct sockaddr*)&newPeerAddr, sizeof(newPeerAddr));
  if(sendbytes < 0)
  {
    perror("sendAnswer sendto:");
  }
  printf("sendAnswer: send %d bytes\n", sendbytes);
  
  free(pAnswerMsg);
}

//build new peerNode List with all the sockaddr_in
struct nodePeer* buildList(struct nodePeer* head, struct sockaddr_in* allAddrs)
{
  printf("start buildList\n"); 
  struct nodePeer tmpNode;
  struct nodePeer* tmp = (struct nodePeer*)malloc(sizeof(struct nodePeer));
  tmpNode.nextPeer = head->nextPeer;
  
  while(allAddrs->sin_family == AF_INET)
  {
    tmp = head->nextPeer;
    head->nextPeer = &tmpNode;
    
    tmpNode.addr = *allAddrs;
    tmpNode.nextPeer = tmp;
    allAddrs++;
  }
  printf("end buildList\n");
  return head;
}

size_t getNoOfPeers(struct sockaddr_in* allPeers)
{
  //printf("size all peers %d\n", sizeof(*allPeers));
  size_t noOfPeers = sizeof(*allPeers)/sizeof(struct sockaddr_in);
  return noOfPeers;
}
