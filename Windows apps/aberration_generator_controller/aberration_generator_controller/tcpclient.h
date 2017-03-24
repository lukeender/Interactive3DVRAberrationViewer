//==================================================== file = tcpClient.c =====
//=  A message "client" program to demonstrate sockets programming            =
//=============================================================================
//=  Notes:                                                                   =
//=    1) This program conditionally compiles for Winsock and BSD sockets.    =
//=       Set the initial #define to WIN or BSD as appropriate.               =
//=    2) This program needs tcpServer to be running on another host.         =
//=       Program tcpServer must be started first.                            =
//=    3) This program assumes that the IP address of the host running        =
//=       tcpServer is defined in "#define IP_ADDR"                           =
//=    4) The steps #'s correspond to lecture topics.                         =
//=---------------------------------------------------------------------------=
//=  Example execution: (tcpServer and tcpClient running on host 127.0.0.1)   =
//=    Received from server: This is a message from SERVER to CLIENT          =
//=---------------------------------------------------------------------------=
//=  Build: bcc32 tcpClient.c or cl tcpClient.c wsock32.lib for Winsock       =
//=         gcc tcpClient.c -lnsl for BSD                                     =
//=---------------------------------------------------------------------------=
//=  Execute: tcpClient                                                       =
//=---------------------------------------------------------------------------=
//=  Author: Ken Christensen                                                  =
//=          University of South Florida                                      =
//=          WWW: http://www.csee.usf.edu/~christen                           =
//=          Email: christen@csee.usf.edu                                     =
//=---------------------------------------------------------------------------=
//=  History:  KJC (08/02/08) - Genesis (from client.c)                       =
//=            KJC (09/09/09) - Minor clean-up                                =
//=            KJC (09/22/13) - Minor clean-up to fix warnings                =
//=============================================================================
#ifndef TCPSERVER_H
#define TCPSERVER_H
#define  WIN                // WIN for Winsock and BSD for BSD sockets

//----- Include files ---------------------------------------------------------
#include <stdio.h>          // Needed for printf()
#include <string.h>         // Needed for memcpy() and strcpy()
#include <stdlib.h>         // Needed for exit()
#ifdef WIN
  #include <windows.h>      // Needed for all Winsock stuff
#endif
#ifdef BSD
  #include <sys/types.h>    // Needed for sockets stuff
  #include <netinet/in.h>   // Needed for sockets stuff
  #include <sys/socket.h>   // Needed for sockets stuff
  #include <arpa/inet.h>    // Needed for sockets stuff
  #include <fcntl.h>        // Needed for sockets stuff
  #include <netdb.h>        // Needed for sockets stuff
#endif

//----- Defines ---------------------------------------------------------------
#define  PORT_NUM         8090   // Port number used at the server
#define  IP_ADDR    "104.168.29.115"  // IP address of server (*** HARDWIRED ***)
void sendData();
void sendData_Gyro();
void sendData_Accel();
void ArdSerial(char*);
//===== Main program ==========================================================
int tcpclient()
{
#ifdef WIN
  WORD wVersionRequested = MAKEWORD(1,1);       // Stuff for WSA functions
  WSADATA wsaData;                              // Stuff for WSA functions
#endif
  int                  MAXRECV = 50;
  int                  client_s;        // Client socket descriptor
  struct sockaddr_in   server_addr; // Server Internet address
  char                 out_buf[12];   // Output buffer for data
  char*                 in_buf = (char*)malloc((MAXRECV + 1) * sizeof(char));
  //for (int i = 0; i < 12; i++) {
//	  out_buf[i] = ' ';
//	  in_buf[i] = ' ';
//  }// Input buffer for data
  int                  retcode;         // Return code
  bool flag = false;

#ifdef WIN
  // This stuff initializes winsock
  WSAStartup(wVersionRequested, &wsaData);
#endif

  // >>> Step #1 <<<
  // Create a client socket
  //   - AF_INET is Address Family Internet and SOCK_STREAM is streams
  client_s = socket(AF_INET, SOCK_STREAM, 0);
  if (client_s < 0)
  {
    exit(-1);
  }

  // >>> Step #2 <<<
  // Fill-in the server's address information and do a connect with the
  // listening server using the client socket - the connect() will block.
  server_addr.sin_family = AF_INET;                 // Address family to use
  server_addr.sin_port = htons(PORT_NUM);           // Port num to use
  server_addr.sin_addr.s_addr = inet_addr(IP_ADDR); // IP address to use
  retcode = connect(client_s, (struct sockaddr *)&server_addr,
    sizeof(server_addr));
  if (retcode < 0)
  {
    printf("*** ERROR - connect() failed \n");
    return(-1);
  }
  else{
      printf("connected\n");
      send(client_s, "laptop", 7, 0); // tell the python tcp server running on the vps this is the laptop connecting
  }
  // >>> Step #3 <<<
  // Receive from the server using the client socket
  for(;;){
 
	  
  retcode = recv(client_s, in_buf, sizeof(in_buf), 0);
  if (retcode < 0)
  {
    printf("*** ERROR - recv() failed \n");
    return(-1);
  }
  // >>> Step #4 <<<
  //If data recieved successfully, send it to ArdSerial function for processing and sending to Arduino
  else {
	  //printf("sent data");
	 // long int end;
	 // long int begin;
	 // if (flag == true) {
		//  end = GetTickCount();
		  //printf("miliseconds: %i\n", (end - begin));
	 // }
	  printf(in_buf);
	  printf("\n");
	  //ArdSerial(in_buf);
	  //begin = GetTickCount();
	  //flag = !flag;
  }
  }

  // >>> Step #5 <<<
  // Close the client socket
#ifdef WIN
  retcode = closesocket(client_s);
  if (retcode < 0)
  {
    printf("*** ERROR - closesocket() failed \n");
    return(-1);
  }
#endif
#ifdef BSD
  retcode = close(client_s);
  if (retcode < 0)
  {
    printf("*** ERROR - close() failed \n");
    return(-1);
  }
#endif

#ifdef WIN
  // Clean-up winsock
  WSACleanup();
#endif

  // Return zero and terminate
  return(0);
}
#endif