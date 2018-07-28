// Client side C/C++ program to demonstrate Socket programming
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#define PORT 8080


//trims off the newlines characters from the fgets read
char * fTrim (char s[]) {
  int i = strlen(s)-1;
  if ((i > 0) && (s[i] == '\n'))
    s[i] = '\0';
  return s;
}

int main(int argc, char const *argv[])
{

//char arrays for storing the contents of the databaseSettings
   char line [255];
   char file [10][255];
   char table[] = "";

//File to store the settings for the database
FILE *plist = fopen("/var/www/databaseSettings", "r");


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
    char buffer[1024] = {0};
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error \n");
        return -1;
    }

    memset(&serv_addr, '0', sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if(inet_pton(AF_INET, "68.134.4.105", &serv_addr.sin_addr)<=0)
    {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    while (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("\nConnection Failed retry in one second \n");

    }

    printf("Hello message sent\n");
    while(1) {
    send(sock , hello , strlen(hello) , 0 );
    connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    valread = read( sock , buffer, 1024);
    if(valread == 0) {
    printf("Server has gone away");
    printf("\n");
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
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
    printf("%s\n",buffer );
    sleep(1);
    }


    return 0;
}
