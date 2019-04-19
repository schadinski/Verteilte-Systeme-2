#include "chatLib.h"

int main(int argc, char *argv[])
{
  //printf("first line \n");
    
  //argv[1] = frindIP
  //argv[2] = myIP
  // if first peer frienIp == myIp
  if ( argc < 3  )
  {
    printf("need 2 ips\n");
    return 1;
  }

  int events;
  fd_set readset;
  char* nickname;
  size_t nameLen;
  size_t nameChars = 0;
  size_t len;
  unsigned int localPort =50123;
  int i;
  int localFD;
  struct sockaddr_in myAddr;

  struct nodePeer* head = (struct nodePeer*)malloc(sizeof(struct nodePeer));
  head->nextPeer = NULL;

  
  //printf("after var init\n");
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
    // own node is first in list
    printf("length of list except 0, is %d\n", getListLength(head) );
    struct nodePeer localNode;
    localNode.addr = myAddr;
    localNode.nextPeer = NULL;
    head->nextPeer = &localNode;
    printf("length of list except 1, is %d\n", getListLength(head) );
    
    //debug
//     struct nodePeer debug;
//     debug = *head->nextPeer;
//     printf(" first addr should be my own: %s\n", inet_ntoa(debug.addr.sin_addr));
    //debug end
     
    //check if i am the first peer
    if (strcmp(argv[1], argv[2])==0)
    {
      //printf("I am first\n");
    }
    else
    {
      //printf("not the first\n");
      
      //build friend addr
      struct sockaddr_in friendAddr;
      memset(&friendAddr, 0, sizeof(friendAddr));
      
      friendAddr.sin_family = AF_INET;
      friendAddr.sin_port = htons(localPort);
      friendAddr.sin_addr.s_addr = inet_addr(argv[1]);
      struct sockaddr_in* pFriendAddr = (struct sockaddr_in*)malloc(sizeof(struct sockaddr_in));
      pFriendAddr = &friendAddr;

//          struct nodePeer tmp;// = (struct nodePeer*)malloc(sizeof(struct nodePeer));
//  	tmp = *head->nextPeer;
//  	printf(" tmp addr is my own: %s\n", inet_ntoa(tmp.addr.sin_addr));
      	//printList(head);
	//sleep(5);
	//tmp->nextPeer = NULL;
      printf("before linktochat\n");
      //send discover
      struct nodePeer* tmp = NULL;
	tmp = linkToChat(localFD, pFriendAddr, localPort, head);
	if(tmp == NULL)
	{
	  printf("Error: Get no peer addresses\n");
	}
	printf(" tmp addr after link to chat: %s\n", inet_ntoa(tmp->addr.sin_addr));
	localNode.nextPeer = tmp;// 	printf(" tmp addr after link to chat: %s\n", inet_ntoa(tmp.addr.sin_addr));

//  	printf(" first addr after link to chat: %s\n", inet_ntoa(tmp.addr.sin_addr));
//	head->nextPeer = &tmp;
	//printList(head);
	//tmp ist Kopie von lacalNode
// 	struct nodePeer debugging;
// 	debugging = *tmp.nextPeer;
// 	printf(" tmp next addr after link to chat: %s\n", inet_ntoa(debugging.addr.sin_addr));
// 	debugging = *debugging.nextPeer;
// 	printf(" second addr after link to chat: %s\n", inet_ntoa(debugging.addr.sin_addr));
// 	

	printf("length of list after link to chat except 2, is %d\n", getListLength(head) );
	

	sleep(5);	

      printf("after linktochat\n");
    }  
      
    

  
    // setup all peer addresses
/*  for(i = 0; i < MAXPEERS; i++)
  {
    peerFDs[i] = socket( AF_INET, SOCK_DGRAM, 0);
    if(peerFDs[i] < 0)
    {
      printf("Error: socket peerFDs[%d]\n", i);
      perror("socket peerFDs:");
    }
    
        [i] = *(struct sockaddr_in*)malloc(sizeof(struct sockaddr_in));
    memset(&allPeerAddrs[i], 0, sizeof(allPeerAddrs[i]));
    
    allPeerAddrs[i].sin_family = AF_INET;
    allPeerAddrs[i].sin_port = htons(localPort);
    allPeerAddrs[i].sin_addr.s_addr = inet_addr(allIPs[i]);
    // filter myIP
    if(strcmp(allIPs[i],myIP) == 0)
    {
        
    }
  }
*/  

  //get user input
  while(nameChars == 0)
  {
    printf("Enter name:");
    nameChars = getline(&nickname, &nameLen, stdin);
  }
  
  //printf("sizeof allPeers is %d\n", sizeof(*allPeerAddrs));
  //size_t noOfPeers = getNoOfPeers(allPeerAddrs);
  //printf("noOfPeers is %d\n", noOfPeers);
  
    struct nodePeer currPeer;
    currPeer = *head;//->nextPeer;
  do
  {
    currPeer = *currPeer.nextPeer;
      //printf("addr %s\n", inet_ntoa(allPeerAddrs[i].sin_addr));
      printf("while send entry: addr is: %s\n", inet_ntoa(currPeer.addr.sin_addr));
      sendEntry(localFD, nickname, currPeer.addr);
      //sleep(5);
      
  }
  while(currPeer.nextPeer != NULL);
  
  currPeer.nextPeer = NULL;

/*  for(i=0; i<MAXPEERS; i++)
  {
  sendEntry(localFD, nickname, allPeerAddrs[i]);
  //printf("after sendEntry in main\n");
  }*/
  FD_ZERO(&readset);
  
  // leave with !Exit
   while(1)
   {
     //noOfPeers = getNoOfPeers(allPeerAddrs);
     char* buf2 = malloc(2096* sizeof(char));
     FD_SET(localFD, &readset);
     FD_SET(0, &readset);
     
     events = select(localFD+1, &readset, 0, 0, 0);
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
	    currPeer.nextPeer = NULL;
	 /*for(i=0;i<(MAXPEERS);i++)
	 {
	 sendExit(localFD, nickname, allPeerAddrs[i]);
	 }*/
	 break;
       }
       else
       {
	 printf("else, send msg\n");
	 currPeer = *head;
	 printf("send msg, after setup currPeer\n");
	  do
	  {
	    	      currPeer = *currPeer.nextPeer;
	      printf("while send msg: addr is: %s\n", inet_ntoa(currPeer.addr.sin_addr));
	      sendMsg(localFD, nickname, buf2, currPeer.addr);

	  }
	  while(currPeer.nextPeer != NULL);
	    currPeer.nextPeer = NULL;
	  /*for(i=0;i<(MAXPEERS);i++)
	  { 
	    sendMsg(localFD, nickname, buf2, allPeerAddrs[i]);
	  }*/
       }
     }
     free(buf2);
   }
 /*for(i=0; i<(MAXPEERS);i++)
 {
   close(peerFDs[i]);
 }*/
 close(localPort);
 free(nickname);
 //free(friendIP);
 return 0;
}
