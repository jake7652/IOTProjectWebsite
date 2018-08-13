// Client side C/C++ program to demonstrate Socket programming
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/wait.h>
#include <resolv.h>
#include <netdb.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <sys/types.h>
#include <signal.h>
#include <dirent.h>
#include <ctype.h>
#include <sys/reboot.h>

#define PORT 8080
#define BUF_LEN 2048

pid_t pID(const char* name) {
	pid_t p = -1;
	size_t i, j;
	char* s = (char*)malloc(264);
	char buf[128];
	FILE* st;
	DIR* d = opendir("/proc");
	if (d == NULL) { free(s); return -1; }
	struct dirent* f;
	while ((f = readdir(d)) != NULL) {
		if (f->d_name[0] == '.') continue;
		for (i = 0; isdigit(f->d_name[i]); i++);
		if (i < strlen(f->d_name)) continue;
		strcpy(s, "/proc/");
		strcat(s, f->d_name);
		strcat(s, "/status");
		st = fopen(s, "r");
		if (st == NULL) { closedir(d); free(s); return -1; }
		do {
			if (fgets(buf, 128, st) == NULL) { fclose(st); closedir(d); free(s); return -1; }
		} while (strncmp(buf, "Name:", 5));
		fclose(st);
		for (j = 5; isspace(buf[j]); j++);
		*strchr(buf, '\n') = 0;
		if (!strcmp(&(buf[j]), name)) {
			sscanf(&(s[6]), "%d", &p);
			//kill(p, SIGKILL);
		}
	}
	closedir(d);
	free(s);
	return p;
}


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

/*---------------------------------------------------------------------*/
/*--- InitCTX - initialize the SSL engine.                          ---*/
/*---------------------------------------------------------------------*/
SSL_CTX* InitCTX(void)
{   SSL_METHOD *method;
    SSL_CTX *ctx;

    OpenSSL_add_all_algorithms();		/* Load cryptos, et.al. */
    SSL_load_error_strings();			/* Bring in and register error messages */
    method = SSLv23_client_method();		/* Create new client-method instance */
    ctx = SSL_CTX_new(method);			/* Create new context */
    if ( ctx == NULL )
    {
        ERR_print_errors_fp(stderr);
        abort();
    }
    return ctx;
}

int main(int argc, char const *argv[])
{
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    char timeString[30];
    strftime(timeString, sizeof(timeString), "%Y-%m-%d %H:%M:%S %Z", &tm);
    printf("Daemon Started On: %d-%d-%d %d:%d:%d\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
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
    char * daemonNames[3] = {"SQLDaemon","SensorDaemon","commands"};
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
    while ((sock = socket(PF_INET, SOCK_STREAM, 0)) < 0)
    {
       // printf("\n Socket creation error \n");
       // return -1;
    }
    // Convert IPv4 and IPv6 addresses from text to binary form
    while(inet_pton(AF_INET, "68.134.4.105", &serv_addr.sin_addr)<=0)
    {
       // printf("\nInvalid address/ Address not supported \n");
       // return -1;
    }
    //connect to the socket server
    while(connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
       // printf("\nConnection Failed retry in one second \n");

    }
    //char array and pointer to keep track of current command value
    char commandMonitor[BUF_LEN+1];
    char * commandMonitorPtr =strcpy(commandMonitor,"-1");
    SSL_CTX *ctx;
    SSL *ssl;
    ctx = InitCTX();
    ssl = SSL_new(ctx);
    SSL_set_fd(ssl, sock);
    SSL_connect(ssl);
    printf("Connection Encryption: %s \n \n", SSL_get_cipher(ssl));
    //hello = "DataTable";
    //infinite loop
    int loopnum = 1;
    while(1) {
        printf("--------------------------------- loop number %d ---------------------------------- \n",loopnum);
        //temp string for building the message that is sent to the socket server
        char lineTemp[BUF_LEN+1] = "";
        //append valid onto the start of the message string so the server can separate our message from others
        char *lineTempPt = strcpy(lineTemp,"VALID,");
        //append the table onto the message
        lineTempPt = strcat(lineTemp,hello);
        lineTempPt = strcat(lineTemp, ",");
        //append the status of all the daemons onto the message string
        for(int i = 0; i < daemons; i++ ){
            if(pID(daemonNames[i]) != -1 || i == daemons-1) {
                FILE * temp = fopen(daemonPaths[i],"r");
                fgets(line, sizeof(line),temp);
                lineTempPt = strcat(lineTemp,line);
                lineTempPt = strcat(lineTemp, ",");
                fflush(temp);
                fclose(temp);
            } else {
                lineTempPt = strcat(lineTemp,"KILLED");
                lineTempPt = strcat(lineTemp, ",");
            }
        }
        //append a end onto the message that's sent to the socket server
        lineTempPt = strcat(lineTemp, "END");
        //send the message to the socket server
        SSL_write(ssl,lineTempPt , strlen(lineTempPt));
        //print out of the message that we sent to the remote server
        printf("Information sent: ");
        printf(lineTempPt);
        printf("\n");
        //read in new command info from the server
        valread = SSL_read( ssl , buffer, BUF_LEN);
        //print out the command that was read in
        printf("Data Read In: ");
        printf(buffer);
        printf("\n");
        sleep(1);

        //if we did not read anything, then there is a error with the server
        if(valread <= 0) {
            SSL_free(ssl);
            //print out that we had a error
            printf("Connection Status: ");
            printf("Bad Connection \n");
            //close the socket
            close(sock);
            sock = 0;


            //recreate the socket and attempt to recreate every second
            while ((sock = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
                //printf("\n Socket creation error \n");
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
                //printf("\nInvalid address/ Address not supported \n");
                sleep(1);
            }

            //attempt to reconnect the socket to the server every second
            while (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
            {
                //printf("\nConnection Failed retry in one second \n");

                sleep(1);
            }
            ssl = SSL_new(ctx);
            SSL_set_fd(ssl, sock);
            SSL_connect(ssl);
            //printf("Connected with %s encryption\n", SSL_get_cipher(ssl));

        } else {//end error handling conditional
        printf("Connection Status: Good Connection \n");

        }
        //append terminating character onto the end of the buffer
        buffer[valread] = '\0';

        //check whether we have recieved a new command vs the last one or this is the first command we have recieved since the daemon started
        if(strcmp(commandMonitor,buffer)==0 || strcmp(commandMonitor,"-1")==0 || valread<=0) {

            strcpy(commandMonitor,"");
            strcpy(commandMonitor,buffer);
        } else{
            //copy the buffer into the command value
            strcpy(commandMonitor,"");
            strcpy(commandMonitor,buffer);

            //10 is a command to reboot
            if(strcmp(buffer,"10")==0) {
                sync();
                reboot(RB_AUTOBOOT);
            }

            //if the recieved command is 0 or 3 or 7 or 6 then we kill the daemon
            //kill SQL daemon
            if(strcmp(buffer,"0") == 0 || strcmp(buffer,"7") == 0) {
                pid_t id = pID(daemonNames[0]);
                if(id!=-1) {
                    kill(id,SIGKILL);
                }
            }
            //kill sensor daemon
            else if(strcmp(buffer,"3") == 0 || strcmp(buffer,"6") == 0){
                pid_t id = pID(daemonNames[1]);
                if(id!=-1) {
                    kill(id,SIGKILL);
                }
            }


            //if we recieve a 6 or 7 then we have recieved a command to start one of the daemons. Starting them is done through a bash script so that a new term window is made
            /* Contents of one of the scripts for reference
            #!/bin/bash
            killall SQLDaemon
            DISPLAY=:0 x-terminal-emulator -e /var/www/daemons/SQLDaemon & disown
            exit 0
            */
            //code 6 is to start the sensor daemon
            if(strcmp(buffer,"6") == 0) {
                exec_prog(sensorArgs);
            } else if(strcmp(buffer,"7") == 0){
                exec_prog(sqlArgs);
            }
            //print out the new command code we recieved
          //  printf("Command code recieved: ");
          //  printf(buffer);
          //  printf("\n");
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
        printf("\r                                                      ");
        printf("\r");
        printf("\033[A");

        printf("\r                                                      ");
        printf("\r");
        printf("\033[A");

        printf("\r                                                      ");
        printf("\r");
        printf("\033[A");

        printf("\r                                                      ");
        printf("\r");
        printf("\033[A");
        printf("\r                                                      ");
        printf("\r");
        printf("\033[A");
        printf("\r                                                                     ");
        printf("\r");
        printf("\n");
        loopnum++;
    } //end infinite loop


    return 0;
}
