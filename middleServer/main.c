#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>

void* agentsHandler (void * );
void* agentListener (void * );
void* serverHandler (void * );
struct agentMessage parseAgentMessage (char* );
char* createServerMessage(char* , char* ,char* , char* ,char*  , char* );
void* listenToServer (void *);


struct agentInfo
{
    int sockfd;
    pthread_t thread ;
    char name[50];
};
struct agentMessage
{
    char name[50];
    char mgName[50];
    char mgContent[50];
    char mgTimestamp[50];
};

int agentCount = 0;
int finish = 0 ;
struct agentInfo agents[200];
struct agentMessage messages[200][4];
int recievedMessages [200] = {0};
int sendMessages[200] = {0};
struct hostent *server;
int portAgents = 0,portServer = 0;

void error(char *msg)
{
    perror(msg);
    exit(1);
}
int main(int argc, char *argv[])
{

    if (argc < 4)
    {
        fprintf(stderr,"usage %s ListenPortNumber ServerIpAddress ServerPortNumber\n", argv[0]);
        exit(0);
    }
    portAgents = atoi(argv[1]);
    server = gethostbyname(argv[2]);
    portServer = atoi(argv[3]);

    finish = 0;
    pthread_t th1;
    pthread_t th2;
    pthread_create(&th1,0,serverHandler,0);
    pthread_create(&th2,0,agentsHandler,0);

    printf(" type stop to stopping the process : ");
    while (1)
    {
        char str[20] ;
        scanf("%s",str);
        if ( strcmp(str,"stop")== 0 )
        {
            finish = 1;
            break;
        }
        else
            printf("undefined command, type stop to stopping the process : ");
    }
    pthread_cancel(th2);
    return 0;
}

void* agentListener (void * arg)
{
    int agentNumber = *(int *) arg ;
    int i = 0 ;
    char buffer[512];
    int first = 1 ;
    int sockfd = agents[agentNumber].sockfd;
    while (1)
    {
        if (finish == 1 )
            break;
        else
        {
            bzero(buffer,512);
            int n = read(sockfd,buffer,512);
            if (n < 0)
                error("ERROR reading from socket");
            struct agentMessage msg ;
            msg = parseAgentMessage(buffer);

            if ( first == 1)
            {
                strcpy( agents[agentNumber].name , msg.name);
                first = 0;
            }
            if ( strcmp(msg.mgName,"cpu") == 0 )
            {
                messages[agentNumber][0] = msg;
                recievedMessages[agentNumber] ++;
            }
            else if ( strcmp(msg.mgName,"memory") == 0 )
            {
                messages[agentNumber][1] = msg;
                recievedMessages[agentNumber] ++;
            }
            else if ( strcmp(msg.mgName,"FS write") == 0 )
            {
                messages[agentNumber][2] = msg;
                recievedMessages[agentNumber] ++;
            }
            else if ( strcmp(msg.mgName,"FS read") == 0 )
            {
                messages[agentNumber][3] = msg;
                recievedMessages[agentNumber] ++;
            }
            else
            {
                usleep(200000);
            }
            i++;
        }
    }
    close(sockfd);
    pthread_exit(0);
}

void* agentsHandler (void * arg)
{
    int clilen;
    int sockfd,newsockfd;
    struct sockaddr_in serv_addr, cli_addr;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portAgents);

    if (bind(sockfd, (struct sockaddr *) &serv_addr,
             sizeof(serv_addr)) < 0)
    {
        error("ERROR on binding");
    }
    listen(sockfd,5);
    clilen = sizeof(cli_addr);

    while (1)
    {
        if (finish == 1)
            break;
        else
        {
            listen(sockfd,5);
            int clilen = sizeof(cli_addr);
            newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
            if (newsockfd < 0)
                error("ERROR on accept");
            agents[agentCount].sockfd = newsockfd;
            int temp = agentCount;
            pthread_create(&agents[agentCount].thread,0,agentListener,&temp);
            agentCount++;
        }
    }
    pthread_exit(0);
}


struct agentMessage parseAgentMessage (char* message)
{
    char messageContent[][20] = {"name","mg name","mg content","mg timestamp"};
    char str[4][50] = {""};
    char* temp;
    char* temp2;
    int i =0 ;
    int length;
    int k = 0;
    for ( ; k < 4 ; k++)
    {
        char strtemp1 [50] = "<";
        strcat(strtemp1,messageContent[k]);
        strcat(strtemp1,">");
        char strtemp2 [50] = "</";
        strcat(strtemp2,messageContent[k]);
        strcat(strtemp2,">");

        temp = strstr(message,strtemp1);
        temp2 = strstr(message,strtemp2);
        i = 0 ;
        length = strlen(strtemp1);
        for ( ; i < temp2 - temp - length ; i++)
        {
            str[k][i] = temp[length + i];
        }
    }
    struct agentMessage t ;
    strcpy(t.name , str[0]);
    strcpy(t.mgName , str[1]);
    strcpy(t.mgContent , str[2]);
    strcpy(t.mgTimestamp , str[3]);
    return t;
}


void* serverHandler(void* arg)
{
    int sockfd ;
    struct sockaddr_in serv_addr;
    if (sockfd < 0)
        error("ERROR opening socket");
    if (server == NULL)
    {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr,
          (char *)&serv_addr.sin_addr.s_addr,
          server->h_length);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    serv_addr.sin_port = htons(portServer);
    if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0)
    {
        error("ERROR connecting");
    }

    pthread_t thReciever ;
    pthread_create(&thReciever,0,listenToServer,&sockfd);
    while (1)
    {
        if (finish == 1)
            break;
        else
        {
            int i = 0;
            for (  ; i < agentCount ; i++)
            {
                if ( recievedMessages[i] > sendMessages[i])
                {
                    sendMessages[i] = recievedMessages[i];
                    if (strlen( messages[i][0].name) != 0 ,strlen(messages[i][0].mgContent) != 0,
                            strlen(messages[i][1].mgContent) != 0,strlen(messages[i][2].mgContent) != 0,
                            strlen(messages[i][3].mgContent) != 0,strlen(messages[i][0].mgTimestamp) != 0 )
                    {
                        char* message = createServerMessage(messages[i][0].name,messages[i][0].mgContent,
                                                            messages[i][1].mgContent,messages[i][2].mgContent,
                                                            messages[i][3].mgContent,messages[i][0].mgTimestamp);
                        int n = write(sockfd,message,strlen(message));
                        if (n < 0)
                            error("ERROR writing to socket");
                        usleep(200);
                    }
                    i++;
                }
            }
            usleep(500000);
        }
    }
    close(sockfd);
    pthread_exit(0);
}


char* createServerMessage(char* agentName, char* cpuContent,char* memoryContent, char* fswContent,
                          char* fsrContent , char* timeStamp)
{
    char* args[6] ;
    args[0] = agentName;
    args[1] = cpuContent;
    args[2] = memoryContent;
    args[3] = fswContent;
    args[4] = fsrContent;
    args[5] = timeStamp;
    char* message = malloc(500 * sizeof(char));
    char messageContent[][20] = {"name","cpu content","mem content","fsw content","fsr content","mg timestamp"};
    int i = 0 ;
    for ( ; i < 6 ; i++)
    {
        strcat(message,"<");
        strcat(message,messageContent[i]);
        strcat(message,">");
        strcat(message,args[i]);
        strcat(message,"</");
        strcat(message,messageContent[i]);
        strcat(message,">");
    }
    return message;
}


void* listenToServer (void * arg)
{
    int sockfd = *(int *) arg ;
    int i = 0 ;
    char buffer[256];
    while (1)
    {
        if (finish == 1 )
            break;
        else
        {
            bzero(buffer,256);
            int n = read(sockfd,buffer,255);
            if (n < 0) error("ERROR reading from socket");
            if ( strlen(buffer) != 0)
            {
                char strtemp1 [50] = "<kill>";
                char strtemp2 [50] = "</kill>";
                char* temp;
                char* temp2;
                temp = strstr(buffer,strtemp1);
                temp2 = strstr(buffer,strtemp2);
                i = 0 ;
                int length = strlen(strtemp1);
                char str[50] = {""};
                for ( ; i < temp2 - temp - length ; i++)
                {
                    str[i] = temp[length + i];
                }
                if ( strlen(str) > 0)
                {
                    int k = 0 ;
                    for(;k<agentCount;k++)
                    {
                        if(strcmp(agents[k].name,str)== 0)
                        {
                            sendMessages[k] = recievedMessages[k];
                            int n = write(agents[k].sockfd,buffer,strlen(buffer));
                            if (n < 0)
                                error("ERROR writing kill to socket ");
                        }
                    }
                }

                usleep(20000);
                i++;
            }
        }
    }
    pthread_exit(0);
}
