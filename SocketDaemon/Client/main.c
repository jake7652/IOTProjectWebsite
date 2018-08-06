// Client side C/C++ program to demonstrate Socket programming
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/wait.h>
#define PORT 8080
#define BUF_LEN 2048

//trims off the newlines characters from the fgets read
char * fTrim (char s[]) {
  int i = strlen(s)-1;
  if ((i > 0) && (s[i] == '\n'))
    s[i] = '\0';
  return s;
}


static int exec_prog(const char **argv)
{
    pid_t   my_pid;
    int     status, timeout /* unused ifdef WAIT_FOR_COMPLETION */;

    if (0 == (my_pid = fork())) {
            if (-1 == execve(argv[0], (char **)argv , NULL)) {
                    perror("child process execve failed [%m]");
                    return -1;
            }
    }

#ifdef WAIT_FOR_COMPLETION
    timeout = 1000;

    while (0 == waitpid(my_pid , &status , WNOHANG)) {
            if ( --timeout < 0 ) {
                    perror("timeout");
                    return -1;
            }
            sleep(1);
    }

    printf("%s WEXITSTATUS %d WIFEXITED %d [status %d]\n",
            argv[0], WEXITSTATUS(status), WIFEXITED(status), status);

    if (1 != WIFEXITED(status) || 0 != WEXITSTATUS(status)) {
            perror("%s failed, halt system");
            return -1;
    }

#endif
    return 0;
}

int main(int argc, char const *argv[])
{
    //start daemons at beginning of this in order to get rid of faulty status reporting when this daemon is running
    const char * sensorArgs[255] = {"/var/www/daemons/sensor.sh","debug"};
    const char * sqlArgs[255] = {"/var/www/daemons/SQL.sh","debug"};
    exec_prog(sensorArgs);
    exec_prog(sqlArgs);
    //line of the sql settings file
    char line [BUF_LEN+1] = "";
    //all lines of the sql settings file
    char file [10][BUF_LEN+1];
    //table that this pi is set to write to in the SQL settings file
    char table[BUF_LEN+1] = "";
    //File to store the settings for the database
    FILE *plist = fopen("/var/www/databaseSettings", "r");

    //directory of the client files
    char clientsLoc[] = "/var/www/clients/";
    int daemons = 3;
    //all daemon related file paths
    char * daemonPaths[3] = {"/var/www/clients/commitDaemon","/var/www/clients/sensorDaemon","/var/www/clients/commands"};
    //path for the file containing the daemon commands
    char commandFilePath[] = "/var/www/clients/commands";
    //file pointer for the daemon command file
    FILE * commandFile;
    //go through each line of the SQL settings file
    int i = 0;
    while (fgets(line, sizeof(line), plist)) {
        strcpy(file[i], fTrim(line));
        strcpy(table, fTrim(line));
        i++;
    }
    //address of the socket on this client
    struct sockaddr_in address;
    //sock status and read var
    int sock = 0, valread;
    //address of the socket server
    struct sockaddr_in serv_addr;
    //name of the table that this client writes to
    char *hello = file[4];
    //buffer that the socket reads into
    char buffer[BUF_LEN+1] = {0};

    //0 out the server address in memory
    memset(&serv_addr, '0', sizeof(serv_addr));
    //set the socket family and port
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    //create the socket on the local machine
    if ((sock = socket(PF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error \n");
        return -1;
    }
    // Convert IPv4 and IPv6 addresses from text to binary form
    while(inet_pton(AF_INET, "68.134.4.105", &serv_addr.sin_addr)<=0)
    {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }
    //connect to the socket server
    while (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("\nConnection Failed retry in one second \n");

    }
    //char array and pointer to keep track of current command value
    char commandMonitor[BUF_LEN+1];
    char * commandMonitorPtr =strcpy(commandMonitor,"");
    //infinite loop
    while(1) {
        //temp string for building the message that is sent to the socket server
        char lineTemp[BUF_LEN+1] = "";
        //append valid onto the start of the message string so the server can separate our message from others
        char *lineTempPt = strcpy(lineTemp,"VALID,");
        //append the table onto the message
        lineTempPt = strcat(lineTemp,hello);
        lineTempPt = strcat(lineTemp, ",");
        //append the status of all the daemons onto the message string
        for(int i = 0; i < daemons; i++ ){
            FILE * temp = fopen(daemonPaths[i],"r");
            fgets(line, sizeof(line),temp);
            lineTempPt = strcat(lineTemp,line);
            lineTempPt = strcat(lineTemp, ",");
            fflush(temp);
            fclose(temp);
        }
        //append a end onto the message that's sent to the socket server
        lineTempPt = strcat(lineTemp, "END");
        //send the message to the socket server
        send(sock , lineTempPt , strlen(lineTempPt) , 0 );
        //print out of the message that we sent to the remote server
        printf("Information sent: ");
        printf(lineTempPt);
        printf("\n");
        //read in new command info from the server
        valread = read( sock , buffer, BUF_LEN);
        //print out the command that was read in
        printf("\n Data Read In: ");
        printf(buffer);
        printf("\n");
        sleep(1);
        //if we did not read anything, then there is a error with the server
        if(valread <= 0) {

            //print out that we had a error
            printf("Server has gone away");
            printf("\n");
            //close the socket
            close(sock);
            sock = 0;


            //recreate the socket and attempt to recreate every second
            while ((sock = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
                printf("\n Socket creation error \n");
                sleep(1);
            }

            //0 out the server address in memory
            memset(&serv_addr, '0', sizeof(serv_addr));
            //set the socket family and port again since we probably need to change the port
            serv_addr.sin_family = AF_INET;
            serv_addr.sin_port = htons(PORT);

            // attempt to convert the ip address of the server and retry every second
            while(inet_pton(AF_INET, "68.134.4.105", &serv_addr.sin_addr)<=0)
            {
                printf("\nInvalid address/ Address not supported \n");
                sleep(1);
            }
            //attempt to reconnect the socket to the server every second
            while (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
            {
                printf("\nConnection Failed retry in one second \n");
                sleep(1);
            }

        } //end error handling conditional
        //append terminating character onto the end of the buffer
        buffer[valread] = '\0';
        //check whether we have recieved a new command vs the last one
        if(strcmp(commandMonitor,buffer)==0) {
        } else{
            //copy the buffer into the command value
            strcpy(commandMonitor,"");
            strcpy(commandMonitor,buffer);
            //if we recieve a 6 or 7 then we have recieved a command to start one of the daemons. Starting them is done through a bash script so that a new term window is made
            /* Contents of one of the scripts for reference
            #!/bin/bash
            killall SQLDaemon
            DISPLAY=:0 x-terminal-emulator -e /var/www/daemons/SQLDaemon & disown
            exit 0
            */
            //code 6 is to start the sensor daemon
            if(strcmp(buffer,"6") == 0) {
                char * arguments[255] = {"/var/www/daemons/sensor.sh"};
                exec_prog(arguments);
            } else if(strcmp(buffer,"7") == 0){
                char * arguments[255] = {"/var/www/daemons/SQL.sh"};
                exec_prog(arguments);
            }
            //print out the new command code we recieved
            printf("Command code recieved: ");
            printf(buffer);
            printf("\n");
            //print the command code out to file
            commandFile = fopen(commandFilePath,"w");
            fprintf(commandFile,buffer);
            fflush(commandFile);
            fclose(commandFile);
    }
        //clear out the read buffer
        strcpy(buffer,"");
        //wait for one second before sending/recieving stuff from server
        sleep(1);
    } //end infinite loop


    return 0;
}
