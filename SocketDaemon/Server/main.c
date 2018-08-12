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
#include <mysql/mysql.h>
#include <resolv.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <mysql/mysql.h>
#define TRUE   1
#define FALSE  0
#define PORT 8080
#define BUF_LEN 2048
#define MAX_CLIENTS 4000

//function to split a string along a delimeter
//you will need to call free() on the result of this function or else a mem leak will occur
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


/*---------------------------------------------------------------------*/
/*--- InitServerCTX - initialize SSL server  and create context     ---*/
/*---------------------------------------------------------------------*/
SSL_CTX* InitServerCTX(void)
{   SSL_METHOD *method;
    SSL_CTX *ctx;

    OpenSSL_add_all_algorithms();		/* load & register all cryptos, etc. */
    SSL_load_error_strings();			/* load all error messages */
    method = SSLv23_server_method();		/* create new server-method instance */
    ctx = SSL_CTX_new(method);			/* create new context from method */
    if ( ctx == NULL )
    {
        ERR_print_errors_fp(stderr);
        abort();
    }
    return ctx;
}

/*---------------------------------------------------------------------*/
/*--- LoadCertificates - load from files.                           ---*/
/*---------------------------------------------------------------------*/
void LoadCertificates(SSL_CTX* ctx, char* CertFile, char* KeyFile)
{
	/* set the local certificate from CertFile */
    if ( SSL_CTX_use_certificate_file(ctx, CertFile, SSL_FILETYPE_PEM) <= 0 )
    {
        ERR_print_errors_fp(stderr);
        abort();
    }
    /* set the private key from KeyFile (may be the same as CertFile) */
    if ( SSL_CTX_use_PrivateKey_file(ctx, KeyFile, SSL_FILETYPE_PEM) <= 0 )
    {
        ERR_print_errors_fp(stderr);
        abort();
    }
    /* verify private key */
    if ( !SSL_CTX_check_private_key(ctx) )
    {
        fprintf(stderr, "Private key does not match the public certificate\n");
        abort();
    }
}

int main(int argc , char *argv[])
{
//    printf("processing[  ");
//    while(1) {
//    printf("\b\b|]");
//    fflush(stdout);
//    usleep(5000);
//    printf("\b\b/]");
//    fflush(stdout);
//    usleep(5000);
//    printf("\b\b-]");
//    fflush(stdout);
//    usleep(5000);
//    printf("\b\b\\]");
//    fflush(stdout);
//    usleep(5000);
//    fflush(stdout);
//    }
//

    //connection for the remote database. Used to help verify if a incoming connection is valid
    MYSQL con;
    mysql_init(&con);
    //mysql_options(con,MYSQL_OPT_COMPRESS,1);

    //char arrays for storing the contents of the databaseSettings
   char line [255];
   char file [10][255];
   char table[] = "";

    //File that stores the settings for the database
    FILE *plist = fopen("/var/www/databaseSettings", "r");
    int iter = 0;
    //go through each line of the settings file
    while (fgets(line, sizeof(line), plist)) {
        strcpy(file[iter], fTrim(line));
        iter++;
    }
    //create the connection to the remote database
    if (mysql_real_connect(&con, file[0], file[1], file[2],file[3], 0, NULL, 0) == NULL)
 		{
      		exit(0);
  		}

    SSL_CTX *ctx;
    ctx = InitServerCTX();								/* initialize SSL */
    LoadCertificates(ctx, "/var/www/certs/domain.crt", "/var/www/certs/domain.key");	/* load certs */
    SSL * sslConnections[MAX_CLIENTS];

    int opt = TRUE;
    int master_socket , addrlen , new_socket , client_socket[MAX_CLIENTS] ,
          max_clients = MAX_CLIENTS , activity , valread , sd;
    int max_sd;

    struct sockaddr_in address;


    //directory of the client files for the daemons
    const char clientsLoc[] = "/var/www/clients/";
    //number of files associated with the client daemons and commands
    const int clientFiles = 3;
    const char commandFileName[] = "commands";
    const char clientFileNames[2][BUF_LEN] = {"Sensor Daemon Status","SQL Daemon Status"};

    const char commandControlFile[] = "Command Control Daemon Status";
    //set of socket descriptors
    fd_set readfds;
    char  tables[MAX_CLIENTS][BUF_LEN+1];
    //a message
    char *message = "ECHO Daemon v1.0 \r\n";

    //initialise all client_socket[] to 0 so not checked
    //and blank out the table names
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
    if (listen(master_socket, MAX_CLIENTS) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    //accept the incoming connection
    addrlen = sizeof(address);
    puts("Waiting for connections ...");
    int loops = 0;


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

        struct timeval tout;
        tout.tv_sec = 5;
        tout.tv_usec = 0;

        //wait for an activity on one of the sockets , timeout is NULL ,
        //so wait indefinitely
       // printf("\n");
        activity = select( max_sd + 1 , &readfds , NULL , NULL , &tout);
       // printf("Loops: %d", loops);
       // printf("\n");
       // printf("Activity: %d", activity);
        loops++;
       // printf("\n");
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
            if(FD_ISSET(new_socket,&readfds)) {
               // printf("SET \n");

            } else {

                //printf("NOT SET \n");
            }

            //index of the new socket in the table names array and the socket descripter array
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

            SSL *ssl;
            ssl = SSL_new(ctx);
            SSL_set_fd(ssl, new_socket);
            SSL_accept(ssl);
            sslConnections[newIndex] = ssl;
            //buffer to read in whatever the client sends to us
            char buffer[BUF_LEN+1];  //data buffer of 2K
            //read in whatever the client is sending out
            valread = SSL_read(ssl,buffer,BUF_LEN);
            sleep(1);

            //delared here, set once we know that we read in some valid input from the client socket
            char ** result;
            int disconnect = 0;
            //if to determine whether we were able to read in any information from the socket at all
            if (valread <= 0 )
                {
                //if we weren't able to read any info or client disconnected, flag to disconnect
                disconnect = 1;
                } else { //else if we actually read some stuff into the buffer then check to see if we read in at least 5 characters(needed for next validation)
                    if(strlen(buffer)>=5) {
                        //if the buffer is 5 or more chars long then split the buffer by commas and store in result
                        result = splitString(buffer);
                        //if the first part of the buffer is not equal to VALID then flag for disconnect and free the result to prevent mem leaks
                        if(strcmp(result[0],"VALID")!=0) {
                            disconnect =1;
                            free(result);
                        }
                    } else { //if the buffer was not long enough, flag for disconnect
                        disconnect = 1;
                    }

                }
                //after determining whether we should disconnect new client, disconnect the client if there is a flag for it
                if(disconnect==1){
                    //Somebody disconnected , get his details and print
                    getpeername(sd , (struct sockaddr*)&address , \
                        (socklen_t*)&addrlen);
                    printf("Host disconnected , ip %s , port %d \n" ,
                          inet_ntoa(address.sin_addr) , ntohs(address.sin_port));

                    //Close the socket and mark as 0 in list for reuse
                    FD_CLR(sd,&readfds);
                    close( sd );
                    SSL_free(ssl);
                    client_socket[newIndex] = 0;
                    strcpy(tables[newIndex],"");
                }

                //if we are not flagged to disconnect, actually perform the initial setup for a new client
                else
                {
                    //temp file location string for holding the client files dir and paths
                    char tempLoc[BUF_LEN+1] = "";
                    char * tempLocPt;
                    tempLocPt = strcpy(tempLoc,clientsLoc);

                    //set the string terminating NULL byte on the end
                    //of the data read
                    buffer[valread] = '\0';
                    char tempBuff[BUF_LEN+1];

                    char checkQuery[BUF_LEN+1];
                    //pointer to the check query string so that I can print it out without seg faulting
                    char *checkQueryPtr;
                    //build the check query string
                    //check query to further verify that the client has a valid SQL table in the remote DB
                    checkQueryPtr = strcpy(checkQuery,"SHOW TABLES LIKE '");
                    checkQueryPtr = strcat(checkQuery,result[1]);
                    checkQueryPtr = strcat(checkQuery, "';");
                    //print out the check query
                  //  printf("\n");
                   // printf(checkQueryPtr);
                    //printf("\n");
                    //the result variable for the table check
                    MYSQL_RES *confresCheck;
                   // printf("BBBBBBBBBBBBBBBBBBBB \n");
                    //query to check if table exists. Attempt reconnect every 1s to SQL if we have a error
                    while(mysql_query(&con,checkQueryPtr)) {
                       // printf("MY SQL ERROR \n");
                        //fprintf(stderr, "%s\n", mysql_error(&con));
                        mysql_close(&con);
                        mysql_real_connect(&con, file[0], file[1], file[2],file[3], 0, NULL, 0);
                        printf("\n");
                        sleep(1);
                    }
                   // printf("BBBBBBBBBBBBBBBBBBBB \n");

                   // printf("DDDDDDDDDD \n");
                   //store the result of the table present check in the result variable
                    if((confresCheck = mysql_store_result(&con))){

                    }
                    //printf("DDDDDDDDDD \n");
                    //if the client is attached to a valid table, then create files needed for it if they don't exist
                    //and/or send out the current command to the client
                    if(mysql_num_rows(confresCheck) != 0) {
                        //printf("\n AAAAAAA \n");
                        //copy over the table name into the buffer
                        strcpy(buffer,result[1]);
                       // printf("\n AAAAAAA \n");
                        //free the result since we only need table name for this one
                        free(result);
                        //concat the table name onto the client dir
                        tempLocPt = strcat(tempLoc, buffer);
                        tempLocPt = strcat(tempLoc,"/");
                        //copy over the table dir into the table array
                        for(int i = 0; i < max_clients; i ++) {
                            if(strcmp(tables[i],"")==0) {
                                strcpy(tables[i],tempLoc);
                                break;
                            }
                        }
                       // printf("TWO \n");
                        //used to determine whether the necessary directory exists
                        struct stat st = {0};
                        //char array and pointer to hold path to the command file
                        //and append the command file path onto them
                        char commandPath[BUF_LEN+1] = "";
                        char * commandPathPt = strcpy(commandPath,tempLocPt);
                        commandPathPt = strcat(commandPath,commandFileName);
                        char sPath[BUF_LEN+1] = "";
                        char * sPathPt = strcpy(sPath,tempLoc);
                        sPathPt = strcat(sPathPt,commandControlFile);
                        FILE *socketFile = fopen(sPathPt,"w+");
                        fprintf(socketFile,"CONNECTED");
                        fflush(socketFile);
                        fclose(socketFile);
                        //file pointer for the command file
                        FILE * commandFile;
                        //if the dir for the client does not exist, create the dir and files
                        if(stat(tempLocPt,&st)==-1) {
                            //printf("directory does not exist so we have to create it  \n");
                            //create the dir and set proper permissions with chmod since the param in mkdir does not work
                            mkdir(tempLocPt,0777);
                            chmod(tempLocPt,0777);
                            //loop to create the daemons files in the dir
                            for(int i = 0; i<clientFiles-1; i++) {
                                //holds the file location
                                char tempFileLoc[BUF_LEN+1] = "";
                                char * tempFileLocPt;
                                //concat the file location
                                tempFileLocPt = strcpy(tempFileLoc,tempLoc);
                                tempFileLocPt = strcat(tempFileLoc,clientFileNames[i]);
                                //open the file in write append mode+ which will create if file not present
                                FILE *temp = fopen(tempFileLocPt,"ab+");
                                //change permissions on file to make it properly accessable
                                chmod(tempFileLocPt,0777);
                                //print a default state to the file then close it
                                fprintf(temp,"9");
                                fflush(temp);
                                fclose(temp);
                            }
                            //open command file in write+ mode that'll open it for writing and create it if not present
                            commandFile = fopen(commandPathPt,"w+");
                            //set proper permissions for the file
                            chmod(commandPathPt,0777);
                            fprintf(commandFile,"9");
                            fflush(commandFile);
                            fclose(commandFile);
                        }
                       // printf("THREE \n");
                        //if there is a valid table attached to client and dir exists, then we just read from command file and send command to client
                        commandFile = fopen(commandPathPt,"r");
                        char line[BUF_LEN+1] ="";
                        //get the first line of the command file
                        fgets(line,sizeof(line),commandFile);
                        //copy trimmed line into the line var
                        strcpy(line,fTrim(line));
                        fclose(commandFile);

                        //printf("FOUR \n");
                         //send command to client
                        SSL_write(ssl, line,strlen(line));
                    } else {
                        //printf("FAILED TO GET ROWS \n");
                        //if client does not have a valid table, then free the result, close the socket, and wipe the tables index thing
                        free(result);
                        FD_CLR(sd,&readfds);
                        close(sd);
                        SSL_free(ssl);
                        client_socket[newIndex] = 0;
                        strcpy(tables[newIndex],"");

                    }
                    mysql_free_result(confresCheck);
                }

        }

        //else its some IO operation on some other socket
        for (int i = 0; i < max_clients; i++)
        {
            //buffer for storing the read on the socket
            char buffer[BUF_LEN+1];
            //running socket descriptor
            sd = client_socket[i];
            //if the socket descriptor is not 0, then we read
            if (FD_ISSET( sd , &readfds))
            {
                SSL *ssl = sslConnections[i];
                valread = SSL_read( ssl , buffer, BUF_LEN);
                //check if incoming message has any contents, if not then disconnect the socket descriptor
                if (valread <= 0 || client_socket[i]==0)
                {
                    //Somebody disconnected , get his details and print
                    getpeername(sd , (struct sockaddr*)&address , \
                        (socklen_t*)&addrlen);
                    printf("Host disconnected , ip %s , port %d \n" ,
                          inet_ntoa(address.sin_addr) , ntohs(address.sin_port));
                    char sPath[BUF_LEN+1] = "";
                    char * sPathPt = strcpy(sPath,tables[i]);
                    sPathPt = strcat(sPathPt,commandControlFile);
                    FILE *socketFile = fopen(sPathPt,"w+");
                    fprintf(socketFile,"DISCONNECTED");
                    fflush(socketFile);
                    fclose(socketFile);
                    //Close the socket and mark as 0 in list for reuse
                    FD_CLR(sd,&readfds);
                    SSL_free(ssl);
                    close( sd );
                    strcpy(tables[i],"");
                    client_socket[i] = 0;
                }

                //Echo back the message that came in
                else
                {

                    //printf("\n CCCC \n");
                   // printf("Information recieved: ");
                   // printf(buffer);
                   // printf("\n");
                    //set the string terminating NULL byte on the end
                    //of the data read
                    buffer[valread] = '\0';
                    //split the read by the commas
                    char ** tempBuff = splitString(buffer);
                    //store the path of the client directory associated with this table
                    char tablePath[BUF_LEN+1] = "";
                    char * tablePathPt = strcpy(tablePath,tables[i]);

                    //loop through client files and write various parts of the read message to those file
                    for(int i2 = 1; i2 < clientFiles; i2++) {
                        //store the path of one daemon file
                        char daemonPath[BUF_LEN+1];
                        char * daemonPathPt = strcpy(daemonPath,tablePath);
                        daemonPathPt = strcat(daemonPath,clientFileNames[i2-1]);
                        //open the daemon file for writing
                        FILE * daemonFile = fopen(daemonPathPt, "w");
                        //write to the daemon file
                        fprintf(daemonFile,tempBuff[(i2+1)]);
                        //print out the file and what was written to it
                       // printf("Daemon File: ");
                       // printf(daemonPathPt);
                       // printf("\n");
                       // printf("Status: ");
                       // printf(tempBuff[(i2+1)]);
                       // printf("\n");
                        //flush the file stream and close the file
                        fflush(daemonFile);
                        fclose(daemonFile);
                    }
                    //store the command file path
                    char commandPath[BUF_LEN+1] = "";
                    char * commandPathPt;
                    commandPathPt = strcat(commandPath,tables[i]);
                    commandPathPt = strcat(commandPath,commandFileName);
                    //print the command file path
                    //printf(commandPathPt);
                    //printf("\n \n");
                    //open the command file for reading
                    FILE * commandFile = fopen(commandPathPt,"r");
                    //get the first line of the commandFile and trim and copy it into a char array
                    char line[BUF_LEN+1] ="";
                    fgets(line,sizeof(line),commandFile);
                    strcpy(line,fTrim(line));
                    //flush and close the file
                    fflush(commandFile);
                    fclose(commandFile);
                    //send the command out to the client socket
                    SSL_write(ssl, line,strlen(line));
                    //free the split buffer
                    free(tempBuff);
                }

            }
        }
    }

    return 0;
}
