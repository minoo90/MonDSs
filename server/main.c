#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>

void show();
void kill(char* );
void now(char* );
void avg(char* );
void nowAll();
void avgAll();
void* listener (void*);
void* middleServerListener (void *);
void calculateAverage ( int );
void save ( char* );

struct agentInformation
{
    char name[50];
    char cpuContent[50];
    char memContent[50];
    char fswContent[50];
    char fsrContent[50];
    char mgTimestamp[50];
};
struct agentAverage
{
    char name[50];
    double cpuContent;
    double memContent;
    double fswContent;
    double fsrContent;
    char mgTimestamp[50];
    int count ;
};

void error(char *msg)
{
    perror(msg);
    exit(1);
}

int finish = 0;
struct agentInformation agents[200];
struct agentAverage agentsAverages[200];
int agentcount = 0 ;
int portno;
int socketfds[200] = {0};
int middleServerCount = 0;

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        fprintf(stderr,"usage %s PortNumber \n", argv[0]);
        exit(0);
    }
    portno = atoi(argv[1]);
    pthread_t listenerThread ;
    pthread_create(&listenerThread,0,listener,0);

    char str[50] ;
    while (1)
    {
        printf(" instructions : Show *Kill+ *Now+ *Avg+ *NowAll *AvgAll and (stop) \n enter the command : ");
        scanf("%s",str);
        char * pch;
        pch = strtok (str," ");
        if (pch != NULL)
        {
            if ( strcmp(pch,"Show") == 0)
            {
                show();
            }
            else if ( strcmp(pch,"Kill") == 0)
            {
                scanf("%s",str);
                pch = strtok (str, " ");
                kill(pch);
            }
            else if ( strcmp(pch,"Now") == 0)
            {
                scanf("%s",str);
                pch = strtok (str, " ");
                now(pch);
            }
            else if ( strcmp(pch,"Avg") == 0)
            {
                scanf("%s",str);
                pch = strtok (str, " ");
                avg(pch);
            }
            else if ( strcmp(pch,"NowAll") == 0)
            {
                nowAll();
            }
            else if ( strcmp(pch,"AvgAll") == 0)
            {
                avgAll();
            }
            else if ( strcmp(pch,"stop") == 0)
            {
                break;
            }
            else if ( strcmp(pch,"clear") == 0)
            {
                printf("\033[2J\033[1;1H");
            }
            else
            {
                printf ( "==========\n");
                printf("command no match \n");
                printf ( "==========\n");
            }
        }
    }
    pthread_cancel(listenerThread);
    return 0;
}
void show()
{
    printf ( "==========\nagentcount : %d\n",agentcount);
    int i = 0 ;
    for ( ; i < agentcount; i++)
        printf("%s\n",agents[i].name);
    printf ( "==========\n");
}
void kill(char* arg)
{
    int i = 0 ;
    int found = 0 ;
    for ( ; i < agentcount ; i++ )
    {
        if ( strcmp(arg,agents[i].name) == 0)
        {
            found = 1;
            int j = 0;
            for ( ; j < middleServerCount ; j++)
            {
                if ( socketfds[j] != 0)
                {
                    char buffer[256];
                    sprintf(buffer,"<kill>%s</kill>%d",arg,j);
                    int n = write(socketfds[j],buffer,strlen(buffer));
                    if (n < 0)
                        error("ERROR writing kill to socket ");
                }
            }
            usleep(200000);
            agentcount = agentcount-1;

            printf("agent %s removed --------- agentcount : %d \n",agents[i].name,agentcount);
            printf ( "==========\n");

            strcpy(agents[i].cpuContent,agents[agentcount].cpuContent);
            strcpy(agents[i].fsrContent,agents[agentcount].fsrContent);
            strcpy(agents[i].fswContent,agents[agentcount].fswContent);
            strcpy(agents[i].memContent,agents[agentcount].memContent);
            strcpy(agents[i].mgTimestamp,agents[agentcount].mgTimestamp);
            strcpy(agents[i].name,agents[agentcount].name);

            strcpy(agentsAverages[i].name,agentsAverages[agentcount].name);
            strcpy(agentsAverages[i].mgTimestamp,agentsAverages[agentcount].mgTimestamp);
            agentsAverages[i].count = agentsAverages[agentcount].count;
            agentsAverages[i].cpuContent = agentsAverages[agentcount].cpuContent;
            agentsAverages[i].fsrContent = agentsAverages[agentcount].fsrContent;
            agentsAverages[i].fswContent = agentsAverages[agentcount].fswContent;
            agentsAverages[i].memContent = agentsAverages[agentcount].memContent;

            break;
        }
    }
    if ( found == 0 )
    {
        printf("client %s not found! \n==========\n",arg);
    }
}
void now(char* agentName)
{
    printf ( "==========\n");
    int i = 0 ;
    for ( ; i < agentcount; i++)
    {
        if ( strcmp( agents[i].name,agentName) == 0 )
        {
            printf("at %s \nCPU usage: %s % \nMemory usage: %s % \nFS write: %s B/sec \nFS read: %s B/sec\n",
                   agents[i].mgTimestamp,agents[i].cpuContent, agents[i].memContent,agents[i].fswContent,
                   agents[i].fsrContent);
            printf ( "==========\n");
            return;
        }
    }
    printf ( "agent name not match\n==========\n");
}
void avg(char* agentName)
{
    printf ( "==========\n");
    int i = 0 ;
    for ( ; i < agentcount; i++)
    {
        if ( strcmp( agents[i].name,agentName) == 0 )
        {
            printf("from %s to %s \nCPU usage: %4.2f % \nMemory usage: %4.2f % \nFS write: %4.2f B/sec \nFS read: %4.2f B/sec\n",
                   agentsAverages[i].mgTimestamp, agents[i].mgTimestamp,agentsAverages[i].cpuContent,
                   agentsAverages[i].memContent,agentsAverages[i].fswContent, agentsAverages[i].fsrContent);
            printf ( "==========\n");
            return;
        }
    }
    printf ( "agent name not match\n==========\n");
}
void nowAll()
{
    printf ( "==========\n");
    double temp1 = 0;
    double temp2 = 0;
    double temp3 = 0;
    double temp4 = 0;
    int i = 0 ;
    for ( ; i < agentcount; i++)
    {
        temp1 += atof(agents[i].cpuContent);
        temp2 += atof(agents[i].memContent);
        temp3 += atof(agents[i].fswContent);
        temp4 += atof(agents[i].fsrContent);
    }

    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    char timeStr[10];
    sprintf(timeStr,"%d:%d:%d",tm.tm_hour,tm.tm_min,tm.tm_sec);
    printf("at %s \nCPU usage: %4.2f % \nMemory usage: %4.2f % \nFS write: %4.2f B/sec \nFS read: %4.2f B/sec\n",
           timeStr,temp1/agentcount,temp2/agentcount,temp3/agentcount,temp4/agentcount);
    printf ( "==========\n");
}
void avgAll()
{
    printf ( "==========\n");
    double temp1 = 0;
    double temp2 = 0;
    double temp3 = 0;
    double temp4 = 0;
    int i = 0 ;
    for ( ; i < agentcount; i++)
    {
        temp1 += agentsAverages[i].cpuContent;
        temp2 += agentsAverages[i].memContent;
        temp3 += agentsAverages[i].fswContent;
        temp4 += agentsAverages[i].fsrContent;
    }

    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    char timeStr[10];
    sprintf(timeStr,"%d:%d:%d",tm.tm_hour,tm.tm_min,tm.tm_sec);
    printf("from %s to %s\nCPU usage: %4.2f % \nMemory usage: %4.2f % \nFS write: %4.2f B/sec \nFS read: %4.2f B/sec\n",
           agentsAverages[0].mgTimestamp, timeStr,temp1/agentcount,temp2/agentcount,temp3/agentcount,
           temp4/agentcount);
    printf ( "==========\n");

}

void* listener (void* arg)
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
    serv_addr.sin_port = htons(portno);

    if (bind(sockfd, (struct sockaddr *) &serv_addr,
             sizeof(serv_addr)) < 0)
        error("ERROR on binding");
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
            int temp = newsockfd;
            pthread_t th;
            pthread_create(&th,0,middleServerListener,&temp);
            socketfds[middleServerCount] = temp;
            middleServerCount++;
        }
    }
    pthread_exit(0);
}
void* middleServerListener (void * arg)
{
    int i = 0 ;
    char buffer[255];
    int sockfd = *(int *) arg;
    while (1)
    {
        if (finish == 1 )
            break;
        else
        {
            bzero(buffer,255);
            int n = read(sockfd,buffer,255);
            if (n < 0) error("ERROR reading from socket");
            if ( strlen(buffer) != 0)
            {
                save( buffer );
                i++;
            }
        }
    }
    close(sockfd);
    pthread_exit(0);
}

void save ( char* message )
{
    char messageContent[][20] = {"name","cpu content","mem content","fsw content","fsr content","mg timestamp"};
    char str[6][50] = {""};
    char* temp;
    char* temp2;
    int i =0 ;
    int length;
    int k = 0;
    for ( ; k < 6 ; k++)
    {
        char strtemp1 [50] = "<";
        strcat(strtemp1,messageContent[k]);
        strcat(strtemp1,">");
        char strtemp2 [50] = "</";
        strcat(strtemp2,messageContent[k]);
        strcat(strtemp2,">");

        temp = strstr(message,strtemp1);
        temp2 = strstr(message,strtemp2);
        i =0 ;
        length = strlen(strtemp1);
        for ( ; i < temp2 - temp - length ; i++)
        {
            str[k][i] = temp[length + i];
        }
    }

    if ( strlen(str[0]) == 0 )
    {
        printf("the size is zero : %s \n" ,str[0] );
        return;
    }
    struct agentInformation t ;
    strcpy( t.name , str[0]);
    strcpy(t.cpuContent , str[1]);
    strcpy(t.memContent , str[2]);
    strcpy(t.fswContent , str[3]);
    strcpy(t.fsrContent , str[4]);
    strcpy(t.mgTimestamp , str[5]);
    i = 0 ;
    int found = 0;
    for ( ; i < agentcount; i++)
    {
        if ( strcmp(agents[i].name , t.name) == 0 )
        {
            strcpy(agents[i].cpuContent,t.cpuContent);
            strcpy(agents[i].memContent,t.memContent);
            strcpy(agents[i].fswContent,t.fswContent);
            strcpy(agents[i].fsrContent,t.fsrContent);
            strcpy(agents[i].mgTimestamp,t.mgTimestamp);
            calculateAverage(i);
            found = 1;
            break;
        }
    }
    if ( found == 0 ) // agent not found
    {
        strcpy(agents[agentcount].name,t.name);
        strcpy(agents[agentcount].cpuContent,t.cpuContent);
        strcpy(agents[agentcount].memContent,t.memContent);
        strcpy(agents[agentcount].fswContent,t.fswContent);
        strcpy(agents[agentcount].fsrContent,t.fsrContent);
        strcpy(agents[agentcount].mgTimestamp,t.mgTimestamp);

        strcpy(agentsAverages[agentcount].name,t.name);
        agentsAverages[agentcount].cpuContent = atof(t.cpuContent);
        agentsAverages[agentcount].memContent = atof(t.memContent);
        agentsAverages[agentcount].fswContent = atof(t.fswContent);
        agentsAverages[agentcount].fsrContent = atof(t.fsrContent);
        strcpy(agentsAverages[agentcount].mgTimestamp,t.mgTimestamp);
        agentsAverages[agentcount].count = 1;
        agentcount++;
    }
}

void calculateAverage ( int index )
{
    double temp = agentsAverages[index].cpuContent * agentsAverages[index].count;
    temp += atof ( agents[index].cpuContent );
    temp = temp / ( agentsAverages[index].count +1 );
    agentsAverages[index].cpuContent = temp;

    temp = agentsAverages[index].memContent * agentsAverages[index].count;
    temp += atof ( agents[index].memContent );
    temp = temp / ( agentsAverages[index].count +1 );
    agentsAverages[index].memContent = temp;

    temp = agentsAverages[index].fswContent * agentsAverages[index].count;
    temp += atof ( agents[index].fswContent );
    temp = temp / ( agentsAverages[index].count +1 );
    agentsAverages[index].fswContent = temp;

    temp = agentsAverages[index].fsrContent * agentsAverages[index].count;
    temp += atof ( agents[index].fsrContent );
    temp = temp / ( agentsAverages[index].count +1 );
    agentsAverages[index].fsrContent = temp;
    agentsAverages[index].count ++;


}
