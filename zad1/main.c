#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <pthread.h>
#include <sys/syscall.h>
#include <unistd.h>


FILE* configurationFile;
bool printInfo = true;

char info[30];
int producers;
int consumers;
int bufferSize;
int rowLength;
int searchMode;
int secondsToWait;

int producerPosition;
int consumerPosition;

FILE* fileToRead;

char** buffer;
int cellsFilled = 0;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
pthread_t *producerThreads;
pthread_t *consumerThreads;
bool allThreadsCreated = false;

bool canReadBuffer = false;

int* queue;

bool configurationFileOpened = false;
bool fileToReadOpened = false;
bool bufferCreated = false;
bool producerThreadsCreated = false;
bool consumersThreadsCreated = false;
bool mutexInitialized = false;
bool queueCreated = false;

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

    if(bufferCreated)
    {
        for(int i = 0;i < bufferSize; i++)
            free(buffer[i]);
        free(buffer);
    }

    if(producerThreadsCreated)
        free(producerThreads);

    if(consumersThreadsCreated)
        free(consumerThreads);

    if(mutexInitialized)
    {
        pthread_mutex_destroy(&mutex);
        pthread_cond_destroy(&cond);
    }

    if(queueCreated)
        free(queue);
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

long gettid(){
    return syscall(SYS_gettid);
}

void* producerAction(void* arg)
{  
    while(!allThreadsCreated);

    sprintf(info, "Producers starts writing to buffer.\n");
    printInformation();

    char* line;
    size_t len = 0;
    while(getline(&line, &len, fileToRead))
    {
        int position = producerPosition;
        buffer[position] = line;
        cellsFilled++;
        sprintf(info, "Producers %ld has written \n %sin buffer.\n", gettid(), line);
        printInformation();

        position++;
        if(position > bufferSize)
            position = 0;

        producerPosition = position;
    }
    canReadBuffer = true;
    sprintf(info, "Producers %ld end his work.\n", gettid());
    printInformation();
    return NULL;
}

void* consumerAction(void* arg)
{
    while(!allThreadsCreated);

    printf("Consumer %ld starts reading from buffer.\n",gettid());
    while(canReadBuffer && cellsFilled != 0)
    {
        bool found = false;
        int position = consumerPosition;
        char* line = buffer[position];
        buffer[position] = NULL;
        cellsFilled--;
        int length = strlen(line);

        if(searchMode == 1)
        {
            if(length < rowLength)
                found = true;
        } 
        else if(searchMode == 2)
        {
            if(length == rowLength)
                found = true;
        } 
        else if(searchMode == 3)
        {
            if(length > rowLength)
                found == true;
        }

        if(found)
            printf("Consumer %ld has found proper line \n[%d]: %s", gettid(), position, line);

        position++;
        if(position > bufferSize)
            position = 0;

        consumerPosition = position;
    }
    printf("Consumer %ld ends his work.\n", gettid());
    return NULL;
}

int main(int argc, char* argv[])
{
    openAndAnalyzeConfiguratinFile(argc, argv);

    buffer = malloc(bufferSize * sizeof(void*));
    bufferCreated = true;
    producerPosition = 0;
    consumerPosition = 0;

    if (pthread_mutex_init(&mutex, NULL) != 0)
    {
        sprintf(info, "Error: cannot initialize mutex.\n");
        printInformation();
        cleanBeforeExit();
        exit(1);
    }
    mutexInitialized = true;

    producerThreads = malloc(producers * sizeof(pthread_t));
    if(producerThreads == NULL)
    {
        sprintf(info, "Error: cannot allocate memory for producers..\n");
        printInformation();
        cleanBeforeExit();
        exit(1);
    }
    consumerThreads = malloc(consumers * sizeof(pthread_t));
    if(consumerThreads == NULL)
    {
        sprintf(info, "Error: cannot allocate memory for consumers.\n");
        printInformation();
        cleanBeforeExit();
        exit(1);
    }

    queue = malloc((consumers + producers) * sizeof(int));
    queueCreated = true;

    for(int i = 0; i < producers + consumers; i++)
        queue[i] = -1;

    for(int i = 0; i < consumers; i++)
    {
        int created = pthread_create(&consumerThreads[i], NULL, consumerAction, NULL);
        if(created != 0)
        {
            sprintf(info, "Error: cannot create consumer thread.\n");
            printInformation();
            cleanBeforeExit();
            exit(1);
        }
    }

    for(int i = 0; i < producers; i++)
    {
        int created = pthread_create(&producerThreads[i], NULL, producerAction, NULL);
        if(created != 0)
        {
            sprintf(info, "Error: cannot create producer thread.\n");
            printInformation();
            cleanBeforeExit();
            exit(1);
        }
    }

    allThreadsCreated = true;

    for(int i = 0; i < consumers; i++)
    {
        int joined = pthread_join(consumerThreads[i], NULL);
        if(joined != 0)
        {
            sprintf(info, "Error: cannot join consumer thread.\n");
            printInformation();
            cleanBeforeExit();
            exit(1);
        }
    }

    for(int i = 0; i < producers; i++)
    {
        int joined = pthread_join(producerThreads[i], NULL);
        if(joined != 0)
        {
            sprintf(info, "Error: cannot join producer thread.\n");
            printInformation();
            cleanBeforeExit();
            exit(1);
        }
    }

    cleanBeforeExit();
    return 0;
}