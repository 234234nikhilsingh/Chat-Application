
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#define MENS_MAX_LEN 500


int socketfd;

void  INThandler(int sig)
{
    signal(sig, SIG_IGN);
    char msg[10]="exit";
    send(socketfd,msg,1000,0);
    close(socketfd);
    exit(0);
    

}

void usage()
{
  fprintf (stderr, "Wrong arguments. Must give two:\n");
  fprintf (stderr, "tcpclient SERVER PORT\n\n");
  fprintf (stderr, "For example:\n");
  fprintf (stderr, "  tcpclient totaki.com 80\n");
  exit(1);
}

void panic(char *msg)
{
  fprintf (stderr, "Fatal error: %s (errno %d, %s)\n", msg, errno, strerror(errno));
  exit(2);
}

int main(int argc, char *argv[])
{
  signal(SIGINT, INThandler);
  char *server;
  int port;
  int finish = 0;
  struct sockaddr_in serverAddress, clientAddress;
  struct hostent *h;
  char buffer [MENS_MAX_LEN];
  fd_set readmask;
  struct timeval timeout;
  size_t recvsize;

  server = "127.0.0.1";
  port = 7891;

  /* Create a TCP socket */
  socketfd = socket(AF_INET, SOCK_STREAM, 0);
  if (socketfd==-1)
    panic("Failed to create socket");

  /* Sets client address */
  clientAddress.sin_family = AF_INET;
  clientAddress.sin_addr.s_addr = htonl(INADDR_ANY);
  clientAddress.sin_port = htons(0);

  /* Bind the address to the socket */
  if (bind(socketfd, (struct sockaddr *)&clientAddress, sizeof(clientAddress))==-1)
    panic("Binding address");

  /* Get the hostname address */
  h = gethostbyname(server);

  /* Sets server address */
  serverAddress.sin_family = h->h_addrtype;
  memcpy((char*) &serverAddress.sin_addr.s_addr, h->h_addr_list[0], h->h_length);
  serverAddress.sin_port = htons(port);

  if (connect(socketfd, (struct sockaddr * )&serverAddress, sizeof(serverAddress))==-1)
    panic("Cannot connect");

  do
    {
      /* We must set all this information on each select we do */
      FD_ZERO(&readmask);   /* empty readmask */
      /* Then we put all the descriptors we want to wait for in a */
      /* mask = readmask */
      FD_SET(socketfd, &readmask);
      FD_SET(STDIN_FILENO, &readmask); /* STDIN_FILENO = 0 (standard input) */
      /* Timeout, we will stop waiting for information */
      timeout.tv_sec=0;
      timeout.tv_usec=100000;

      /* The first parameter is the biggest descriptor+1. The first one
       was 0, so every other descriptor will be bigger.*/
      /* readfds = &readmask */
      /* writefds = we are not waiting for writefds */
      /* exceptfds = we are not waiting for exception fds */
      if (select(socketfd+1, &readmask, NULL, NULL, &timeout)==-1)
    panic("Error on SELECT");

      /* If something was received */
      if (FD_ISSET(socketfd, &readmask))
    {
      recvsize = recv(socketfd, buffer, MENS_MAX_LEN, 0);
      if (recvsize==-1)
        panic("Cannot receive information");
      buffer[recvsize] = '\0';
      if(strcmp(buffer,"exit")==0)
      {
        close(socketfd);
        return 0;
      }
      char *token=strtok(buffer,"\n");
      while(token!=NULL)
      {

      printf ("\t%s\n", token);
      token=strtok(NULL,"\n");
      }


    }

      /* If something was written by the user */
      if (FD_ISSET(STDIN_FILENO, &readmask))
    {
      fgets(buffer, MENS_MAX_LEN, stdin);
	       
      if (send(socketfd, buffer, strlen(buffer)+1, 0)==-1)
        panic ("Cannot sent message");

      strcpy(buffer,"");
    }

    
    } while (!finish);

  return EXIT_SUCCESS;
}
