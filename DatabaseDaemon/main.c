
//#include <sys/types.h>
//#include <sys/stat.h>
//#include <fcntl.h>
//#include <termios.h>
//#include <stdio.h>
#include <stdio.h>
#include <string.h>
//#include <stdlib.h>




//MYSQL
//#include <my_global.h>
#include <mysql.h>

//Modem Parameters
#define BAUDRATE B9600
#define MODEMDEVICE "/dev/ttyUSB0" /* Note rfcomm0 is Bluetooth SSP */
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

typedef int bool;
#define true 1
#define false 0

volatile int STOP=FALSE;



//trims off the newlines characters from the fgets read
char * fTrim (char s[]) {
  int i = strlen(s)-1;
  if ((i > 0) && (s[i] == '\n'))
    s[i] = '\0';
  return s;
}

main()
{


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
while (fgets(line, sizeof(line), plist)) {
    strcpy(file[i], fTrim(line));
    strcpy(file2[i], fTrim(line));
    strcpy(table, fTrim(line));
    i++;
}


 //connection for the remote database
    MYSQL con;
    mysql_init(&con);


//connection for the local database
    MYSQL *localCon = mysql_init(NULL);

    if (&con == NULL)
        {
            fprintf(stderr, "%s\n", mysql_error(&con));
            exit(1);
        }
        mysql_options(&con,MYSQL_OPT_COMPRESS,1);

//create the connection to the remote database
    if (mysql_real_connect(&con, file[0], file[1], file[2],file[3], 0, NULL, 1) == NULL)
 		{
      		finish_with_error(&con);
  		}

//create the connection to the local database
    if (mysql_real_connect(localCon, "localhost", file[1], file[2],file[3], 3306, NULL, 0) == NULL)
 		{
      		finish_with_error(localCon);
  		}


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
while(mysql_query(&con, checkQuery)) {
//if we have a error with the database we most likely lost connection, so attempt to reestablish connection every 1 second. Do nothing if the query works
fprintf(stderr, "%s\n", mysql_error(&con));
mysql_real_connect(&con, file[0], file[1], file[2],file[3], 0, NULL, 1);
sleep(1);
}

//store the result of the table present check in the result variable
confresCheck = mysql_store_result(&con);

//print out the query for the table check
printf("\n --------------------------- TABLE EXISTS CHECK ---------------------\n \n");
printf(checkQueryPtr);
printf("\n \n --------------------------- TABLE EXISTS CHECK ---------------------\n \n");


printf(" \n --------------------------- DOES TABLE EXIST? ---------------------\n \n");
//if the result of the table check query is not empty, then the table exists.
if(mysql_num_rows(confresCheck) != 0) {
printf("Table exists, continuing \n");
mysql_free_result(confresCheck);
} else {//of the result of the table check query is empty, then the table does not exist and we have to create the table on the remote DB.
printf("Table does not exist, creating table");
printf("\n");
//string to store the table creation query
char tableQuery[2047];
//build the table creation query
strcpy(tableQuery, "CREATE TABLE ");
strcat(tableQuery, file2[4]);
strcat(tableQuery, " (TransmissionKey VARCHAR(50), SystemID VARCHAR(10), SoftwareVersion VARCHAR(50), HardwareVersion VARCHAR(50), RTCDataTime VARCHAR(45), RTCTemperature DECIMAL(10,2), PressureTemperature DECIMAL(10,2), PressurePressure DECIMAL(10,2), PressureAlititude DECIMAL(10,2), Temperature1 DECIMAL(10,2), Temperature2 DECIMAL(10,2), Humidity DECIMAL(10,2), HumidityTemperature DECIMAL(10,2), HumidityHeatIndex DECIMAL(10,2), Transducer INT(11)); ");
printf(tableQuery);
//tell the database to create the table
mysql_query(&con, tableQuery);

}

printf("\n --------------------------- DOES TABLE EXIST? ---------------------\n \n");


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
char lastTime[255] = "";
//the result variable for the last time in the remote database
MYSQL_RES *confresTime;


printf("\n --------------------------- LAST ROW QUERY ---------------------\n \n");
printf(lastQuery);
printf("\n \n --------------------------- LAST ROW QUERY ---------------------\n \n");

//poll the last line of the remote database
while(mysql_query(&con, lastQuery)) {
//if we have a error with the database we most likely lost connection, so attempt to reestablish connection every 1 second. Do nothing if the query works
fprintf(stderr, "%s\n", mysql_error(&con));
mysql_close(&con);
mysql_real_connect(&con, file[0], file[1], file[2],file[3], 0, NULL, 1);
sleep(1);
}






//store the result of the last line query
confresTime = mysql_store_result(&con);


//if there is a row with a time string, store it in the last time variable, otherwise the last time variable will just be ""
while (row = mysql_fetch_row(confresTime)){
strcat(lastTime,row[Time]);
}


//print the last time from the remote database
printf("\n");
printf(lastTime);
printf("\n");
//free up the memory associated with the result after we are done with it
mysql_free_result(confresTime);

//result variable for the running request to the remote database
MYSQL_RES *confres;

//variable to store the number of fields in the database
int numfields;

//query all rows with time keys not in the remote db
char catchupQuery[1023] = "SELECT * FROM DataTable WHERE RTCDataTime > ";
strcat(catchupQuery,lastTime);

printf("\n");
printf(catchupQuery);
printf("\n");
//get all rows from the remote database
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
bool connectionInterrupted = false;

bool newData = false; //is there new data that needs to be inserted into the remote database?

//basically infinite loop
while(!STOP) {

printf("\n");
printf("\n");

fprintf(stderr, "%s\n", mysql_error(&con));

//reset the connection interrupted and new data variables at the start of the loop
connectionInterrupted = false;
newData = false;

//start building the start of the insert query
command = strcpy(inQueryTemp,"");
command = strcat(inQueryTemp,inQuery);

//running counter keeping track of the current row
int rowCounter = 0;

//read each row from the result of query to the local database
while(row = mysql_fetch_row(confres)) {



//if we are looking at a row with a newer timestamp than the last one, then build the insert query
if(strcmp(row[Time],lastTime) > 0) {


//if there is a newer timestamp in the local database vs the remote database, then we have new data to insert
newData = true;

//each inserted row is separated by a comma, but the first row in the query does not have a comma
if(rowCounter > 0) {
command = strcat(inQueryTemp, ",");
}
command = strcat(inQueryTemp, "(");

//update the last time value
strcpy(lastTime,row[Time]);

//build the insert query for each field
for(int i = 0; i < numfields; i++) {

//the type of the field is a string, so we have to append a quote at the start of the field
if(i==Key || i==Time) {
command = strcat(inQueryTemp,"'");
}
//append the returned contents of the field
command = strcat(inQueryTemp,row[i]);

//if the field is the key or time, then we have to put a quote since it's of the type string
if(i==Key || i==Time) {
//if it is not the last field then we appened a quote and comma
if(i<(numfields-1)){
command = strcat(inQueryTemp,"',");
} else {

command = strcat(inQueryTemp,"'");
}

} else {
//append a comma onto the end of the string if it is not the last field in the string
if(i<(numfields-1)){
command = strcat(inQueryTemp,",");
}

}

}

//increment the row counter for every row
rowCounter++;


//put a enclosing paren at the end of the row insert
command = strcat(inQueryTemp, ")");


//if we have a insert command that is getting too long we have to just do the insert and reset the string length
if(strlen(inQueryTemp) >= 1400000) {
//reset the row counter to 0 and newData to false because we are working on building a new query
rowCounter = 0;
newData = false;

//append the ending semicolon onto the query
command = strcat(inQueryTemp, ";");

//print out the query
printf("\n");
printf(command);
printf("\n");


//send the query to the the server and wait until the server is ready to recieve the query
while(mysql_query(&con, command)) {
connectionInterrupted = true;
//if we have a error with the database we most likely lost connection, so attempt to reestablish connection every 1 second. Do nothing if the query works
fprintf(stderr, "%s\n", mysql_error(&con));

mysql_close(&con);
mysql_close(localCon);
mysql_init(&con);
localCon = mysql_init(NULL);
mysql_real_connect(&con, file[0], file[1], file[2],file[3], 0, NULL, 1);
mysql_real_connect(localCon, "localhost", file[1], file[2],file[3], 0, NULL, 1);
sleep(1);
}

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
if(newData==true) {
printf("\n");
printf(command);
printf("\n");
//query the server to insert the rows with same fault tolerance in case we lose connection
while(mysql_query(&con, command)) {
connectionInterrupted = true;
//if we have a error with the database we most likely lost connection, so attempt to reestablish connection every 1 second. Do nothing if the query works
fprintf(stderr, "%s\n", mysql_error(&con));
mysql_close(&con);
mysql_close(localCon);
mysql_init(&con);
localCon = mysql_init(NULL);
mysql_real_connect(&con, file[0], file[1], file[2],file[3], 0, NULL, 1);
mysql_real_connect(localCon, "localhost", file[1], file[2],file[3], 0, NULL, 1);
sleep(1);
}

}

//commit the inserts to the remote database
//mysql_query(con, "COMMIT;");
sleep(1);

char runningQuery[1023] = "SELECT * FROM DataTable WHERE RTCDataTime > ";
strcat(runningQuery,lastTime);

//query and store for the next last 90 rows of the local database
if(connectionInterrupted == false ) {
if(mysql_query(localCon,runningQuery)) {

//if we fail to query the first time we most likely lost connection, so change the query to selecting all lines(since downtime varies) and attempt to reconnect
while(mysql_query(localCon,runningQuery)) {
//if we have a error with the database we most likely lost connection, so attempt to reestablish connection every 1 second. Do nothing if the query works
fprintf(stderr, "%s\n", mysql_error(&con));
mysql_close(&con);
mysql_close(localCon);
mysql_init(&con);
localCon = mysql_init(NULL);
mysql_real_connect(&con, file[0], file[1], file[2],file[3], 0, NULL, 1);
mysql_real_connect(localCon, "localhost", file[1], file[2],file[3], 0, NULL, 1);
sleep(1);
}
}
} else { //if the connection was interrupted then we have to poll all lines of the local database to accomidate for a unknown downtime
printf("\n");
printf("The connection to remote server was broken so we have to repoll all lines from the local db");
//since the connection can be broken for a indeterminable amount of time, we need to get all rows from the local database again
mysql_query(localCon,runningQuery);
}
//store the result of the local DB query
confres = mysql_store_result(localCon);
strcpy(inQueryTemp, "");
}
return 0;
}



void finish_with_error(MYSQL *con)
    {
		fprintf(stderr, "%s\n", mysql_error(con));
  		mysql_close(con);

  		exit(1);
    }





