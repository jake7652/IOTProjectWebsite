/*
DEVELOPMENT HEADER
Author: Jacob Barnett
Version: defined below
Description: Takes the data from the arduino attached to this machine and inserts it into the local database.
END DEVELOPMENT HEADER
*/
#define VERSION "1.0.1b"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

//MYSQL
//#include </home/pi/Development/Daemon Source/TideGuageDaemon201806191426/my_global.h>
#include <mysql/mysql.h>

#include <pthread.h>

//Modem Parameters
#define BAUDRATE B9600
#define MODEMDEVICE "/dev/ttyUSB0" /* Note rfcomm0 is Bluetooth SSP */
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

//SensorArray Start and Stop Characters
#define PACKETHEADER1 '$' //Tide Guage Header Packet
#define PACKETHEADER2 '*' //Tide Guage Data Packet
#define PACKETTAIL '@'

volatile int STOP=FALSE;

struct SQLStatementComponets
    {
        int StatementType;//1:SELECT, 2:INSERT, 3: DELETE
        char SelectClause[255];
        char TableName[255];
        char StatementData[255];
        char WhereClause[255];
        char SQLStatement[1024];
    };

struct SQLStatementComponets *pSQLStatementComponets;
struct SQLStatementComponets SQL;

//Function Headers
void finish_with_error(MYSQL *con);
void BuildSQLStatement(struct SQLStatementComponets x);


    char keystr[] = "0,4,1,1,1,4,2,2,2,2,2,2,2,2,2,1,99";
    char *keystrtoken;
    char *keystrptr = keystr;

    char queryheader []="INSERT INTO DataTable VALUES (";
    char querytail [] = ");";

//trims off the newlines characters from the fgets read
char * fTrim (char s[]) {
  int i = strlen(s)-1;
  if ((i > 0) && (s[i] == '\n'))
    s[i] = '\0';
  return s;
}

//Status print thread. Separate thread so that the daemon does not slow down the print
void *statusThread(void *vargp)
{
    printf("Sensor Update Daemon Running... [  ");
    while(1) {
    printf("\b\b|]");
    fflush(stdout);
    usleep(90000);
    printf("\b\b/]");
    fflush(stdout);
    usleep(90000);
    printf("\b\b-]");
    fflush(stdout);
    usleep(90000);
    printf("\b\b\\]");
    fflush(stdout);
    usleep(90000);
    fflush(stdout);
    }

    return NULL;
}


main(int argc, char ** argv)
{
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    printf("Version: " );
    printf(VERSION);
    printf("\n");
    printf("Daemon Started On: %d-%d-%d %d:%d:%d\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
    int DEBUG = 0;
    //id for the status print thread
    pthread_t *thread_id = malloc(sizeof(pthread_t));
    if(argc > 1) {
        if(strcmp(argv[1],"debug")==0) {
            DEBUG = 1;


        }
    } else {
        //pthread_yield();
        pthread_create(thread_id,NULL,statusThread,NULL);
        //pthread_create(&thread_id, NULL, statusThread, NULL);
        //pthread_join(thread_id, NULL);

    }

    const char * commandsFilePath = "/var/www/clients/commands";
    FILE * commandsFile = fopen(commandsFilePath,"w");
    fprintf(commandsFile, "4");
    fclose(commandsFile);
    commandsFile = fopen(commandsFilePath,"r");
    const  char *stopCode = "3";

    const char * statusFilePath = "/var/www/clients/sensorDaemon";
    FILE * statusFile = fopen(statusFilePath,"w");

    fprintf(statusFile, "STARTING");
    fflush(statusFile);
    fclose(statusFile);
    //Terminal Variables
    int fd,c, res, lennum;
    long tcounter,gcounter,bcounter;
    struct termios oldtio,newtio;
    char buf[255];
    int ValidPacket;

/*
    //SQLStatementBuilderVariables

    pSQLStatementComponets = &SQL;

    //SQLStatementBuilderInitialization
    SQL.StatementType=3;
    strcpy(SQL.SelectClause,"*");
    strcpy(SQL.TableName,"DataTable");
    strcpy(SQL.StatementData,"(1,\"1.023\")");
    strcpy(SQL.WhereClause,"1=1");
    strcpy(SQL.SQLStatement,"");

    //Call to SQLStatmentBuilderFunction
    BuildSQLStatement(SQL);
*/

    char querypart1 []="INSERT INTO DataTable VALUES (";
    char querypart2 []=",";
    char querypart3 []=")";
    //char querypart3 []=")\"";
    char querybuf[255];

    //char querybuf[255]="\"INSERT INTO DataTable VALUES (";
    char scounter[25];



// Setup Modem Connection
    /* Open Modem device for reading and writing and not as controlling tty
       because we don't want to get killed if linenoise sends CTRL-C */
    fd = open(MODEMDEVICE, O_RDWR | O_NOCTTY );
//if we are having problems opening the port at the start then we keep trying until it's open
    while(fd<0) {
    close(fd);
    sleep(1);
    fd = open(MODEMDEVICE, O_RDWR | O_NOCTTY );
    if(DEBUG == 1) {
    perror(MODEMDEVICE);
    }
    char error[255] = "";
    strcat(error,MODEMDEVICE);
    strcat(error, ": ");
    char * errorPt = strcat(error,strerror(errno));
    statusFile = fopen(statusFilePath,"w");
    fprintf(statusFile, errorPt);
    fflush(statusFile);
    fclose(statusFile);
    }
    statusFile = fopen(statusFilePath,"w");
    fprintf(statusFile, "NORMAL_UPDATES");
    fflush(statusFile);
    fclose(statusFile);

    /* save current port settings */
    tcgetattr(fd,&oldtio);

    /* Clear struct for new port settings */
    bzero(&newtio, sizeof(newtio));

    //BaudRate, ,8N1,Local Connection, No Modem Control, Enable receiving characters
    newtio.c_cflag = BAUDRATE | CRTSCTS | CS8 | CLOCAL | CREAD;

    //Ignore Bytes with Parity Errors
    newtio.c_iflag = IGNPAR;

    //Raw Output
    newtio.c_oflag = 0;

    /* set input mode (non-canonical, no echo,...) */
    //newtio.c_lflag = 0; //NON-CANCONICAL
    newtio.c_lflag = ICANON;//CANCONICAL

    //Inter-character timer unused
    newtio.c_cc[VTIME]    = 0;

    //Block read until # chars received
    newtio.c_cc[VMIN]     = 1;

    //Clean modem line and activate the settings for the port
    tcflush(fd, TCIOFLUSH);
    tcsetattr(fd,TCSANOW,&newtio);

//Open and setup database

    MYSQL *con = mysql_init(NULL);

    if (con == NULL)
        {
            if(DEBUG == 1) {
                fprintf(stderr, "%s\n", mysql_error(con));
            }
            exit(1);
        }

    if (mysql_real_connect(con, "localhost", "Tide", "MasterWater2025","TideGuage", 0, NULL, 0) == NULL)
 		{
      		finish_with_error(con);
  		}

    tcounter, gcounter, bcounter = 0;

//Terminal Loop
    while (STOP==FALSE)
        {
statusFile = fopen(statusFilePath,"w");
fprintf(statusFile, "NORMAL_UPDATES");
fflush(statusFile);
fclose(statusFile);
char line [255] = "";
commandsFile = fopen(commandsFilePath, "r");
fgets(line, sizeof(line), commandsFile);
fflush(commandsFile);
fclose(commandsFile);
strcpy(line,fTrim(line));
if(strcmp(line,stopCode)==0) {
    commandsFile = fopen(commandsFilePath,"w");
    fprintf(commandsFile, "5");
    fclose(commandsFile);
    statusFile = fopen(statusFilePath,"w");
    fprintf(statusFile, "KILLED");
    fflush(statusFile);
    fclose(statusFile);

    exit(0);



}


        /* loop forever input */
        res = read(fd,buf,255);   /* returns after 1 chars have been input */


        buf[res]=0;               /* so we can printf... */
        //lennum = res;
        ValidPacket=FALSE;

        if(buf[0] == PACKETHEADER1 && buf[res-3] == PACKETTAIL)
            {

                //XX="INSERT INTO DataTab"
                // res is the length of the packet

                ValidPacket=TRUE;
                if(DEBUG == 1) {
                    printf("\n%s<+>%d<+>%d\n","Good Tide Guage Header",res, gcounter);
                    printf("\n%s\n", buf);
                }
                querybuf[0]='\0';
                strcat(querybuf,querypart1);
                if(DEBUG == 1) {
                    sprintf(scounter,"%d",gcounter);
                }
                strcat(querybuf,scounter);
                strcat(querybuf,",");
                strcat(querybuf,"\"");
                strcat(querybuf,buf);
                strcat(querybuf,"\")");
                if(DEBUG == 1) {
                    printf("%s\n",querybuf);
                }
                //Execute SQL INSERT
                //if (mysql_query(con, querybuf))
                //   {
                //      finish_with_error(con);
                //   }
                if(DEBUG == 1) {
                    printf("%s\n","++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++");
                }
                gcounter++;
            }


        if(buf[0] == PACKETHEADER2 && buf[res-3] == PACKETTAIL)
            {

                //XX="INSERT INTO DataTab"
                ValidPacket=TRUE;
                if(DEBUG == 1) {
                    printf("\n%s<+>%d<+>%d\n","Good Tide Guage Data",res, gcounter);
                    printf("\n%s\n", buf);
                }
                //querybuf[0]='\0';
                //strcat(querybuf,querypart1);
                //sprintf(scounter,"%d",gcounter);
                //strcat(querybuf,scounter);
                //strcat(querybuf,",");
                //strcat(querybuf,"\"");
                //strcat(querybuf,buf);
                //strcat(querybuf,"\")");

                //printf("%s\n",querybuf);

                if(DEBUG == 1) {
                    printf("%s\n","xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx");

                }
                char datainstr[255] = "";
                char *datainstrtoken;
                char *datainstrptr = datainstr;
                int datainstrcounter =0;

                char keystr[] = "0,4,1,1,1,4,2,2,2,2,2,2,2,2,2,1,99";
                char *keystrtoken;
                char *keystrptr = keystr;
                int keystrcounter=0;

                char *ptr;
                char dataoutstr[255] = "";

                strcat(datainstr,buf); //Assign buffer to be processed

                while ((datainstrtoken = strtok_r(datainstrptr, ",", &datainstrptr)) && (keystrtoken = strtok_r(keystrptr, ",", &keystrptr)))
                    {
                        if (atoi(keystrtoken) == 0) //Header
                        {
                            //Do nothing to strip out character
                        }
                        if (atoi(keystrtoken) == 1) //Integer
                        {
                            if (datainstrcounter != 1) {ptr = strcat (dataoutstr, ",");}
                                ptr = strcat (dataoutstr,datainstrtoken);
                        }
                        if (atoi(keystrtoken) == 2) //Float / Decimal
                        {
                            if (datainstrcounter != 1) {ptr = strcat (dataoutstr, ",");}
                                ptr = strcat (dataoutstr,datainstrtoken);
                        }
                        if (atoi(keystrtoken) == 3) //String / VarChar
                        {
                            if (datainstrcounter != 1) {ptr = strcat (dataoutstr, ",");}
                                ptr = strcat (dataoutstr, "'");
                                ptr = strcat (dataoutstr,datainstrtoken);
                                ptr = strcat (dataoutstr, "'");
                        }
                        if (atoi(keystrtoken) == 4) //DateTime
                        {
                            if (datainstrcounter != 1) {ptr = strcat (dataoutstr, ",");}
                                ptr = strcat (dataoutstr, "'");
                                ptr = strcat (dataoutstr,datainstrtoken);
                                ptr = strcat (dataoutstr, "'");
                        }
                        if (atoi(keystrtoken) == 99) //Tail
                        {
                            //Do nothing to strip out character
                        }

                        datainstrcounter++;
                        keystrcounter++;
                    }
                if(DEBUG == 1) {
                    printf("%s\n",dataoutstr);printf("\n");
                }
                //Build SQL Statment
                char sqlstatement [255] ="";
                ptr = strcat (sqlstatement,queryheader);
                ptr = strcat (sqlstatement,dataoutstr);
                ptr = strcat (sqlstatement,querytail);
                if(DEBUG == 1) {
                    printf("%s\n",sqlstatement);
                    printf("\n");printf("\n");
                }

                //Execute SQL INSERT
                if (mysql_query(con, sqlstatement))
                   {
                      finish_with_error(con);
                   }

                if (mysql_query(con, "COMMIT;"))
                   {
                      finish_with_error(con);
                   }
                if(DEBUG == 1) {
                    printf("%s\n","++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++");
                }
                gcounter++;

            }
        if (ValidPacket == FALSE)
            {
                //close the device
                close(fd);
                //might as well give it a second to close
                sleep(1);
                //attempt to reopen the usb device since we most likely lost connection to it
                fd = open(MODEMDEVICE, O_RDWR | O_NOCTTY );
                //set the attributes again
                tcsetattr(fd,TCSANOW,&newtio);
                if(DEBUG == 1) {
                    //print out the error on the usb device
                    perror(MODEMDEVICE);
                }
                char error[255] = "";
                strcat(error,MODEMDEVICE);
                strcat(error, ": ");
                char * errorPt = strcat(error,strerror(errno));
                statusFile = fopen(statusFilePath,"w");
                fprintf(statusFile, errorPt);
                fflush(statusFile);
                fclose(statusFile);
                //print out that it was bad and some other stuff
                if(DEBUG == 1) {
                    printf("%s\n","Bad");
                    printf("<+>%d<+>%d\n", res, bcounter);
                    printf("%s\n","------------------------------------------------------------");
                }
                bcounter++;
            }
        tcounter++;                /* Increment count read since start */

        }
    tcsetattr(fd,TCSANOW,&oldtio);
}
//  mysql_close(con);
//exit(0);

//    return 0;


void finish_with_error(MYSQL *con)
    {
		fprintf(stderr, "%s\n", mysql_error(con));
  		mysql_close(con);
  		exit(1);
    }



