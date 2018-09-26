#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "xlsxwriter.h"

//function to split a string along a delimeter
//you will need to call free() on the result of this function or else a mem leak will occur
char ** splitString(char * str)
{

    int i;
    int count = 0;
    const char delims[] = ",";
    char *result = NULL;
    char **store = NULL;
    char **tmp = NULL;
    result = strtok(str, delims);
    while (result != NULL)
    {
        free(tmp);
        tmp = malloc(count * sizeof(char *));
        for (i=0; i<count; i++)
        {
            tmp[i] = store[i];
            // printf("%s\n", tmp[i]);
        }
        free(store);
        store = malloc((count + 2) * sizeof(char *));
        for (i=0; i<count; i++)
        {

            store[i] = tmp[i];
        }
        //memcpy( result[i], &result[1], sizeof(result)-1);
        //result[sizeof(result)-1] = '\0';
        store[count] = result;
        count++;
        // printf("%s\n", result);

        result = strtok(NULL, delims);

    }
    free(tmp);
//          tmp = malloc(count * sizeof(char *));
//
//          for (i=0; i<count; i++) {
//
//               tmp[i] = store[i];
//
//             //  memcpy( tmp[i], &tmp[i][1], sizeof(tmp[i]));
//              // tmp[i][strlen(tmp[i])-1] = '\0';
//          }
//          free(store);
//          store = malloc((count + 1) * sizeof(char *));
//          for (i=0; i<count; i++) {
//
//               store[i] = tmp[i];
//               //printf("%s\n",store[i]);
//          }
    //memcpy( result[i], &result[1], sizeof(result)-1);
    //result[sizeof(result)-1] = '\0';

    //put a -1 at the end of store so that we know when to stop looping over it
    store[count] = "-1";
    count++;
    //free(tmp);
    // free(store);
    return store;

}

//trims off the newlines characters from the fgets read
char * fTrim (char s[])
{
    int i = strlen(s)-1;
    if ((i > 0) && (s[i] == '\n'))
        s[i] = '\0';
    return s;
}
char * filename;
int main(int argc, char *argv[])
{

    //this application should be called by a php script with 2 arguments apart from the normal ./filename argument
    //i just put one here since i'm lazy and don't wanna validate
    if(argc > 1)
    {

        filename = argv[1];
    }
    else
    {

        filename = "/var/www/exports/export5b9abd931a614Test";
    }
    //printf("ree");
    clock_t begin = clock();

    FILE * fileptr;
    //buffers to serve as a line in the CSV file, name of the csv file, and name of the xlsx file
    char line[1024];
    char csvFile[1024];
    char xlsxFile[1024];
    //store the filename in the csvFile and xlsxFile buffers and add the extensions onto the end of each
    sprintf(csvFile, "%s%s",filename,".csv");
    sprintf(xlsxFile, "%s%s",filename,".xlsx");
    //open the csv file to read it
    fileptr = fopen(csvFile, "r");
    //our rownum on the current sheet
    int rowNum = 0;
    //the current sheet number
    int sheetNum = 1;
    //options for the workbook
    lxw_workbook_options options;
    //turn constant memory on so that we do not use a ton of ram when writing a very large sheet
    options.constant_memory = 1;
    options.tmpdir = NULL;

    //create the workbook with the xlsxFile path and options that were stated before
    lxw_workbook  *workbook  = workbook_new_opt(xlsxFile,&options);
    //properties that show up in the xlsx file
    lxw_doc_properties properties = {
    .title    = "export",
    .subject  = "Export tideguage data to spreadsheet",
    .author   = "Tideguage",
    .manager  = "Anonymous",
    .company  = "Tideguage",
    .category = "Data Export",
    .keywords = "export,sensor,sensors,tideguage,Tideguage",
    .comments = "Units/Format:\nRTCTemperature: F\nHumidityTemperature: F\nPressureTemperature: F\nHumidity: % Saturation\nHumidityHeatIndex: F\nTransducer: mm\nPressurePressure: Hg/mm\nPressureAlititude: ft\nTemperature1: F\nTemperature2: F\nRTCDataTime: yyyyMMddhhmmss 24 hour local time format",
    .status   = "",
    };
    //set workbook properties to the previously defined properties
    workbook_set_properties(workbook, &properties);
    //current worksheet variable
    lxw_worksheet *worksheet;
    //just get the first line of the csv file for headers
    fgets(line, sizeof(line), fileptr);
    //split the first line so that we get each field individually
    char ** splitLine = splitString(fTrim(line));
    //buffer to store the current sheet name in
    char sheetNameBuf[20];
    //store the name of the current sheet in the sheet name buffer
    sprintf(sheetNameBuf, "Sheet%d", sheetNum);
    //format to use when we are messing with the column widths
    lxw_format *format = workbook_add_format(workbook);
    //add the first worksheet
    worksheet = workbook_add_worksheet(workbook, sheetNameBuf);
    //set columns 0 through 30 on the current worksheet to have a width of 20px
    worksheet_set_column(worksheet,0,30,20,format);
    //increment sheetNumber to what our next sheet will be
    sheetNum++;

    //write the header so the user knows what the column's field is
    for(int i = 0; strcmp(splitLine[i],"-1") != 0 ; i++)
    {
        worksheet_write_string(worksheet,0,i,splitLine[i],NULL);
    }

    free(splitLine);
    //row num will have to start at 1 for the first sheet since the header is the 0 row
    rowNum = 1;

    //The row limit per spreadsheet that is set by the xlsx file format
    int rowLimit = 1048575;

    //go through each line of the settings file
    while (fgets(line, sizeof(line), fileptr))
    {
        //get the current line of the csv and split it along the commas
        splitLine = splitString(fTrim(line));
        //if our sheet is too large, just create a new sheet and start writing to that
        if(rowNum-rowLimit>0)
        {
            //printf("b\n");
            sprintf(sheetNameBuf, "Sheet%d", sheetNum);

            worksheet = workbook_add_worksheet(workbook, sheetNameBuf);
            worksheet_set_column(worksheet,0,30,20,format);
            sheetNum++;
            rowNum=0;
        }


        //C does not have a very good way to get length of a string array, so i just put "-1" as the last index in splitString() of it to tell loops when to stop
        for(int i = 0; strcmp(splitLine[i],"-1") != 0 ; i++)
        {
             //just write every split entry of the CSV file to the worksheet as a number, not a string so that the user can actually manipulate the data as numbers
             worksheet_write_number(worksheet,rowNum,i,atof(splitLine[i]),NULL);

        }
        //free the line since we did some weird memory allocation with it
        free(splitLine);

        rowNum++;
    }


    fclose(fileptr); //close file.

    //close workbook so that we write out the data
    workbook_close(workbook);
    //ending timestamp for debugging purposes
    clock_t end = clock();
    //remove the csv file as we have already created the xlsx so this is not needed anymore
    remove(csvFile);
    //get the time spent on the execution of the program in seconds
    double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
    //print out how long it took to execute the program
    printf("%s %d\n","Execution Time", time_spent);

    return 0;
}
