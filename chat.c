#include "chatLib.h"

int main(int argc, char *argv[])
{ 
  //argv[1] = frindIP
  //argv[2] = myIP
  if ( argc < 3  )
  {
    printf("need 2 ips\n");
    return 1;
  }

  fd_set readset;
  char* nickname;
  size_t nameLen;
  size_t nameChars = 0;
  size_t len;
  unsigned int localPort =50123;
  int localFD;
  struct sockaddr_in myAddr;

  struct nodePeer* head = (struct nodePeer*)malloc(sizeof(struct nodePeer));
  head->nextPeer = NULL;
 
  nickname = malloc(32*sizeof(char));
  
  // build own Addresse
    memset(&myAddr, 0, sizeof(myAddr));
    
    localFD = socket( AF_INET, SOCK_DGRAM, 0);
    myAddr.sin_family = AF_INET;
    myAddr.sin_port = htons(localPort);
    myAddr.sin_addr.s_addr = inet_addr(argv[2]);
    struct sockaddr* pMyAddr = (struct sockaddr*)malloc(sizeof(struct sockaddr_in));
    pMyAddr = (struct sockaddr*)&myAddr;
    int rc = bind(localFD, pMyAddr, sizeof(*pMyAddr));
      if (rc < 0)
      {
        printf("Error: Bind local FD\n");
        perror("bind()");
      }
    struct nodePeer localNode;
    localNode.addr = myAddr;
    localNode.nextPeer = NULL;
    head->nextPeer = &localNode;
     
    //check if i am not the first peer
    if(strcmp(argv[1], argv[2])!=0)
    {
      //build friend addr
      struct sockaddr_in friendAddr;
      memset(&friendAddr, 0, sizeof(friendAddr));
      
      friendAddr.sin_family = AF_INET;
      friendAddr.sin_port = htons(localPort);
      friendAddr.sin_addr.s_addr = inet_addr(argv[1]);
      struct sockaddr_in* pFriendAddr = &friendAddr;

      //send discover
      linkToChat(localFD, pFriendAddr, localPort, head);
    }  

  //get user input
  while(nameChars == 0)
  {
    printf("Enter name:");
    nameChars = getline(&nickname, &nameLen, stdin);
  }

    struct nodePeer currPeer;
    currPeer = *head;
  do
  {
    currPeer = *currPeer.nextPeer;
    sendEntry(localFD, nickname, currPeer.addr);      
  }
  while(currPeer.nextPeer != NULL);
  

  FD_ZERO(&readset);
  
  // leave with !Exit
   while(1)
   {
     printList(head);
     currPeer.nextPeer = NULL;
     char* buf2 = malloc(2096* sizeof(char));
     FD_SET(localFD, &readset);
     FD_SET(0, &readset);
     
     select(localFD+1, &readset, 0, 0, 0);
     if(FD_ISSET(localFD,&readset))
     {    
	recvPeerMsg(localFD, head);
     }
     if(FD_ISSET(STDIN_FILENO,&readset))
     {
       getline(&buf2, &len, stdin);
       
       if(strstr(buf2, "!Exit"))
       {
	  currPeer = *head;
	  do
	  {
	      currPeer = *currPeer.nextPeer;
	      sendExit(localFD, nickname, currPeer.addr);

	  }
	  while(currPeer.nextPeer != NULL);
	  break;
       }
       else
       {
	  currPeer = *head;
	  do
	  {
	      currPeer = *currPeer.nextPeer;
	      sendMsg(localFD, nickname, buf2, currPeer.addr);
	  }
	  while(currPeer.nextPeer != NULL);	  
       }
     }
     free(buf2);
   }
 close(localPort);
 free(nickname);
 free(head);
 return 0;
}
