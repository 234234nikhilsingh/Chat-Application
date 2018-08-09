

    #include <errno.h> 		
    #include <semaphore.h> 	
    #include <sys/socket.h> 	
    #include <string.h> 	
    #include <stdlib.h> 	
    #include <sys/mman.h> 	
    #include <sys/wait.h> 	
    #include <netinet/in.h> 	
    #include <unistd.h> 	
    #include <arpa/inet.h> 
    #include <stdio.h> 
    #include <sys/mman.h> 	
    #include <time.h> 
    #include <fcntl.h>
    #include <stdlib.h>  	
    #include <pthread.h> 	
    #include <poll.h>		

    struct pollfd poll_set[6];

    char buffer[10000]; // To be used for sending/receiving msgs to/from the client
    sem_t *sema = NULL; // semaphore to be used for concurrency
    sem_t *msgSema=NULL;



  //store client information
  typedef struct node {
    char connectionTime[30]; //Will store connection time information about the client
    int id; //Will have the id of the connected client
    int socket; //Will contain client socket
    int status;

  }
  Client;
  Client allClient[6];
  Client nullClient;

//structure for msg
  typedef struct mg
  {
   char message[1024];
   int source;
   int destination; 
   int all;
  }Message;


  Message messages[5];
  Message nullMsg;

  int noOfMsgs;
  int count; 
  

//It enqueues a message from the queue
  int enqueueMsg(Message msg)
  {
    if(noOfMsgs==5) return -1;
   else{
      sem_wait(msgSema);
      messages[noOfMsgs]=msg;
      noOfMsgs++;
      printf("\nMessage enqueued successfully");
      fflush(stdin);
      fflush(stdout);
      sem_post(msgSema);
      return 1;
    }
    
  }

//it dequeues the msg from the queue
  Message dequeueMsg()
  {
    if(noOfMsgs==0) return nullMsg;
    sem_wait(msgSema);
    Message send=messages[0];
    for(int i=1;i<=5;i++)
    {
      messages[i]=messages[i+1];
    }
    noOfMsgs--;
    printf("\nMessage dequeued successfully");
    fflush(stdin);
    fflush(stdout);
    sem_post(msgSema);
    return send;
  }


  void printClientInfo(Client client) {

    printf("\nConnectioned At            : %s", client.connectionTime);
    printf("\nClient Id Is               : %d", client.id);
    printf("\nClient Connected At Socket : %d", client.socket);
    printf("\n");

  }

  /*   
    This function converts the string to lower case.It takes one para       
    which is nothing but the input string.                                   
 
  */

  void stringLowerCase(char s[]) {
    int c = 0;
    // check till end of string
    while (s[c] != '\0') {
      //if a upper case character is found,add 32 to convert to lower case
      if (s[c] >= 'A' && s[c] <= 'Z') {
        s[c] = s[c] + 32;
      }
      c++;
    }
  }

  /*   
     This function performs a write operation when readWrite=1 and performs 
     a read operation when readWrite=0.The second param line contains line  
     to written to server.txt in case of write request whereas for read     
     request it contains the line number                              
                                                                        
    */

  //generate an id for client in range 10000 to 90000

  int getClientId() {
    int randomID = rand() % 90000 + 10000;
    return randomID;
  }

  //    sem_wait (sema);  
  //    sem_post (sema);




  /*
     This function checks for all online client.It returns 
     all the clients that are connected to the app.
      The list also includes the id of the caller client

  */
  
  char *getOnlineClients(int excludeId)
  {
  static char getList[400];
             char cid[10];

  strcpy(getList,"Online Clients:");
  for(int i=1;i<=5;i++)
  {
    if(allClient[i].id!=nullClient.id)
      {
         if(allClient[i].id!=excludeId)
        { 
           sprintf(cid,"%d",allClient[i].id);
           strcat(getList,"\n");
           strcat(getList,cid);
           
        }   
   


      }

  }

  sprintf(cid,"%d",excludeId);
  strcat(getList,"\n");
  strcat(getList,cid);
  strcat(getList,"(You)");
  
  return getList;
  }


  /*    
   
     This function performs a write operation when readWrite=1 and performs 
     a read operation when readWrite=0.The second param line contains line  
     to written to server.txt in case of write request whereas for read     
     request it contains the line number                                    
 
  */

  char *strToInt(int no)
  {
    static char ans[50];
    strcpy(ans,"");
    sprintf(ans,"%d",no);
    return ans;
  }


  void init()
  {
              
              for(int i=0;i<=5;i++){
              poll_set[i].fd=0;
              poll_set[i].events=POLLIN;
              }                                                                                          // For Handling Concurrency
    msgSema=(sem_t * ) mmap(NULL, sizeof(*msgSema), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    noOfMsgs = 0;
    count=0;
    
  /*  initialize the semaphore */
    if (sem_init(msgSema, 1, 1) < 0) {
      perror("sem_init");
      exit(EXIT_FAILURE);
    }

    nullClient.id=-1;
    nullMsg.source=-1000;
    nullMsg.destination=-1000;
     
    for(int i=1;i<=5;i++)
     allClient[i]=nullClient;  
  }

  int findSocketDescriptor(int num)
  {
    for(int i=1;i<=5;i++)
    {
      if(allClient[i].id==num)
        return allClient[i].socket;
    }
    return -1;

  }
  void *handler(void *x)
  {

      while(1)
      {
        Message ii=dequeueMsg();
        if(ii.source==nullMsg.source) continue;
        int source=findSocketDescriptor(ii.source);
        if(ii.all==0)
        {
            
            int destination=findSocketDescriptor(ii.destination);
            
            if(destination==-1)
              {
                char msg[100]="Destination Could Not Be Found.Pleas try again";
                send(source,msg,1000,0);
              }  
              send(destination,ii.message,1000,0);

        }
        else
        {
            for(int i=1;i<=5;i++)
            {
              if(allClient[i].id!=ii.source && allClient[i].id!=nullClient.id)
              {
                int destination=findSocketDescriptor(allClient[i].id);
                send(destination,ii.message,1000,0);
              }
            }
        }
        
      }


  }

  /*Message createMessage(int sourceId,int destinationId,char  msg[])
  {
    return message;
  }*/

  Client getClientByFD(int fd)
  {
    for(int i=1;i<=5;i++)
    {
      if(allClient[i].socket==fd)
      {
        return allClient[i];
      }
    }
  }

  int addSocket(int socketfd)
  {


          char welcomeMsg[500];
          if(count+1<=5)
          {
            count++;
          //create client
          Client newClient;

          //Fill the value of client
          newClient.socket = socketfd;
          newClient.id =  getClientId();
          time_t rawtime;
          time(&rawtime);
          strcpy(newClient.connectionTime, asctime(localtime(&rawtime)));
          allClient[count] = newClient;          
          strcpy(welcomeMsg,"Welcome To the App.");
          strcat(welcomeMsg,"Your details are as below:");
          strcat(welcomeMsg,"\nId :");
          strcat(welcomeMsg,strToInt(newClient.id));
          strcat(welcomeMsg,"\nConnected At :");
          strcat(welcomeMsg,newClient.connectionTime);
          poll_set[count].fd=socketfd;
          poll_set[count].events=POLLIN;
    
          send(socketfd,welcomeMsg,1000,0);
          return count;
          }
          else
          {
            strcpy(welcomeMsg,"Connection Limit Exceeded");
            send(socketfd,welcomeMsg,1000,0);
            strcpy(welcomeMsg,"exit");
            send(socketfd,welcomeMsg,1000,0);
            close(socketfd);
          }

          
          return -1;

  }


  /*void *handler(void *x)
  {
     printf("handler started \n");
            fflush(stdin);
            fflush(stdout);
    while(1){
    //  for( int i =0; i < MAX_CONN; i++ )
        {
          if( poll_set[i].revents & POLLIN )
          {
            
            printf("We have a connection \n");
            fflush(stdin);
            fflush(stdout);
          }

        } 
    }

  }*/

  int welcomeSocket;

void  INThandler(int sig)
{
    signal(sig, SIG_IGN);
    char msg[10]="exit";
    for(int i=1;i<=5;i++)
    {
      if(allClient[i].id!=nullClient.id){
        send(allClient[i].socket,msg,1000,0);  
        close(allClient[i].socket);
      }
    }
    close(welcomeSocket);
    exit(0);
    

}


  int main() {
  signal(SIGINT, INThandler);
    init();
    pthread_t clientThread;
    int xx;

    if(pthread_create(&clientThread, NULL, handler, xx)) {
    fprintf(stderr, "Error creating thread\n");
    return 1;
    }

    char welcomeMsg[500];
    // declare a file pointer
    FILE *fp;


        
    // two sockets
    int  clientSocket;

    //command will contain the command to be executed i.e it will have either read/write/exit
    //it may contain invalid command also 
    char command[30];

    //line will contain the statement after the command 'read'/'write' 
    char line[10000];

    struct sockaddr_in serverAddr;
    struct sockaddr_storage serverStorage;
    socklen_t addr_size;

    /*---- Create the socket. The three arguments are: ----*/
    /* 1) Internet domain 2) Stream socket 3) Default protocol (TCP in this case) */
    welcomeSocket = socket(PF_INET, SOCK_STREAM, 0);
    if (welcomeSocket == -1) {
      printf("Error: unable to open a socket\n");
      exit(1);
    }

    /*---- Configure settings of the server address struct ----*/
    /* Address family = Internet */
    serverAddr.sin_family = AF_INET;
    /* Set port number, using htons function to use proper byte order */
    serverAddr.sin_port = htons(7891);
    /* Set IP address to localhost */
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    /* Set all bits of the padding field to 0 */
    memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);

    /*---- Bind the address struct to the socket ----*/
    if (bind(welcomeSocket, (struct sockaddr * ) & serverAddr, sizeof(serverAddr)) == -1) {
      printf("Error: unable to bind\n"); // Error while binding to socket
      exit(1);
    }

    /*---- Listen on the socket, with 5 max connection requests queued ----*/
    if (listen(welcomeSocket, 5) == 0);
    else //error in serving the request
    {

      printf("Error in listening/serving the client\n");
      exit(1);
    }
  
    poll_set[0].fd=welcomeSocket;
    addr_size = sizeof serverStorage;
    while (1) 
    {
        poll(poll_set, count+1, -1);
        for(int fd_index = 0; fd_index < count+1; fd_index++)
        {

            if(poll_set[fd_index].fd==0) continue;

            if( poll_set[fd_index].revents & POLLIN ) 
            {
                
                if(poll_set[fd_index].fd == welcomeSocket) 
                  {  //accept client connection
                      clientSocket = accept(welcomeSocket, (struct sockaddr *) & serverStorage, & addr_size); //accept request from client
                      addSocket(clientSocket);

                  }
    

                else
                  {  
                              

                              strcpy(buffer,"");
                              strcpy(line,"");
                             int noOfBytes= recv(poll_set[fd_index].fd, buffer, 10000, 0);

                             Client newClient=getClientByFD(poll_set[fd_index].fd);
                             printf("\nServing client %d\n", newClient.id);

                                    if(noOfBytes>0)
                                      {
                                              char *pos;
                                              if ((pos=strchr(buffer, '\n')) != NULL)
                                                *pos = '\0';
                                          char *token = strtok(buffer, " ");
                                          strcpy(command, token);
                                          token = strtok(NULL, " ");
                                          stringLowerCase(command);        
                                          while (token != NULL) 
                                          {
                                              strcat(line, token);
                                              strcat(line," ");
                                              token = strtok(NULL, " ");
                                          }
                                      

                                                if(strcmp(command,"+broadcast")==0)
                                                {

                                                  Message message;    
                                                  strcpy(message.message,strToInt(newClient.id));
                                                  strcat(message.message,": ");
                                                  strcat(message.message,line);
                                                  message.all=1;
                                                  //message.destination=clientId;
                                                  message.source=newClient.id;
   
                                                  int status=enqueueMsg(message);
                                                  if(status==-1)
                                                  {
                                                      strcpy(line,"Buffer is full");
                                                      send(newClient.socket,line,1000,0);
                                                  } 
                                                    continue;    
                                                }


                                                if(strcmp(command,"all")==0)
                                                {
                                                  char * list=getOnlineClients(newClient.id);
                                                  send(newClient.socket,list,1000,0);
                                                  continue;
                                                }

                                                if(strcmp(command,"exit")==0)
                                                {


                                                    int deleteId;
                                                    for(int i=1;i<=5;i++)
                                                      {  
                                                          if(allClient[i].id==newClient.id)
                                                          {
                                                            deleteId=i;
                                                              break;

                                                          }
                                                      }
                                                      printf("Removing Client %d From the App\n",allClient[deleteId].id);
                                                      char deleteMsg[30]="";
                                                      strcat(deleteMsg,strToInt(newClient.id));
                                                      strcat(deleteMsg,": Exiting from app");

                                                     for(int k=1;k<=5;k++)
                                                     {
                                                      if(allClient[k].id==nullClient.id || allClient[k].id==newClient.id) continue;
                                                      send(allClient[k].socket,deleteMsg,1000,0);
                                                     }
                                                      

                                                      for(int j=deleteId;j<=5;j++)
                                                      {
                                                        allClient[j]=allClient[j+1];
                                                      }
                                                      allClient[5]=nullClient;

                                                      count--;

                                                        send(newClient.socket,command,1000,0);
                                                        close(newClient.socket);
                                                      poll_set[5].fd=0;

                                                        for(int k=fd_index;k<=4;k++)
                                                        {
                                                          poll_set[k]=poll_set[k+1];
                                                        }
                                                        continue;
                                                }
                                      
                                                  int clientId = atoi(command); // Get the integer value from the text
                                      


                                                  if (clientId <= 0)         // Invalid Request. Line number can't be less than or equal to 0
                                                  {
                                                      strcpy(line,"Bad Request: Please Enter Valid Command!.\n1) Type <+broadcast|clientId> <msg_string> \n 2)Enter 'all' to get all online clients");
                                                      send(newClient.socket,line,1000,0);          
                                                  } 
                             
                                                  else
                                                  {
                                          
                                                      Message message;    
                                                      strcpy(message.message,strToInt(newClient.id));
                                                      strcat(message.message,": ");
                                                      strcat(message.message,line);
                                                      message.destination=clientId;
                                                      message.source=newClient.id;
                                                      message.all=0;
                                                      int status=enqueueMsg(message);
                                                      if(status==-1)
                                                      {
                                                            strcpy(line,"Buffer is full");
                                                            send(newClient.socket,line,1000,0);
                                                      } 
                                                  }
                                  
                                        fflush(stdout);
                                        fflush(stdin);

                                  }//end of if




                  }

            }
            
        }
    }      

    
    return 0;
  }
