#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

FILE* configurationFile;
bool printInfo = true;

char info[30];
int producers;
int consumers;
int bufferSize;
int rowLength;
int searchMode;
int secondsToWait;

FILE* fileToRead;


bool configurationFileOpened = false;
bool fileToReadOpened = false;

void printInformation()
{
    if(printInfo)
        printf("%s",info);
}

void cleanBeforeExit()
{
    if(configurationFileOpened)
        fclose(configurationFile);

    if(fileToReadOpened)
        fclose(fileToRead);
}

void openAndAnalyzeConfiguratinFile(int argc, char* argv[])
{
    if(argc < 2)
    {
        sprintf(info, "Error: too few arguments, should be 2.\n");
        printInformation();
        cleanBeforeExit();
        exit(1);
    }

    configurationFile = fopen(argv[1], "r");
    if(configurationFile == NULL)
    {
        sprintf(info, "Error: cannot open %s file.\n",argv[1]);
        printInformation();
        cleanBeforeExit();
        exit(1);
    }
    configurationFileOpened = true;

    char* line;
    size_t len = 0;

    getline(&line, &len, configurationFile);
    producers = strtol(line, NULL, 10);
    if(producers == 0)
    {
        sprintf(info, "Wrong producers number.\n");
        printInformation();
        cleanBeforeExit();
        exit(1);
    }
    sprintf(info, "Producers number: %d\n", producers);
    printInformation();

    getline(&line, &len, configurationFile);
    consumers = strtol(line, NULL, 10);
    if(consumers == 0)
    {
        sprintf(info, "Wrong consumer number.\n");
        printInformation();
        cleanBeforeExit();
        exit(1);
    }
    sprintf(info, "Consumers number: %d\n", consumers);
    printInformation();

    getline(&line, &len, configurationFile);
    bufferSize = strtol(line, NULL, 10);
    if(bufferSize == 0)
    {
        sprintf(info, "Wrong buffer size.\n");
        printInformation();
        cleanBeforeExit();
        exit(1);
    }
    sprintf(info, "Buffer size: %d\n", bufferSize);
    printInformation();

    getline(&line, &len, configurationFile);
    strtok(line, "\n");
    fileToRead = fopen(line, "r");
    if(fileToRead == NULL)
    {
        sprintf(info, "Error: cannot open %s file.\n", line);
        printInformation();
        cleanBeforeExit();
        exit(1);
    }
    fileToReadOpened = true;
    sprintf(info, "Input file: %s\n", line);
    printInformation();

    getline(&line, &len, configurationFile);
    rowLength = strtol(line, NULL, 10);
    if(rowLength == 0)
    {
        sprintf(info, "Wrong row length.\n");
        printInformation();
        cleanBeforeExit();
        exit(1);
    }
    sprintf(info, "Row length: %d\n", rowLength);
    printInformation();

    getline(&line, &len, configurationFile);
    searchMode = strtol(line, NULL, 10);
    if(searchMode < 1 || searchMode > 3)
    {
        sprintf(info, "Wrong search mode, should be 1 -> '<', 2 -> '=', 3 -> '>'.\n");
        printInformation();
        cleanBeforeExit();
        exit(1);
    }
    sprintf(info, "Search mode: %d\n", searchMode);
    printInformation();

    getline(&line, &len, configurationFile);
    int printMode = strtol(line, NULL, 10);

    if(printMode == 1)
    {
        printInfo = false;
        sprintf(info, "Print mode: not allowed\n");
        printInformation();
    } else if(printMode == 2)
    {
        printInfo = true;
        sprintf(info, "Print mode: allowed\n");
        printInformation();
    } else
    {
        sprintf(info, "Wrong print mode, should be 1 - print not allowed or 2 - print allowed.\n");
        printInformation();
        cleanBeforeExit();
        exit(1);
    }

    getline(&line, &len, configurationFile);
    secondsToWait = strtol(line, NULL, 10);
    if(secondsToWait == 0)
    {
        sprintf(info, "Wrong second to wait value.\n");
        printInformation();
        cleanBeforeExit();
        exit(1);
    }
    sprintf(info, "Seconds to wait: %ds\n", secondsToWait);
    printInformation();
    

}

int main(int argc, char* argv[])
{
    openAndAnalyzeConfiguratinFile(argc, argv);
    return 0;
}