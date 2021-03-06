/*
DEVELOPMENT HEADER
Author: Jacob Barnett
Version: defined below
Description: Takes the data from the local database table and commits it to a table in the remote database.
END DEVELOPMENT HEADER
*/
#define VERSION "1.0.3b"
#define SQL_TIMEOUT 10

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <netdb.h>

//MYSQL
//#include <my_global.h>
#include <mysql/mysql.h>

#include <pthread.h>



volatile int STOP=0;



//trims off the newlines characters from the fgets read
char * fTrim (char s[])
{
    int i = strlen(s)-1;
    if ((i > 0) && (s[i] == '\n'))
        s[i] = '\0';
    return s;
}

//Status print thread. Separate thread so that the daemon does not slow down the print
void *statusThread(void *vargp)
{
    printf("Remote SQL Update Daemon running... [  ");
    while(1)
    {
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

int is_network_up(char *chkhost, unsigned short chkport)
{
    int sock = -1;
    struct addrinfo * res, *rp;
    int ret = 0;
    char sport[10];
    snprintf(sport, sizeof sport, "%d", chkport);

    struct addrinfo hints = { .ai_socktype=SOCK_STREAM };

    if (getaddrinfo(chkhost, sport, &hints, &res))
    {
        perror("gai");
        return 0;
    }

    for (rp = res; rp && !ret; rp = rp->ai_next)
    {
        sock = socket(rp->ai_family, rp->ai_socktype,
                      rp->ai_protocol);
        if (sock == -1) continue;
        if (connect(sock, rp->ai_addr, rp->ai_addrlen) != -1)
        {
            char node[200], service[100];
            getnameinfo(res->ai_addr, res->ai_addrlen, node, sizeof node, service, sizeof
                        service, NI_NUMERICHOST);

            //printf("Success on %s, %s\n", node, service);
            ret = 1;                  /* Success */
        }
        close(sock);
    }
    freeaddrinfo(res);

    return ret;
}
struct arg_struct
{
    char * ip ;
    int port ;
    char * statusFilePath ;
};
void *databaseUpCheck(void *vargp)
{
    struct arg_struct *args = (struct arg_struct *)vargp;
    while(1)
    {
        int isUp = is_network_up(args->ip,args->port);
        if(isUp==0)
        {
//printf("thing\n");
            FILE * statusFile = fopen(args->statusFilePath,"w");
            fprintf(statusFile,"CANNOT_CONNECT_TO_DB");
            fflush(statusFile);
            fclose(statusFile);
        }
        sleep(5);
    }

}

main(int argc, char** argv)
{


    //argv[1] = "debug";
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    printf("Version: ");
    printf(VERSION);
    printf("\n");
    printf("Daemon Started On: %d-%d-%d %d:%d:%d\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

//printf("num args \n");
//printf("%d",argc);
//printf("\n");
//printf(argv[0]);
//printf("\n");

    int DEBUG = 0;

    char status[1023] = "";
    char *statusPt;


    const char * commandsFilePath = "/var/www/clients/commands";
    FILE * commandsFile = fopen(commandsFilePath,"w");
    fprintf(commandsFile, "1");
    fclose(commandsFile);
    commandsFile = fopen(commandsFilePath,"r");
    const char * stopCode = "0";

    const char * statusFilePath = "/var/www/clients/commitDaemon";
    FILE * statusFile = fopen(statusFilePath,"w");

    fprintf(statusFile, "STARTING");
    fflush(statusFile);
    fclose(statusFile);
    //system("/usr/bin/x-terminal-emulator -e /var/www/Daemons/UpdateDaemon &");

    //enum of all fields in the database
    enum fields {Key, ID, SVer, HVer, Time = 4,RTemp=5,PTemp=6,PPress=7,PAlt,Temp1,Temp2,Humidity,HumidityTemp,HumidityHeatIndex,Transducer};



    //char arrays for storing the contents of the databaseSettings
    char line [255];
    char file2 [10][255];
    char file [10][255];
    char table[] = "";

    //File to store the settings for the database
    FILE *plist = fopen("/var/www/databaseSettings", "r");


    //go through each line of the settings file
    int i = 0;
    while (fgets(line, sizeof(line), plist))
    {
        strcpy(file[i], fTrim(line));
        strcpy(file2[i], fTrim(line));
        strcpy(table, fTrim(line));
        i++;
    }


    mysql_library_init(0,NULL,NULL);
    //connection for the remote database
    MYSQL * con = mysql_init(con);
    if(!con)
    {
        //exit(0);
    }
    unsigned int sqlTimeout = 10;
    mysql_options(con,MYSQL_OPT_COMPRESS,1);
    //exit(0);
    //connection for the local database
    MYSQL *localCon = mysql_init(localCon);
    //mysql_options(localCon,MYSQL_OPT_CONNECT_TIMEOUT,&sqlTimeout);
    //mysql_options(localCon,MYSQL_OPT_READ_TIMEOUT,&sqlTimeout);
    //mysql_options(localCon,MYSQL_OPT_WRITE_TIMEOUT,&sqlTimeout);
    mysql_ssl_set(con,"/var/www/certs/mysql/client-key.pem","/var/www/certs/mysql/client-cert.pem","/var/www/certs/mysql/ca.pem",NULL,NULL);
    //mysql_options(con,MYSQL_OPT_CONNECT_TIMEOUT,&sqlTimeout);
    //mysql_options(con,MYSQL_OPT_WRITE_TIMEOUT,&sqlTimeout);
    //mysql_options(con,MYSQL_READ_DEFAULT_FILE,"/var/www/daemons/my.cnf");
    //printf("%s\n",mysql_error(con));
    if (con == NULL)
    {
        if(DEBUG)
        {
            fprintf(stderr, "%s\n", mysql_error(con));
        }
        //exit(1);
    }
    // mysql_options(con,MYSQL_OPT_COMPRESS,1);

    statusFile = fopen(statusFilePath,"w");
    fprintf(statusFile,"CONNECT_REMOTE_DB");
    fflush(statusFile);
    fclose(statusFile);

    //create the connection to the remote database
    while (!mysql_real_connect(con, file[0], file[1], file[2],file[3], 0, NULL, 0))
    {
        //finish_with_error(con);
        //mysql_close(con);
        //printf("AAAAA\n");
    }

    struct arg_struct args;
    args.ip = file[0];
    args.port = 3306;
    args.statusFilePath = statusFilePath;
    pthread_t *dbThread_id = malloc(sizeof(pthread_t));
    pthread_create(dbThread_id,NULL,databaseUpCheck,(void *)&args);
    statusFile = fopen(statusFilePath,"w");
    fprintf(statusFile,"CONNECT_LOCAL_DB");
    fflush(statusFile);
    fclose(statusFile);
    //create the connection to the local database
    while (!mysql_real_connect(localCon, "localhost", file[1], file[2],file[3], 0, NULL, 0))
    {
        //mysql_close(localCon);
        //finish_with_error(localCon);
    }

    //unsigned long thing = mysql_thread_id(con);
    // int flags = fcntl(mysql_get_socket(con),F_GETFL,0);
    // fcntl(mysql_get_socket(con),F_SETFL,flags | O_NONBLOCK);
    printf("Connected to database with encryption: ");
    printf("%s\n",mysql_get_ssl_cipher(con));

    //id for the status print thread
    pthread_t *thread_id = malloc(sizeof(pthread_t));
    if(argc > 1)
    {
        if(strcmp(argv[1],"debug")==0)
        {
            DEBUG = 1;


        }
    }
    else if(DEBUG == 0)
    {
        //pthread_yield();
        pthread_create(thread_id,NULL,statusThread,NULL);

        //pthread_create(&thread_id, NULL, statusThread, NULL);
        //pthread_join(thread_id, NULL);

    }

//for(int i = 0; i < localFields; i++)
//{
//  printf("Field %u is %s\n", i, fields[i].name);
//}
//exit(0);
//int n = snprintf(NULL,0,"%lu",mysql_thread_id(con));
//assert(n>0);
//char buffer[n+1];
//int c = snprintf(buffer,n+1,"%lu",mysql_thread_id(con));
//assert(buffer[n]=='\0');
//assert(c==n);
//printf(buffer);
//printf("\n");
//printf(mysql_stat(con));
//printf("\n");
    //char array to store the query that checks to see if the table we want exists
    char checkQuery[255];
    //pointer to the check query string so that i can print it out without seg faulting
    char *checkQueryPtr;
    //build the check query string
    checkQueryPtr = strcpy(checkQuery,"SHOW TABLES LIKE '");
    checkQueryPtr = strcat(checkQuery, file2[4]);
    checkQueryPtr = strcat(checkQuery, "';");
    //the result variable for the table check
    MYSQL_RES *confresCheck;
    //query the remote database for whether the table is present or not
    statusFile = fopen(statusFilePath,"w");
    fprintf(statusFile,"CHECK_TABLE_EXISTS");
    fflush(statusFile);
    fclose(statusFile);
    while(mysql_query(con, checkQuery))
    {
        if(DEBUG)
        {
            //if we have a error with the database we most likely lost connection, so attempt to reestablish connection every 1 second. Do nothing if the query works
            fprintf(stderr, "%s\n", mysql_error(con));
        }
        mysql_real_connect(con, file[0], file[1], file[2],file[3], 0, NULL, 0);
        sleep(1);
    }

    //store the result of the table present check in the result variable
    confresCheck = mysql_store_result(con);

    if(DEBUG)
    {
        //print out the query for the table check
        printf("\n --------------------------- TABLE EXISTS CHECK ---------------------\n \n");
        printf(checkQueryPtr);
        printf("\n \n --------------------------- TABLE EXISTS CHECK ---------------------\n \n");


        printf(" \n --------------------------- DOES TABLE EXIST? ---------------------\n \n");
    }
    //if the result of the table check query is not empty, then the table exists.
    if(mysql_num_rows(confresCheck) != 0)
    {
        if(DEBUG)
        {
            printf("Table exists, continuing \n");
        }
        mysql_free_result(confresCheck);
    }
    else    //of the result of the table check query is empty, then the table does not exist and we have to create the table on the remote DB.
    {

        if(DEBUG)
        {

            printf("Table does not exist, creating table");
            printf("\n");
        }
        //string to store the table creation query
        char tableQuery[102400];
        char *tableQueryPt = tableQuery;
        //build the table creation query
        tableQueryPt += sprintf(tableQueryPt, "CREATE TABLE ");
        //strcpy(tableQuery, "CREATE TABLE (");
        tableQueryPt += sprintf(tableQueryPt, file2[4]);
        tableQueryPt += sprintf(tableQueryPt, " (");
        unsigned int localFields;
        MYSQL_FIELD *fields;
        MYSQL_RES * fieldRes;
        mysql_query(localCon,"DESCRIBE DataTable;");
        fieldRes = mysql_store_result(localCon);
        unsigned int localFieldNum = mysql_num_rows(fieldRes);
//printf("\n%d", localFieldNum);
        char ** field;
        unsigned int currentField = 0;
        //sleep(2);
        while(field = mysql_fetch_row(fieldRes))
        {
            tableQueryPt += sprintf(tableQueryPt, "%s ", field[0]);
            tableQueryPt += sprintf(tableQueryPt, "%s ", field[1]);
            tableQueryPt += sprintf(tableQueryPt, "%s ", (strcmp(field[2],"YES")==0)?"NULL":"NOT NULL");
            tableQueryPt += sprintf(tableQueryPt, "%s ",(strcmp(field[3],"")!=0) ? (strcmp(field[3],"PRI")==0)?"PRIMARY":"UNIQUE" : "NULL");
            char defValue[255] = "DEFAULT ";
            tableQueryPt += sprintf(tableQueryPt, "%s ", (field[4]==NULL)?"NULL":strcat(defValue,field[4]));
            tableQueryPt += sprintf(tableQueryPt, "%s", field[5]);
            if(currentField < localFieldNum-1)
            {
                tableQueryPt += sprintf(tableQueryPt, ",");
            }
            currentField++;
        }
        tableQueryPt += sprintf(tableQueryPt,");");
        mysql_free_result(fieldRes);
        // printf("%s",tableQuery);

//strcat(tableQuery, " (TransmissionKey VARCHAR(50), SystemID VARCHAR(10), SoftwareVersion VARCHAR(50), HardwareVersion VARCHAR(50), RTCDataTime VARCHAR(45), RTCTemperature DECIMAL(10,2), PressureTemperature DECIMAL(10,2), PressurePressure DECIMAL(10,2), PressureAlititude DECIMAL(10,2), Temperature1 DECIMAL(10,2), Temperature2 DECIMAL(10,2), Humidity DECIMAL(10,2), HumidityTemperature DECIMAL(10,2), HumidityHeatIndex DECIMAL(10,2), Transducer INT(11)); ");
        if(DEBUG)
        {
            printf("%s",tableQuery);

        }
        //tell the database to create the table
        statusFile = fopen(statusFilePath,"w");
        fprintf(statusFile,"CREATE_NEW_TABLE");
        fflush(statusFile);
        fclose(statusFile);
        mysql_query(con, tableQuery);
        //exit(0);
    }
    if(DEBUG)
    {
        printf("\n --------------------------- DOES TABLE EXIST? ---------------------\n \n");
    }

    //the string that is the query to insert into the remote database
    char inQuery[255];
    //build the insert query string
    strcpy(inQuery, "INSERT INTO ");
    char * tempF = "";
    strcat(inQuery,file2[4]);
    strcat(inQuery," VALUES ");


    //string for the query to get the last line of the remote database
    char lastQuery[1023] = "";
    //build last line remote query
    strcat(lastQuery,"SELECT * FROM ( SELECT * FROM ");
    strcat(lastQuery,file2[4]);
    strcat(lastQuery, " ORDER BY RTCDataTime DESC LIMIT 1) sub ORDER BY RTCDataTime ASC;");
    //running row from database from the results of the query
    char ** row;
    //the last time in the remote database
    char lastTime[255] = "0";
    //the result variable for the last time in the remote database
    MYSQL_RES *confresTime;

    if(DEBUG)
    {
        printf("\n --------------------------- LAST ROW QUERY ---------------------\n \n");
        printf(lastQuery);
        printf("\n \n --------------------------- LAST ROW QUERY ---------------------\n \n");
    }
    //poll the last line of the remote database
    statusFile = fopen(statusFilePath,"w");
    fprintf(statusFile,"GET_LAST_UPDATE");
    fflush(statusFile);
    fclose(statusFile);
    while(mysql_query(con, lastQuery))
    {
        if(DEBUG)
        {
            //if we have a error with the database we most likely lost connection, so attempt to reestablish connection every 1 second. Do nothing if the query works
            fprintf(stderr, "%s\n", mysql_error(con));
        }
        while(!mysql_real_connect(con, file[0], file[1], file[2],file[3], 0, NULL, 0)) {
        sleep(1);
        }
        sleep(1);
    }






    //store the result of the last line query
    confresTime = mysql_store_result(con);


    //if there is a row with a time string, store it in the last time variable, otherwise the last time variable will just be ""
    while (row = mysql_fetch_row(confresTime))
    {
        strcat(lastTime,row[Time]);
    }

    if(DEBUG)
    {
        //print the last time from the remote database
        printf("\n");
        printf(lastTime);
        printf("\n");
    }
    //free up the memory associated with the result after we are done with it
    mysql_free_result(confresTime);

    //result variable for the running request to the remote database
    MYSQL_RES *confres;

    //variable to store the number of fields in the database
    int numfields;

    //query all rows with time keys not in the remote db
    char catchupQuery[1023] = "SELECT * FROM DataTable WHERE RTCDataTime > ";
    strcat(catchupQuery,lastTime);
    strcat(catchupQuery,";");
    if(DEBUG)
    {
        printf("\n");
        printf(catchupQuery);
        printf("\n");
    }
    //get all rows from the remote database
    statusFile = fopen(statusFilePath,"w");
    fprintf(statusFile,"GET_NEW_ROWS");
    fflush(statusFile);
    fclose(statusFile);
    mysql_query(localCon,catchupQuery);



    //get all the rows of the remote database
    confres = mysql_store_result(localCon);

    //get the number of fields in the remote database
    numfields = mysql_num_fields(confres);

    //insert query alternative char pointer/array
    char *command;
    //big freaking char array for inserting into the database
    char inQueryTemp[1500000]="";

    //flag to tell whether the connection was interrupted
    int connectionInterrupted = 0;

    int newData = 0; //is there new data that needs to be inserted into the remote database?

    //basically infinite loop
    while(!STOP)
    {
        strcpy(line,"");
        commandsFile = fopen(commandsFilePath, "r");
        fgets(line, sizeof(line), commandsFile);
        fflush(commandsFile);
        fclose(commandsFile);
        strcpy(line,fTrim(line));
        statusFile = fopen(statusFilePath,"w");
        fprintf(statusFile,"RUNNING_UPDATES");
        fflush(statusFile);
        fclose(statusFile);
//printf("\n");
//printf(line);
//printf("\n");
        //if the first line of this file is 0, then this daemon has been signaled to be killed but is still running, while 1 is currently running, and 2 is dead and not running, and 3 is signaled to be started
        if(strcmp(line,stopCode) == 0)
        {

            commandsFile = fopen(commandsFile, "w");
            statusFile = fopen(statusFilePath,"w");
            fprintf(statusFile,"KILLED");
            fflush(statusFile);
            fclose(statusFile);
            //signal that the daemon has been killed
            fprintf(commandsFile,"2");
            //fflush(continueRunning);
            fclose(commandsFile);
            exit(1);
        }

        //reset the connection interrupted and new data variables at the start of the loop
        connectionInterrupted = 0;
        newData = 0;

        //start building the start of the insert query
        command = strcpy(inQueryTemp,"");
        command = strcat(inQueryTemp,inQuery);

        //running counter keeping track of the current row
        int rowCounter = 0;

        //read each row from the result of query to the local database
        while(row = mysql_fetch_row(confres))
        {



            //if we are looking at a row with a newer timestamp than the last one, then build the insert query
            if(strcmp(row[Time],lastTime) > 0)
            {

                //if there is a newer timestamp in the local database vs the remote database, then we have new data to insert
                newData = 1;

                //each inserted row is separated by a comma, but the first row in the query does not have a comma
                if(rowCounter > 0)
                {
                    command = strcat(inQueryTemp, ",");
                }
                command = strcat(inQueryTemp, "(");

                //update the last time value
                strcpy(lastTime,row[Time]);

                //build the insert query for each field
                for(int i = 0; i < numfields; i++)
                {

                    //the type of the field is a string, so we have to append a quote at the start of the field
                    if(i==Key || i==Time)
                    {
                        command = strcat(inQueryTemp,"'");
                    }
                    //append the returned contents of the field
                    command = strcat(inQueryTemp,row[i]);

                    //if the field is the key or time, then we have to put a quote since it's of the type string
                    if(i==Key || i==Time)
                    {
                        //if it is not the last field then we appened a quote and comma
                        if(i<(numfields-1))
                        {
                            command = strcat(inQueryTemp,"',");
                        }
                        else
                        {

                            command = strcat(inQueryTemp,"'");
                        }

                    }
                    else
                    {
                        //append a comma onto the end of the string if it is not the last field in the string
                        if(i<(numfields-1))
                        {
                            command = strcat(inQueryTemp,",");
                        }

                    }

                }

                //increment the row counter for every row
                rowCounter++;


                //put a enclosing paren at the end of the row insert
                command = strcat(inQueryTemp, ")");


                //if we have a insert command that is getting too long we have to just do the insert and reset the string length
                if(strlen(inQueryTemp) >= 1400000)
                {
                    //reset the row counter to 0 and newData to false because we are working on building a new query
                    rowCounter = 0;
                    newData = 0;

                    //append the ending semicolon onto the query
                    command = strcat(inQueryTemp, ";");
                    if(DEBUG)
                    {
                        //print out the query
                        printf("\n");
                        printf(command);
                        printf("\n");
                    }

                    //send the query to the the server and wait until the server is ready to recieve the query
                    while(mysql_query(con, command))
                    {
                        connectionInterrupted = 1;
                        if(DEBUG)
                        {
                            //if we have a error with the database we most likely lost connection, so attempt to reestablish connection every 1 second. Do nothing if the query works
                            fprintf(stderr, "%s\n", mysql_error(con));

                        }
                        statusFile = fopen(statusFilePath,"w");
                        char tempStatus[] = "REMOTE_SQL_ERROR:";
                        char *tempStatusPt = strcat(tempStatus,mysql_error(con));
                        fprintf(statusFile,tempStatusPt);
                        fflush(statusFile);
                        fclose(statusFile);
                        // mysql_close(con);
                        //mysql_close(localCon);
                        //con = mysql_init(con);
                        //localCon = mysql_init(NULL);
                        while(!mysql_real_connect(con, file[0], file[1], file[2],file[3], 0, NULL, 0))
                        {
                            sleep(1);
                        }
                        // while(!mysql_real_connect(localCon, "localhost", file[1], file[2],file[3], 0, NULL, 0)) {
                        // sleep(1);
                        // }
                        sleep(1);
                    }
                    statusFile = fopen(statusFilePath,"w");
                    char tempStatus[] = "RUNNING UPDATE";
                    fprintf(statusFile,tempStatus);
                    fflush(statusFile);
                    fclose(statusFile);
                    //reset the command and query strings
                    command = strcpy(inQueryTemp,"");
                    strcpy(inQueryTemp, "");
                    command = strcat(inQueryTemp,inQuery);


                }


            }


        }
        //append the ending semicolon onto the end of the query string
        command = strcat(inQueryTemp, ";");

        //free memory associated with the remote query
        mysql_free_result(confres);

        //if we have new rows to insert, then insert those, otherwise we don't really do anything
        if(newData==1)
        {
            if(DEBUG)
            {
                printf("\n");
                printf(command);
                printf("\n");
            }
            //query the server to insert the rows with same fault tolerance in case we lose connection
            while(mysql_query(con, command))
            {
                connectionInterrupted = 1;
                if(DEBUG)
                {
                    //if we have a error with the database we most likely lost connection, so attempt to reestablish connection every 1 second. Do nothing if the query works
                    fprintf(stderr, "%s\n", mysql_error(con));
                }
                statusFile = fopen(statusFilePath,"w");
                char tempStatus[] = "REMOTE_SQL_ERROR: ";
                char *tempStatusPt = strcat(tempStatus,mysql_error(con));
                fprintf(statusFile,tempStatusPt);
                fflush(statusFile);
                fclose(statusFile);
                //mysql_close(con);
//                mysql_close(localCon);
//                con = mysql_init(con);
//                localCon = mysql_init(NULL);
                while(!mysql_real_connect(con, file[0], file[1], file[2],file[3], 0, NULL, 0))
                {
                    sleep(1);
                }
//                while(!mysql_real_connect(localCon, "localhost", file[1], file[2],file[3], 0, NULL, 0)){
//                sleep(1);
//                }
                sleep(1);
            }

        }

        //commit the inserts to the remote database
//mysql_query(con, "COMMIT;");
        sleep(1);

        char runningQuery[1023] = "SELECT * FROM DataTable WHERE RTCDataTime > ";
        if(lastTime!=NULL)
        {
            strcat(runningQuery,lastTime);
        }
        else
        {


        }
        //query and store for the next last 90 rows of the local database
        if(connectionInterrupted == 0 )
        {
            if(mysql_query(localCon,runningQuery))
            {

                //if we fail to query the first time we most likely lost connection, so change the query to selecting all lines(since downtime varies) and attempt to reconnect
                while(mysql_query(localCon,runningQuery))
                {
                    if(DEBUG)
                    {
                        //if we have a error with the database we most likely lost connection, so attempt to reestablish connection every 1 second. Do nothing if the query works
                        fprintf(stderr, "%s\n", mysql_error(con));
                        //mysql_dump_debug_info(con);
                    }
//                    mysql_close(con);
//                    mysql_close(localCon);
//                    con = mysql_init(con);
//                    localCon = mysql_init(NULL);
                    while(!mysql_real_connect(con, file[0], file[1], file[2],file[3], 0, NULL, 0))
                    {
                        sleep(1);
                    }
//                    mysql_real_connect(localCon, "localhost", file[1], file[2],file[3], 0, NULL, 0);
                    sleep(1);
                }
            }
        }
        else     //if the connection was interrupted then we have to poll all lines of the local database to accomidate for a unknown downtime
        {
            if(DEBUG)
            {
                printf("\n");
                printf("The connection to remote server was broken so we have to repoll all lines from the local db");
            }
            //since the connection can be broken for a indeterminable amount of time, we need to get all rows from the local database again
            mysql_query(localCon,runningQuery);
        }
        //store the result of the local DB query
        confres = mysql_store_result(localCon);
        strcpy(inQueryTemp, "");
        statusFile = fopen(statusFilePath,"w");
        fprintf(statusFile,"RUNNING_UPDATE");
        fflush(statusFile);
        fclose(statusFile);
    }
    return 0;
}



void finish_with_error(MYSQL *con)
{
    fprintf(stderr, "%s\n", mysql_error(con));
    mysql_close(con);

    exit(1);
}





