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

char line [BUF_LEN] = "";
char file [10][BUF_LEN];
char table[BUF_LEN] = "";
//File to store the settings for the database
FILE *plist = fopen("/var/www/databaseSettings", "r");


    char clientsLoc[] = "/var/www/clients/";
    int daemons = 3;
    char * daemonPaths[3] = {"/var/www/clients/commitDaemon","/var/www/clients/sensorDaemon","/var/www/clients/commands"};
    //FILE * daemonStatusFiles[2] = {fopen("/var/www/clients/commitDaemon","r"),fopen("/var/www/clients/sensorDaemon","r")};
    char commandFilePath[] = "/var/www/clients/commands";
    FILE * commandFile;
//go through each line of the settings file
int i = 0;
while (fgets(line, sizeof(line), plist)) {
    strcpy(file[i], fTrim(line));
    strcpy(table, fTrim(line));
    i++;
}

    struct sockaddr_in address;
    int sock = 0, valread;
    struct sockaddr_in serv_addr;
    char *hello = file[4];
    char buffer[BUF_LEN] = {0};
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error \n");
        return -1;
    }

    memset(&serv_addr, '0', sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert IPv4 and IPv6 addresses from text to binary form
    while(inet_pton(AF_INET, "68.134.4.105", &serv_addr.sin_addr)<=0)
    {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    while (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("\nConnection Failed retry in one second \n");

    }
    char commandMonitor[BUF_LEN];
    char * commandMonitorPtr =strcpy(commandMonitor,"");
    while(1) {

    char lineTemp[BUF_LEN] = "";
    char *lineTempPt = strcpy(lineTemp,hello);
    lineTempPt = strcat(lineTemp, ",");
    for(int i = 0; i < daemons; i++ ){
    FILE * temp = fopen(daemonPaths[i],"r");
    fgets(line, sizeof(line),temp);
    lineTempPt = strcat(lineTemp,line);
    lineTempPt = strcat(lineTemp, ",");
    }
    lineTempPt = strcat(lineTemp, "END");
//    printf(hello);
    send(sock , lineTempPt , strlen(lineTempPt) , 0 );
    printf("Information sent: ");
    printf(lineTempPt);
    printf("\n");
    //connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    valread = read( sock , buffer, BUF_LEN);
    sleep(1);
    if(valread <= 0) {
    printf("Server has gone away");
    printf("\n");
    close(sock);
    while ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
       printf("\n Socket creation error \n");
        sleep(1);
    }
     // Convert IPv4 and IPv6 addresses from text to binary form
    while(inet_pton(AF_INET, "68.134.4.105", &serv_addr.sin_addr)<=0)
    {
       printf("\nInvalid address/ Address not supported \n");
        sleep(1);
    }

     while (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("\nConnection Failed retry in one second \n");
        sleep(1);

    }

    }
    buffer[valread] = '\0';
    if(strcmp(commandMonitor,buffer)==0) {
    } else{
    strcpy(commandMonitor,"");
    strcpy(commandMonitor,buffer);
    if(strcmp(buffer,"6") == 0) {
    /* Contents of one of the scripts for reference
    #!/bin/bash
    DISPLAY=:0 x-terminal-emulator -e /var/www/daemons/SQLDaemon & disown
    exit 0
*/
        char * arguments[255] = {"/var/www/daemons/sensor.sh"};
        exec_prog(arguments);
    } else if(strcmp(buffer,"7") == 0){
        char * arguments[255] = {"/var/www/daemons/SQL.sh"};
        exec_prog(arguments);
    }
    printf("Command code recieved: ");
    printf(buffer);
    printf("\n");
    commandFile = fopen(commandFilePath,"w");
    fprintf(commandFile,buffer);
    fflush(commandFile);
    fclose(commandFile);
    sleep(10);
    }
    strcpy(buffer,"");
    sleep(1);
    }


    return 0;
}
