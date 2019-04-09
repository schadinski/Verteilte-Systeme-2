// Hinweise: 

// * Hier gibt es sicherlich optimierungsmöglichkeiten, z.B. sind 
//   alle drei send-Funktionen sehr ähnlich.

// * Debugging-Statements verbleiben im Code, da diese für Teil 2
//   nützlich sein werden.

#include "chatLib.h"

// setup memory, filter msg type and output
void recvPeerMsg(int fd, struct sockaddr_in* allPeerAddrs)
{
//  printf("recvfrom\n");
  struct chatPDU* pCurrMsg = malloc(sizeof(struct chatPDU));
  struct sockaddr* peerAddr = malloc(sizeof(struct sockaddr));
  unsigned int peerAddrlen;
  int recvBytes;
    
  peerAddrlen = sizeof(*peerAddr);
  recvBytes = recvfrom(fd, (struct chatPDU*)pCurrMsg, sizeof(*pCurrMsg), 0, peerAddr, &peerAddrlen);
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
    case DISCOVER:  sendAnswer(allPeerAddrs);
                    break;                
    case ENTRY:     printf("%s ist dem Chat beigetreten\n", pCurrMsg->name);
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
//  printf("send\n");
  int sendbytes;
  struct chatPDU* pCurrMsg = malloc(sizeof(struct chatPDU));
  
  strncpy(pCurrMsg->msg, buf2, 2096);
  strncpy(pCurrMsg->name, nickname, 32);
  pCurrMsg->typ = MSG;
  sendbytes = sendto(fd, (const struct chatPDU*)pCurrMsg, sizeof(*pCurrMsg), 0, (struct sockaddr*)&peerAddr, sizeof(peerAddr));
  if(sendbytes < 0)
  {
    perror("sendMsg sendto:");
  }
//  printf("Msg: send %d bytes\n", sendbytes);
  free(pCurrMsg);
}

// setup memory and PDU, send
void sendEntry(int fd, char nickname[32], struct sockaddr_in peerAddr)
{
//  printf("entry\n");
  int sendbytes;
  struct chatPDU* pEntryMsg = (struct chatPDU*)malloc(sizeof(struct chatPDU));
    
  pEntryMsg->typ = ENTRY;
  strncpy(pEntryMsg->name, nickname, 32);
  
  sendbytes = sendto(fd, (const struct chatPDU*)pEntryMsg, sizeof(*pEntryMsg), 0, (struct sockaddr*)&peerAddr, sizeof(peerAddr));
  if(sendbytes < 0)
  {
    perror("sendEntry sendto:");
  }
//  printf("Entry: send %d bytes\n", sendbytes);
  free(pEntryMsg);
}

// setup memory and PDU, send
void sendExit(int fd, char nickname[13], struct sockaddr_in peerAddr)
{
//  printf("exit\n");
  int sendbytes;
  struct chatPDU* pExitMsg = malloc(sizeof(struct chatPDU));
  pExitMsg->typ = EXIT;
  strncpy(pExitMsg->name, nickname, 32);
  sendbytes = sendto(fd, (const struct chatPDU*)pExitMsg, sizeof(*pExitMsg), 0, (struct sockaddr*)&peerAddr, sizeof(peerAddr));
  if(sendbytes < 0)
  {
    perror("sendExit sendto:");
  }
//  printf("Exit: send %d bytes\n", sendbytes);
  free(pExitMsg);
}

//send discover msg, recv answer from friend, build list of all peers
struct sockaddr_in* linkToChat(int fd, in_addr_t friendIP, unsigned int localPort)
{
  //  printf("discover\n");
  int sendbytes;
  
  //build pdu
  struct chatPDU* pDiscoverMsg = (struct chatPDU*)malloc(sizeof(struct chatPDU));
    
  pDiscoverMsg->typ = DISCOVER;
  
  //build sockaddr
  struct sockaddr_in* friendAddr = (struct sockaddr_in*)malloc(sizeof(struct sockaddr_in));
  memset(&friendAddr, 0, sizeof(friendAddr));
    
  friendAddr->sin_family = AF_INET;
  friendAddr->sin_port = htons(localPort);
  friendAddr->sin_addr.s_addr = friendIP;
  
  // send discover msg
  sendbytes = sendto(fd, (const struct chatPDU*)pDiscoverMsg, sizeof(*pDiscoverMsg), 0, (struct sockaddr*)&friendAddr, sizeof(friendAddr));
  if(sendbytes < 0)
  {
    perror("sendDiscover sendto:");
  }
  //  printf("Discover: send %d bytes\n", sendbytes);
  
  
  
  struct sockaddr* newFriendAddr = (struct sockaddr*)malloc(sizeof(struct sockaddr));
  newFriendAddr = (struct sockaddr*)friendAddr;
  // wait for answer
  unsigned int friendAddrLen = 0; 
  int recvBytes;
    
  friendAddrLen = sizeof((struct sockaddr)*newFriendAddr);
  struct chatPDU* pAnswerMsg = (struct chatPDU*)malloc(sizeof(struct chatPDU));

  recvBytes = recvfrom(fd, (struct chatPDU*)pAnswerMsg, sizeof(*pAnswerMsg), 0, newFriendAddr, &friendAddrLen);
  if(recvBytes < 0)
  {
    perror("recvfrom:");
  }
  
  struct sockaddr_in* allAddrs = (struct sockaddr_in*) malloc(sizeof(*pAnswerMsg->msg));
  switch(pAnswerMsg->typ)
  {
    case ANSWER:      // pAnswerMsg->msg; enthält sockaddr_in als char[] bzw char*
                      //ggf memcpy  für rein
                      allAddrs = (struct sockaddr_in*)pAnswerMsg->msg;
                      return allAddrs;
                    break;                
    default: 	    printf("Error: got message without typ answer\n");
                    return NULL;
  }
  


  free(pDiscoverMsg);
  //free(&friendAddr);
}

void sendAnswer(struct sockaddr_in* allPeerAddrs)
{
      //ggf memcpy  für allAddrs in msg
    printf("send answer\n");
}
