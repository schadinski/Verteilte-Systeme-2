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
  //int* peerFDs;
  //char myIP[20];
  int localFD;
 // char* allIPs;
  //in_addr_t friendIP;
  int activePeers = 0;
  struct sockaddr_in myAddr;


  
  //printf("after var init\n");
  nickname = malloc(32*sizeof(char));
  struct sockaddr_in* allPeerAddrs = (struct sockaddr_in*)malloc(MAXPEERS*sizeof(struct sockaddr_in));
  
  // generiere meine Addresse, inkl fd und bind
  // send discover
  // lausche auf antwort:
  //        recvfrom mit timeout-> dann return 1
  //        recvfrom erfolgreich: - hole alle addressen raus
  //                              - fertige Liste mit allen addressen + meiner Adresse
//                                - sendEntry() an alle
  
  // build own Addresse
    memset(&myAddr, 0, sizeof(myAddr));
    
    localFD = socket( AF_INET, SOCK_DGRAM, 0);
    //printf("after socket\n");
    myAddr.sin_family = AF_INET;
    //printf("after sin_family\n");
    myAddr.sin_port = htons(localPort);
    //printf("after set port\n");
    myAddr.sin_addr.s_addr = inet_addr(argv[2]);
    //printf("after setup myAddr\n");
    struct sockaddr* pMyAddr = (struct sockaddr*)malloc(sizeof(struct sockaddr_in));
    pMyAddr = (struct sockaddr*)&myAddr;
    int rc = bind(localFD, pMyAddr, sizeof(*pMyAddr));
      if (rc < 0)
      {
        printf("Error: Bind local FD\n");
        perror("bind()");
      }
    
     
    //check if i am the first peer
    if (strcmp(argv[1], argv[2])==0)
    {
      printf("I am first\n");
      //set my add in allPeerAddrs[0]
      // no discover send, wait for get discover
      allPeerAddrs[0] = myAddr;
    }
    else
    {
       // set my add to allPeerAddrs[x] 

      printf("not the first\n");
      //build friend addr
      struct sockaddr_in friendAddr;
      memset(&friendAddr, 0, sizeof(friendAddr));
      
      //int friendFD = socket( AF_INET, SOCK_DGRAM, 0);
      //printf("after socket\n");
      friendAddr.sin_family = AF_INET;
      //printf("after sin_family\n");
      friendAddr.sin_port = htons(localPort);
      //printf("after set port\n");
      friendAddr.sin_addr.s_addr = inet_addr(argv[1]);
      struct sockaddr_in* pFriendAddr = (struct sockaddr_in*)malloc(sizeof(struct sockaddr_in));
      pFriendAddr = &friendAddr;
      allPeerAddrs[0] = myAddr;
      allPeerAddrs[1] = friendAddr;
      //printf("before linktochat\n");
      //send discover
      struct sockaddr_in* allAddr = (struct sockaddr_in*)malloc((MAXPEERS-2)*sizeof(struct sockaddr_in));
      allAddr = linkToChat(localFD, pFriendAddr, localPort);
      allPeerAddrs += 2;
      allPeerAddrs = allAddr;
      //printf("addr %s\n", inet_ntoa(allPeerAddrs[0].sin_addr));
      //printf("msg received is : %d\n", ntohs(allPeerAddrs[0].sin_port) );
      //build array
      //size_t noOfPeers = getNoOfPeers(allPeerAddrs);
      //printf("no of peers before add my own %d\n", noOfPeers);
      //allPeerAddrs[noOfPeers+1] = myAddr;
      //printf("allpeers[last] addr is %s\n", inet_ntoa(allPeerAddrs[noOfPeers+1].sin_addr));
      //noOfPeers = getNoOfPeers(allPeerAddrs);
      //printf("no of peers after add my own %d\n", noOfPeers);

      //printf("after linktochat\n");
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
  
  printf("sizeof allPeers is %d\n", sizeof(*allPeerAddrs));
  size_t noOfPeers = getNoOfPeers(allPeerAddrs);
  //printf("noOfPeers is %d\n", noOfPeers);
  
  i=0;
  while(i<noOfPeers)
  {
      //printf("addr %s\n", inet_ntoa(allPeerAddrs[i].sin_addr));
      sendEntry(localFD, nickname, allPeerAddrs[i]);
      i++;
  }

/*  for(i=0; i<MAXPEERS; i++)
  {
  sendEntry(localFD, nickname, allPeerAddrs[i]);
  //printf("after sendEntry in main\n");
  }*/
  FD_ZERO(&readset);
  
  // leave with !Exit
   while(1)
   {
     noOfPeers = getNoOfPeers(allPeerAddrs);
     char* buf2 = malloc(2096* sizeof(char));
     FD_SET(localFD, &readset);
     FD_SET(0, &readset);
     
     events = select(localFD+1, &readset, 0, 0, 0);
     if(FD_ISSET(localFD,&readset))
     {    
	recvPeerMsg(localFD, allPeerAddrs);
     }
     if(FD_ISSET(STDIN_FILENO,&readset))
     {
       getline(&buf2, &len, stdin);
       if(strstr(buf2, "!Exit"))
       {
	  i = 0;
	  while(i<noOfPeers)
	  {
	      sendExit(localFD, nickname, allPeerAddrs[i]);
	      i++;
	  }
	 /*for(i=0;i<(MAXPEERS);i++)
	 {
	 sendExit(localFD, nickname, allPeerAddrs[i]);
	 }*/
	 break;
       }
       else
       {
	 i = 0;
	  while(i<noOfPeers)
	  {
	      sendMsg(localFD, nickname, buf2, allPeerAddrs[i]);
	      i++;
	  }
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
