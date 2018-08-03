//Example code: A simple server side code, which echos back the received message.
//Handle multiple socket connections with select and fd_set on Linux
#include <stdio.h>
#include <string.h>   //strlen
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>   //close
#include <arpa/inet.h>    //close
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros
//#include <sys/types.h>
#include <sys/stat.h>
//#include <unistd.h>
#include <mysql.h>
#define TRUE   1
#define FALSE  0
#define PORT 8080
#define BUF_LEN 2048

char ** splitString(str) {

     int i;
     int count = 0;
     const char delims[] = ",";
     char *result = NULL;
     char **store = NULL;
     char **tmp = NULL;
     result = strtok(str, delims);
     while (result != NULL) {
          free(tmp);
          tmp = malloc(count * sizeof(char *));
          for (i=0; i<count; i++) {
               tmp[i] = store[i];
          }
          free(store);
          store = malloc((count + 1) * sizeof(char *));
          for (i=0; i<count; i++) {
               store[i] = tmp[i];
          }
          store[count] = result;
          count++;
         // printf("%s\n", result);

          result = strtok(NULL, delims);
     }

     free(tmp);
    // free(store);
return store;

}
//trims off the newlines characters from the fgets read
char * fTrim (char s[]) {
  int i = strlen(s)-1;
  if ((i > 0) && (s[i] == '\n'))
    s[i] = '\0';
  return s;
}


int main(int argc , char *argv[])
{



    //connection for the remote database
    MYSQL con;
    mysql_init(&con);
    mysql_options(&con,MYSQL_OPT_COMPRESS,1);

    //char arrays for storing the contents of the databaseSettings
   char line [255];
   char file [10][255];
   char table[] = "";

//File to store the settings for the database
FILE *plist = fopen("/var/www/databaseSettings", "r");
int iter = 0;
//go through each line of the settings file
while (fgets(line, sizeof(line), plist)) {
    strcpy(file[iter], fTrim(line));
    iter++;
}
//create the connection to the remote database
    if (mysql_real_connect(&con, file[0], file[1], file[2],file[3], 0, NULL, 1) == NULL)
 		{
      		exit(0);
  		}

    int opt = TRUE;
    int master_socket , addrlen , new_socket , client_socket[30] ,
          max_clients = 30 , activity , valread , sd;
    int max_sd;
    struct sockaddr_in address;

    const char clientsLoc[] = "/var/www/clients/";
    const int clientFiles = 3;
    const char commandFileName[] = "commands";
    const char clientFileNames[2][BUF_LEN] = {"Sensor Daemon Status","SQL Daemon Status"};
    //set of socket descriptors
    fd_set readfds;
    char  tables[30][BUF_LEN];
    //a message
    char *message = "ECHO Daemon v1.0 \r\n";

    //initialise all client_socket[] to 0 so not checked
    for (int i = 0; i < max_clients; i++)
    {
        client_socket[i] = 0;
        strcpy(tables[i],"");
    }

    //create a master socket
    if( (master_socket = socket(AF_INET , SOCK_STREAM , 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    //set master socket to allow multiple connections ,
    //this is just a good habit, it will work without this
    if( setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt,
          sizeof(opt)) < 0 )
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    //type of socket created
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( PORT );

    //bind the socket to localhost port 8888
    if (bind(master_socket, (struct sockaddr *)&address, sizeof(address))<0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    printf("Listener on port %d \n", PORT);

    //try to specify maximum of 3 pending connections for the master socket
    if (listen(master_socket, 30) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    //accept the incoming connection
    addrlen = sizeof(address);
    puts("Waiting for connections ...");

    while(TRUE)
    {
        //clear the socket set
        FD_ZERO(&readfds);

        //add master socket to set
        FD_SET(master_socket, &readfds);
        max_sd = master_socket;

        //add child sockets to set
        for (int i = 0 ; i < max_clients ; i++)
        {
            //socket descriptor
            sd = client_socket[i];

            //if valid socket descriptor then add to read list
            if(sd > 0)
                FD_SET( sd , &readfds);

            //highest file descriptor number, need it for the select function
            if(sd > max_sd)
                max_sd = sd;
        }

        //wait for an activity on one of the sockets , timeout is NULL ,
        //so wait indefinitely
        activity = select( max_sd + 1 , &readfds , NULL , NULL , NULL);

        if ((activity < 0) && (errno!=EINTR))
        {
            printf("select error");
        }

        //If something happened on the master socket ,
        //then its an incoming connection
        if (FD_ISSET(master_socket, &readfds))
        {
            if ((new_socket = accept(master_socket,
                    (struct sockaddr *)&address, (socklen_t*)&addrlen))<0)
            {
                perror("accept");
                exit(EXIT_FAILURE);
            }

            //inform user of socket number - used in send and receive commands
            printf("New connection , socket fd is %d , ip is : %s , port : %d \n" , new_socket , inet_ntoa(address.sin_addr) , ntohs(address.sin_port));

            int newIndex = -1;
            //add new socket to array of sockets
            for (int i = 0; i < max_clients; i++)
            {
                //if position is empty
                if( client_socket[i] == 0 )
                {
                    client_socket[i] = new_socket;
                    sd = client_socket[i];
                    printf("Adding to list of sockets as %d\n" , i);
                    newIndex = i;
                    break;
                }
            }
            char buffer[BUF_LEN];  //data buffer of 1K
            printf("\n new connection break 1 \n ");
            valread = read( sd , buffer, BUF_LEN);
            printf("\n new connection break 1 \n ");
            char *breakBuf = strcat(buffer,"");
            printf(breakBuf);
            printf("\n");
            if (valread <= 0)
                {
                    //Somebody disconnected , get his details and print
                    getpeername(sd , (struct sockaddr*)&address , \
                        (socklen_t*)&addrlen);
                    printf("Host disconnected , ip %s , port %d \n" ,
                          inet_ntoa(address.sin_addr) , ntohs(address.sin_port));

                    //Close the socket and mark as 0 in list for reuse
                    close( sd );
                    client_socket[newIndex] = 0;
                    strcpy(tables[newIndex],"");
                }

                //Echo back the message that came in
                else
                {

                    char tempLoc[BUF_LEN] = "";
                    char * tempLocPt;
                    tempLocPt = strcpy(tempLoc,clientsLoc);

                    //set the string terminating NULL byte on the end
                    //of the data read
                    printf("\n break 2 \n");
                    buffer[valread] = '\0';
                    char tempBuff[BUF_LEN];
                    char ** result = splitString(buffer);
                     printf("\n break 2 \n");
                    //char * tempBuffPt = strcpy(tempBuff,buffer);


                    char checkQuery[255];
                    //pointer to the check query string so that i can print it out without seg faulting
                    char *checkQueryPtr;
                    //build the check query string
                    checkQueryPtr = strcpy(checkQuery,"SHOW TABLES LIKE '");
                    checkQueryPtr = strcat(checkQuery,buffer);
                    checkQueryPtr = strcat(checkQuery, "';");

                    //the result variable for the table check
                    MYSQL_RES *confresCheck;
                    //query to check if table exists
                    mysql_query(&con,checkQueryPtr);
                    //store the result of the table present check in the result variable
                    confresCheck = mysql_store_result(&con);
                    if(mysql_num_rows(confresCheck) != 0) {
                    strcpy(buffer,result[0]);
                    free(result);
                    tempLocPt = strcat(tempLoc, buffer);
                    tempLocPt = strcat(tempLoc,"/");
                    for(int i = 0; i < max_clients; i ++) {
                    if(strcmp(tables[i],"")==0) {
                    strcpy(tables[i],tempLoc);
                    break;
                    }
                    }
                    struct stat st = {0};
                    char commandPath[BUF_LEN] = "";
                    char * commandPathPt = strcpy(commandPath,tempLocPt);
                    commandPathPt = strcat(commandPath,commandFileName);
                    FILE * commandFile;
                    if(stat(tempLocPt,&st)==-1) {
                    printf("directory does not exist so we have to create it  \n");
                    mkdir(tempLocPt,0777);
                    chmod(tempLocPt,0777);
                    for(int i = 0; i<clientFiles-1; i++) {
                        char tempFileLoc[BUF_LEN] = "";
                        char * tempFileLocPt;
                        tempFileLocPt = strcpy(tempFileLoc,tempLoc);
                        tempFileLocPt = strcat(tempFileLoc,clientFileNames[i]);
                        FILE *temp = fopen(tempFileLocPt,"ab+");
                        chmod(tempFileLocPt,0777);
                        fprintf(temp,"9");
                        fflush(temp);
                        fclose(temp);
                    }
                    commandFile = fopen(commandPathPt,"w+");
                    chmod(commandPathPt,0777);
                    fprintf(commandFile,"9");
                    fflush(commandFile);
                    fclose(commandFile);
                    }
                    commandFile = fopen(commandPathPt,"r");
                    char line[BUF_LEN] ="";
                    fgets(line,sizeof(line),commandFile);
                    strcpy(line,fTrim(line));
                    fclose(commandFile);
                    send(sd, line,strlen(line),0);
                    } else {
                    free(result);
                    close(sd);
                    client_socket[newIndex] = 0;
                    strcpy(tables[newIndex],"");
                    }
                mysql_free_result(confresCheck);
                }

        }

        //else its some IO operation on some other socket
        for (int i = 0; i < max_clients; i++)
        {
            char buffer[BUF_LEN];
            sd = client_socket[i];

            if (FD_ISSET( sd , &readfds))
            {

                //Check if it was for closing , and also read the
                //incoming message
                if ((valread = read( sd , buffer, BUF_LEN)) <= 0)
                {
                    //Somebody disconnected , get his details and print
                    getpeername(sd , (struct sockaddr*)&address , \
                        (socklen_t*)&addrlen);
                    printf("Host disconnected , ip %s , port %d \n" ,
                          inet_ntoa(address.sin_addr) , ntohs(address.sin_port));

                    //Close the socket and mark as 0 in list for reuse
                    close( sd );
                    strcpy(tables[i],"");
                    client_socket[i] = 0;
                }

                //Echo back the message that came in
                else
                {
                    //set the string terminating NULL byte on the end
                    //of the data read

                    printf("Information recieved: ");
                    printf(buffer);
                    printf("\n");
                    buffer[valread+1] = '\0';
                    char ** tempBuff = splitString(buffer);
                    char tablePath[BUF_LEN] = "";
                    char * tablePathPt = strcpy(tablePath,tables[i]);


                    for(int i2 = 1; i2 < clientFiles; i2++) {
                    char daemonPath[BUF_LEN];
                    char * daemonPathPt = strcpy(daemonPath,tablePath);
                    daemonPathPt = strcat(daemonPath,clientFileNames[i2-1]);
                    FILE * daemonFile = fopen(daemonPathPt, "w");
                    fprintf(daemonFile,tempBuff[i2]);
                    printf("Daemon File: ");
                    printf(daemonPathPt);
                    printf("\n");
                    printf("Status: ");
                    printf(tempBuff[i2]);
                    printf("\n");
                    fflush(daemonFile);
                    fclose(daemonFile);
                    }

                    char commandPath[BUF_LEN] = "";
                    char * commandPathPt;
                    commandPathPt = strcat(commandPath,tables[i]);
                    commandPathPt = strcat(commandPath,commandFileName);
                    printf(commandPathPt);
                    printf("\n \n");
                    FILE * commandFile = fopen(commandPathPt,"r");
                    char line[BUF_LEN] ="";
                    fgets(line,sizeof(line),commandFile);
                    strcpy(line,fTrim(line));
                    fflush(commandFile);
                    fclose(commandFile);
                    send(sd, line,strlen(line),0);
                    free(tempBuff);
                }
            }
        }
    }

    return 0;
}
