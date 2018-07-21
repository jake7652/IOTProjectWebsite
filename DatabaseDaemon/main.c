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

 //  free(line);

 //connection for the remote database
    MYSQL *con = mysql_init(NULL);
//connection for the local database
    MYSQL *localCon = mysql_init(NULL);

    if (con == NULL)
        {
            fprintf(stderr, "%s\n", mysql_error(con));
            exit(1);
        }
//create the connection to the remote database
    if (mysql_real_connect(con, file[0], file[1], file[2],file[3], 0, NULL, 1) == NULL)
 		{
      		finish_with_error(con);
  		}

//create the connection to the local database
    if (mysql_real_connect(localCon, "localhost", file[1], file[2],file[3], 3306, NULL, 0) == NULL)
 		{
      		finish_with_error(localCon);
  		}


char checkQuery[255];
strcpy(checkQuery,"SHOW TABLES LIKE '");
strcat(checkQuery, file2[4]);
strcat(checkQuery, "';");
MYSQL_RES *confresCheck;
while(mysql_query(con, checkQuery)) {
//if we have a error with the database we most likely lost connection, so attempt to reestablish connection every 1 second. Do nothing if the query works
fprintf(stderr, "%s\n", mysql_error(con));
mysql_real_connect(con, file[0], file[1], file[2],file[3], 0, NULL, 1);
sleep(1);
}
char ** checkRow;
confresCheck = mysql_store_result(con);
printf(checkQuery);
printf("\n");

if(mysql_num_rows(confresCheck) != 0) {
printf("Table exists, continuing \n");
} else {
printf("Table does not exist, creating table");
printf("\n");
char tableQuery[2047];
strcpy(tableQuery, "CREATE TABLE ");
strcat(tableQuery, file2[4]);
strcat(tableQuery, " (TransmissionKey VARCHAR(50), SystemID VARCHAR(10), SoftwareVersion VARCHAR(50), HardwareVersion VARCHAR(50), RTCDataTime VARCHAR(45), RTCTemperature DECIMAL(10,2), PressureTemperature DECIMAL(10,2), PressurePressure DECIMAL(10,2), PressureAlititude DECIMAL(10,2), Temperature1 DECIMAL(10,2), Temperature2 DECIMAL(10,2), Humidity DECIMAL(10,2), HumidityTemperature DECIMAL(10,2), HumidityHeatIndex DECIMAL(10,2), Transducer INT(11)); ");
printf(tableQuery);
mysql_query(con, tableQuery);

}
//exit(0);
//the string that is the query to insert into the database
char inQuery[255];
//build the insert query string
strcpy(inQuery, "INSERT INTO ");
char * tempF = "";
strcat(inQuery,file2[4]);
strcat(inQuery," VALUES ");
//strcat(inQuery," (");

printf("\n");
printf(inQuery);
printf("\n");

//string for the running query to poll data from the local database
char query[1023]="";

//build the local query string
char *queryPT;
strcat(query,"SELECT * FROM ( SELECT * FROM ");
printf("\n");

strcat(query,"DataTable");
strcat(query, " ORDER BY RTCDataTime DESC LIMIT 90) sub ORDER BY RTCDataTime ASC;");

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



printf("\n");
printf(lastQuery);

//poll the last line of the remote database
while(mysql_query(con, lastQuery)) {
//if we have a error with the database we most likely lost connection, so attempt to reestablish connection every 1 second. Do nothing if the query works
fprintf(stderr, "%s\n", mysql_error(con));
mysql_real_connect(con, file[0], file[1], file[2],file[3], 0, NULL, 1);
sleep(1);
}






//store the result of the last line query
confresTime = mysql_store_result(con);

//if there is a row with a time string, store it in the last time variable, otherwise the last time variable will just be ""
while (row = mysql_fetch_row(confresTime)){
strcat(lastTime,row[Time]);
}

//print the last time from the remote database
printf("\n");
printf(lastTime);

//result variable for the running request to the remote database
MYSQL_RES *confres;

//variable to store the number of fields in the database
int numfields;




int firstLoop = 1;

//get all rows from the remote database
mysql_query(localCon,"SELECT * FROM DataTable");

//get all the rows of the remote database
confres = mysql_store_result(localCon);

//get the number of fields in the remote database
numfields = mysql_num_fields(confres);




char * val;

//basically infinite loop
while(!STOP) {

bool newData = false; //is there new data that needs to be inserted?
//insert query alternative char pointer/array
char *command;
//big freaking char array for inserting into the database
char inQueryTemp[1500000]="";
//start building the start of the insert query
command = strcpy(inQueryTemp,"");
command = strcat(inQueryTemp,inQuery);
int rowCounter = 0;
//read each row from the result of query to the local database
while(row = mysql_fetch_row(confres)) {



//if we are looking at a row with a newer timestamp than the last one, then build the insert query
if(strcmp(row[Time],lastTime) > 0) {
newData = true;
if(rowCounter > 0) {
command = strcat(inQueryTemp, ",");
}
command = strcat(inQueryTemp, "(");
//update the last time value
strcpy(lastTime,row[Time]);


//build the insert query for each field
for(int i = 0; i < numfields; i++) {

if(i==Key || i==Time) {
command = strcat(inQueryTemp,"'");
}

command = strcat(inQueryTemp,row[i]);

if(i==Key || i==Time) {

if(i<(numfields-1)){
command = strcat(inQueryTemp,"',");
}

} else {

if(i<(numfields-1)){
command = strcat(inQueryTemp,",");
}

}

}


//end building the insert query
//printf("\n");
//printf(strlen(command));
//printf("\n");
//printf(command);
rowCounter++;



command = strcat(inQueryTemp, ")");




//inserthe values into the remote database

//strcpy(inQueryTemp,"");

}


}
command = strcat(inQueryTemp, ";");
//flag to tell whether the connection was interrupted
bool connectionInterrupted = false;

//if we have new rows to insert, then insert those, otherwise we don't really do anything
if(newData==true) {
printf("\n");
printf(command);
printf("\n");
while(mysql_query(con, command)) {
connectionInterrupted = true;
//if we have a error with the database we most likely lost connection, so attempt to reestablish connection every 1 second. Do nothing if the query works
fprintf(stderr, "%s\n", mysql_error(con));
mysql_real_connect(con, file[0], file[1], file[2],file[3], 0, NULL, 1);
mysql_real_connect(localCon, "localhost", file[1], file[2],file[3], 0, NULL, 1);
sleep(1);
}

}

//commit the inserts to the remote database
//mysql_query(con, "COMMIT;");
sleep(1);
//query and store for the next last 90 rows of the local database
if(connectionInterrupted == false ) {
if(mysql_query(localCon,query)) {
//if we fail to query the first time we most likely lost connection, so change the query to selecting all lines(since downtime varies) and attempt to reconnect
while(mysql_query(localCon,"SELECT * FROM DataTable")) {
//if we have a error with the database we most likely lost connection, so attempt to reestablish connection every 1 second. Do nothing if the query works
fprintf(stderr, "%s\n", mysql_error(con));
mysql_real_connect(con, file[0], file[1], file[2],file[3], 0, NULL, 1);
mysql_real_connect(localCon, "localhost", file[1], file[2],file[3], 0, NULL, 1);
sleep(1);
}
}
} else {
printf("\n");
printf("The connection to remote server was broken so we have to repoll all lines from the local db");
//since the connection can be broken for a indeterminable amount of time, we need to get all rows from the local database again
mysql_query(localCon,"SELECT * FROM DataTable");
}

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


