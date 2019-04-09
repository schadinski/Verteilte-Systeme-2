#include "chatLib.h"

int main(int argc, char *argv[])
{
  printf("first line \n");
    
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
  int* peerFDs;
  struct sockaddr_in* allPeerAddrs;
  //char myIP[20];
  int localFD;
 // char* allIPs;
  //in_addr_t friendIP;
  int activePeers = 0;
  struct sockaddr_in myAddr;
  
  printf("after var init\n");
  nickname = malloc(32*sizeof(char));
//  friendIP = malloc(sizeof(in_addr_t);
  //allIPs = (in_addr_t*)malloc(MAXPEERS*sizeof(in_addr_t));
 // printf("argv[2] is %s\n", argv[2]);
  //myIP = malloc(sizeof(argv[2])*sizeof(char));
  
  //strncpy(myIP,argv[2], sizeof(argv[2]));
    //strcpy(myIP,argv[2]);

  //friendIP = inet_addr(argv[1]);
  //  printf("after set friendip\n");

  //allIPs[activePeers] = inet_addr(argv[1]);
  //TODO senDiscover braucht localFD
 // sendDiscover(friendIP, localPort);
  
  // generiere meine Addresse, inkl fd und bind
  // send discover
  // lausche auf antwort:
  //        recvfrom mit timeout-> dann return 1
  //        recvfrom erfolgreich: - hole alle addressen raus
  //                              - fertige Liste mit allen addressen + meiner Adresse
//                                - sendEntry() an alle
  
  // build own Addresse
 // myAddr = (struct sockaddr_in*)malloc(sizeof(struct sockaddr_in));
    memset(&myAddr, 0, sizeof(myAddr));
    
    localFD = socket( AF_INET, SOCK_DGRAM, 0);
    printf("after socket\n");
    myAddr.sin_family = AF_INET;
        printf("after sin_family\n");

    myAddr.sin_port = htons(localPort);
    printf("after set port\n");
    myAddr.sin_addr.s_addr = inet_addr(argv[2]);
    printf("after setup myAddr\n");
    struct sockaddr_in* pMyAddr = (struct sockaddr_in*)malloc(sizeof(struct sockaddr_in));
    pMyAddr = &myAddr;
    int rc = bind(localFD, (struct sockaddr*)pMyAddr, sizeof(pMyAddr));
      if (rc < 0)
      {
        printf("Error: Bind local FD\n");
        perror("bind()");
      }
  printf("before linktochat\n");

  allPeerAddrs = linkToChat(localFD, argv[1], localPort);
  printf("after linktochat\n");
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
  
  for(i=0; i<MAXPEERS; i++)
  {
  sendEntry(localFD, nickname, allPeerAddrs[i]);
  //printf("after sendEntry in main\n");
  }
  FD_ZERO(&readset);
  
  // leave with !Exit
   while(1)
   {
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
	 for(i=0;i<(MAXPEERS);i++)
	 {
	 sendExit(localFD, nickname, allPeerAddrs[i]);
	 }
	 break;
       }
       else
       {
	  for(i=0;i<(MAXPEERS);i++)
	  { 
	    sendMsg(localFD, nickname, buf2, allPeerAddrs[i]);
	  }
       }
     }
     free(buf2);
   }
 for(i=0; i<(MAXPEERS);i++)
 {
   close(peerFDs[i]);
 }
 close(localPort);
 free(nickname);
 //free(friendIP);
 return 0;
}
