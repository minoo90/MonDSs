#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>


void* cpuThread (void * arg);
void* memoryThread (void * arg);
void* fsWriteThread (void * arg);
void* fsReadThread (void * arg);
void sendMessage(char* );
char* createAgentMessage(char* , char* ,char* , char* );
void createSocketConnection ();
void* recievingMessage (void *);
double memUsage(void);
double fsReadUsage(void);
double fsWriteUsage(void);
double cpuUsage(void);

int finish = 0;
int  portno,sockfd ;
struct hostent *server;
char* name;

void error(char *msg)
{
    perror(msg);
    close(sockfd);
    exit(1);
}
int main(int argc, char *argv[])
{

    if (argc < 4) {
        fprintf(stderr,"usage %s hostname IpAddress PortNumber\n", argv[0]);
        exit(0);
    }

    name = argv[1];
    server = gethostbyname(argv[2]);
    portno = atoi(argv[3]);
    createSocketConnection ();

    pthread_t th1;
    pthread_t th2;
    pthread_t th3;
    pthread_t th4;
    pthread_t th5;
    finish = 0;
    pthread_create(&th1,0,memoryThread,0);
    pthread_create(&th2,0,cpuThread,0);
    pthread_create(&th3,0,fsReadThread,0);
    pthread_create(&th4,0,fsWriteThread,0);
    pthread_create(&th5,0,recievingMessage,0);

    printf(" type stop to stopping the process : ");
    while (1)
    {
        char str[20] ;
        scanf("%s",str);
        if ( strcmp(str,"stop")== 0 )
        {
            finish = 1;
            close(sockfd);
            exit(0);
            break;
        }
        else
            printf("undefined command, type stop to stopping the process : ");
    }

    pthread_join(th5,0);
    pthread_join(th4,0);
    pthread_join(th3,0);
    pthread_join(th2,0);
    pthread_join(th1,0);

    close(sockfd);
    return 0;
}
void* cpuThread(void * arg)
{
    while (1)
    {
        if (finish == 1 )
            break;
        else
        {
            time_t t = time(NULL);
            struct tm tm = *localtime(&t);
            char timeStr[10] = {};
            sprintf(timeStr,"%d:%d:%d",tm.tm_hour,tm.tm_min,tm.tm_sec);

            double d = cpuUsage();
            char cpuStr[50];
            sprintf(cpuStr,"%4.2f",d);
            char* temp = createAgentMessage(name,"cpu",cpuStr,timeStr);
            sendMessage(temp);
            sleep(2);
        }
    }
    pthread_exit(0);
}
void* memoryThread (void * arg)
{
    while (1)
    {
        if (finish == 1)
            break;
        else
        {
            time_t t = time(NULL);
            struct tm tm = *localtime(&t);
            char timeStr[10] = {};
            sprintf(timeStr,"%d:%d:%d",tm.tm_hour,tm.tm_min,tm.tm_sec);

            double d = memUsage();
            char memStr[50];
            sprintf(memStr,"%4.2f",d);
            char* temp = createAgentMessage(name,"memory",memStr,timeStr);
            sendMessage(temp);
            sleep(2);
        }
    }
    pthread_exit(0);
}
void* fsWriteThread (void * arg)
{
    while (1)
    {
        if (finish == 1 )
            break;
        else
        {
            time_t t = time(NULL);
            struct tm tm = *localtime(&t);
            char timeStr[10] = {};
            sprintf(timeStr,"%d:%d:%d",tm.tm_hour,tm.tm_min,tm.tm_sec);

            double d = fsWriteUsage();
            char fswStr[50];
            sprintf(fswStr,"%4.2f",d);
            printf("\n");
            char* temp = createAgentMessage(name,"FS write",fswStr,timeStr);
            sendMessage(temp);
            sleep(2);
        }
    }
    pthread_exit(0);
}
void* fsReadThread (void * arg)
{
    while (1)
    {
        if (finish == 1 )
            break;
        else
        {
            time_t t = time(NULL);
            struct tm tm = *localtime(&t);
            char timeStr[10] = {};
            sprintf(timeStr,"%d:%d:%d",tm.tm_hour,tm.tm_min,tm.tm_sec);

            double d = fsReadUsage();
            char fsrStr[50];
            sprintf(fsrStr,"%4.2f",d);
            char* temp = createAgentMessage(name,"FS read",fsrStr,timeStr);
            sendMessage(temp);
            sleep(2);
        }
    }
    pthread_exit(0);
}
void* recievingMessage (void * arg)
{
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
                printf("message %d :%s^\n",i,buffer);
                if (strcmp(buffer,"stop") == 0)
                {
                    finish = 1;
                    exit(0);
                    break;
                }
                else
                {
                    char strtemp1 [50] = "<kill>";
                    char strtemp2 [50] = "</kill>";
                    char* temp;
                    char* temp2;
                    temp = strstr(buffer,strtemp1); // name
                    temp2 = strstr(buffer,strtemp2);
                    i =0 ;
                    int length = strlen(strtemp1);
                    char str[50] = {""};
                    for ( ; i < temp2 - temp - length ; i++)
                    {
                        str[i] = temp[length + i];
                    }
                    if ( strcmp(str,name) == 0 )
                    {
                        finish = 1;
                        exit(0);
                        break;
                    }
                }
                usleep(20000);
                i++;
            }
        }
    }
    pthread_exit(0);
}


void sendMessage(char* message)
{
    int n = write(sockfd,message,strlen(message));
    if (n < 0)
        error("ERROR writing to socket");
    printf(" message = %s\n",message);
}

void createSocketConnection ()
{
    struct sockaddr_in serv_addr;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr,
          (char *)&serv_addr.sin_addr.s_addr,
          server->h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0)
        error("ERROR connecting");
}


char* createAgentMessage(char* agentName, char* mgName,char* mgContent, char* mgTimeStamp)
{
    char* args[4] ;
    args[0] = agentName;
    args[1] = mgName;
    args[2] = mgContent;
    args[3] = mgTimeStamp;
    char* message = malloc(500 * sizeof(char));
    bzero(message,500);
    char messageContent[][20] = {"name","mg name","mg content","mg timestamp"};
    int i = 0 ;
    for ( ; i < 4 ; i++)
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

double cpuUsage(void)
{
    unsigned long long a[4],b[4];
    double cpuavg;
    FILE *fp;
    fp = fopen("/proc/stat","r");
    fscanf(fp,"%*s %llu %llu %llu %llu",&a[0],&a[1],&a[2],&a[3]);
    fclose(fp);
    sleep(1);
    fp = fopen("/proc/stat","r");
    fscanf(fp,"%*s %llu %llu %llu %llu",&b[0],&b[1],&b[2],&b[3]);
    fclose(fp);
    cpuavg = (((double)(b[0]+b[1]+b[2]) - (double)(a[0]+a[1]+a[2])) / ((double)(b[0]+b[1]+b[2]+b[3]) - (double)(a[0]+a[1]+a[2]+a[3])))*100;
    return cpuavg;
}

double memUsage(void)
{
    FILE *mem_file;
    char buffer[256];
    unsigned int memtotal,memfree,buffs,cachd;
    unsigned int memused;
    mem_file=fopen("/proc/meminfo","r");
    while (fgets(buffer, sizeof(buffer), mem_file)) {
        if (!strncmp(buffer, "MemTotal:", 9))
        {
                sscanf(buffer,"MemTotal: %u", &memtotal);
        }
        if (!strncmp(buffer, "MemFree:", 8))
        {
                sscanf(buffer,"MemFree: %u", &memfree);
        }
        if (!strncmp(buffer, "Buffers:", 8))
        {
                sscanf(buffer,"Buffers: %u", &buffs);
        }
        if (!strncmp(buffer, "Cached:", 7))
        {
                sscanf(buffer,"Cached: %u", &cachd);
        }
    }
    fclose(mem_file);
    memused=(memtotal-memfree);
    memused=(memused-buffs);
    memused=(memused-cachd);
    double memusage;
    memusage=(memused/(double)memtotal)*100;
    return memusage;
}

double fsReadUsage(void){
    FILE *fp;
    char buffer[1024] = "";
    char *devName[40];
    unsigned int minor;
    unsigned long long r1,r2;
    fp = fopen("/proc/diskstats","r");
    while (fgets(buffer, sizeof(buffer), fp))
    {
        sscanf(buffer,"%*d %d %s %*u %*u %llu %*u %*u %*u %*llu %*u %*u %*u %*u",&minor,&devName,&r1);
        if (!strncmp(devName, "sda", 3) && !minor)
        {
                break;
        }
    }
    fclose(fp);
    bzero(buffer,1024);
    sleep(1);
    fp = fopen("/proc/diskstats","r");
    while (fgets(buffer, sizeof(buffer), fp))
    {
        sscanf(buffer,"%*d %d %s %*u %*u %llu %*u %*u %*u %*llu %*u %*u %*u %*u",&minor,&devName,&r2);
        if (!strncmp(devName, "sda", 3) && !minor)
        {
                break;
        }
    }
    fclose(fp);
    double readfs;
    readfs=((double)r2-r1);
    return readfs;
}

double fsWriteUsage(void)
{
    FILE *fp;
    char buffer[1024] = "";
    char *devName[40];
    unsigned int minor;
    unsigned long long w1,w2;
    fp = fopen("/proc/diskstats","r");
    while (fgets(buffer, sizeof(buffer), fp))
    {
        sscanf(buffer,"%*d %d %s %*u %*u %*llu %*u %*u %*u %llu %*u %*u %*u %*u",&minor,&devName,&w1);
        if (!strncmp(devName, "sda", 3) && !minor)
        {
                break;
        }
    }
    fclose(fp);
    bzero(buffer,1024);
    sleep(1);
    fp = fopen("/proc/diskstats","r");
    while (fgets(buffer, sizeof(buffer), fp))
    {
        sscanf(buffer,"%*d %d %s %*u %*u %*llu %*u %*u %*u %llu %*u %*u %*u %*u",&minor,&devName,&w2);
        if (!strncmp(devName, "sda", 3) && !minor)
        {
                break;
        }
    }
    fclose(fp);
    double writefs;
    writefs=((double)w2-w1);
    return writefs;
}
