
//#include <sys/types.h>
//#include <sys/stat.h>
//#include <fcntl.h>
//#include <termios.h>
//#include <stdio.h>
#include <stdio.h>
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


//the string that is the query to insert into the database
char inQuery[255];
//build the insert query string
strcpy(inQuery, "INSERT INTO ");
char * tempF = "";
strcat(inQuery,file2[4]);
strcat(inQuery," VALUES");
strcat(inQuery," (");

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
strcat(query, " ORDER BY RTCDataTime DESC LIMIT 10) sub ORDER BY RTCDataTime ASC;");

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
if(mysql_query(con, lastQuery)) {
fprintf(stderr, "%s\n", mysql_error(con));
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

//read each row from the result of query to the local database
while(row = mysql_fetch_row(confres)) {



//if we are looking at a row with a newer timestamp than the last one, then build the insert query
if(strcmp(row[Time],lastTime) > 0) {

//update the last time value
strcpy(lastTime,row[Time]);
//insert query alternative char pointer/array
char *command;
//creat the insert query char array.
char inQueryTemp[255]="";

//start building the start of the insert query
command = strcpy(inQueryTemp,"");
command = strcat(inQueryTemp,inQuery);

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
command = strcat(inQueryTemp,");");
printf("\n");
printf(command);
//inserthe values into the remote database
if(mysql_query(con, command)) {
fprintf(stderr, "%s\n", mysql_error(con));

}
strcpy(inQueryTemp,"");
}


}
//commit the inserts to the remote database
mysql_query(con, "COMMIT;");
sleep(1);
//query and store for the next last 10 rows of the remote database
mysql_query(localCon,query);
confres = mysql_store_result(localCon);
}
return 0;
}



void finish_with_error(MYSQL *con)
    {
		fprintf(stderr, "%s\n", mysql_error(con));
  		mysql_close(con);

  		exit(1);
    }


