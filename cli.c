#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include<fcntl.h>
#include<termios.h>
#include<sys/ioctl.h>


void printls(char *read_buff)
{
    int maxsize = 0 ;
    int i= 0 ;
    int count = 0;
    int t = 0;
    while (read_buff[i] != '\0')
    {
        if (read_buff[i] == '\n')
        {
            t+=1;
        
            if (count > maxsize)
            {
                maxsize = count ;
            }
            count = 0;
            i++ ;
            continue ;
        }
        i++ ;
        count++ ;
    }

    struct winsize size ;
    ioctl (STDIN_FILENO, TIOCGWINSZ, &size) ;
    int num =  (size.ws_col) / (maxsize + 3);

    i=3;

    while (1)
    {
        int j=0;
        for(; j < num; j++)
        {
            int sign = maxsize ;
            while (read_buff[i] != '\n')
            {
                printf ("%c", read_buff[i]) ;
                i++ ;
                sign--;
            }
            while (sign != 0)
            {
                sign--;
                printf(" ");
            }
            i++;
            printf("   ");
            if(read_buff[i] == '\0')
            {
                printf("\n");
                return;
            }
        }
        printf("\n");
    }
}

int recv_file(int sockfd,char* name)
{
    char buff[128]={0};
    if( recv(sockfd,buff,127,0) <= 0)
    {
        return -1;
    }
    if( strncmp(buff,"ok",2) != 0)
    {
        printf("%s\n",buff);
        return 0;
    }

    int size =0;
    sscanf(buff+3,"%d",&size);
    int fd = open(name,O_WRONLY|O_CREAT,0600);
    if( fd == -1)
    {
        send(sockfd,"err",3,0);
        return;
    }

    send(sockfd,"ok",2,0);

    int num = 0;
    int cur_size = 0;
    char data[256] = {0};
    while(1)
    {
        num = recv(sockfd,data,256,0);
        if( num <= 0)
        {
            return -1;
        }
        write(fd,data,num);                     
        cur_size = cur_size+num;

        float f = cur_size*100.0/size;
        printf("\033[?25l");
        printf("download:%.2f%%\r",f);
        fflush(stdout);
       // printf("\033[?25h");
        if( cur_size >= size)
        {
            break;
        }
    }
    printf("\033[?25h");
    printf("\n");
    return 0;
}
void send_file(int c,char* myargv[])
{
    if( myargv[1] == NULL)
    {
        send(c,"get:no file name!",17,0);
        return ;
    }
    int fd = open(myargv[1],O_RDONLY);
    if( fd == -1)
    {
        send(c,"not found!",10,0);
        return ;
    }
    int size = lseek(fd,0,SEEK_END);
    lseek(fd,0,SEEK_SET);

    char status[32] = {0};
    sprintf(status,"ok#%d",size);
    send(c,status,strlen(status),0);

    char cli_status[32]={0};
    if( recv(c,cli_status,31,0) <= 0)
    {
        return ;
    }

    if( strcmp(cli_status,"ok") != 0)
    {
        return;
    }
    char data[256]={0};
    int num = 0;
    int cur_size = 0;
    while((num = read(fd,data,256)) > 0)
    {
        send(c,data,num,0);
        
        cur_size = cur_size+num;
        float f = cur_size*100.0/size;
        printf("\033[?25l");
        printf("download:%.2f%%\r",f);
        fflush(stdout);
      //  printf("\033[?25h");

    
  //  printf("\033[?25h");
        if( cur_size >= size)
        {
            break;
        }
    }
    printf("\033[?25h");

    close(fd);

    return;
}

int main()
{
    int sockfd = socket(AF_INET,SOCK_STREAM,0);
    assert( sockfd != -1 );
    struct sockaddr_in saddr;
    memset(&saddr,0,sizeof(saddr));
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(6000);
    saddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    int res = connect(sockfd,(struct sockaddr*)&saddr,sizeof(saddr));
    assert( res != -1 );

    while( 1 )
    {
        char buff[128] = {0};
        printf("connect success~]$ ");
        fflush(stdout);
        fgets(buff,128,stdin);

        if ( strncmp(buff,"exit",4) == 0 )
        {
            break;
        }

        buff[strlen(buff)-1]=0;

        if( buff[0] == 0)
        {
            continue;
        }
        char tmp[128] = {0};
        strcpy(tmp,buff);

        char* myargv[10] = {0};
        char *s = strtok(tmp," ");

        int i=0;
        while( s != NULL)
        {
            myargv[i++] = s;
            s = strtok(NULL," ");
        }
        if( strcmp(myargv[0],"get") == 0)
        {                           
            send(sockfd,buff,strlen(buff),0); 
            recv_file(sockfd,myargv[1]); 
        }                
        else if( strcmp(myargv[0],"put") == 0)
        {   
            send(sockfd,buff,strlen(buff),0);    
            send_file(sockfd,myargv);
        }
        
        else if(strcmp(myargv[0],"ls") == 0)
        {
            send(sockfd,buff,strlen(buff),0);
            char read_buff[1024] = {0};
            recv(sockfd,read_buff,1023,0);
            printls(read_buff);
        }
        else
        {
            send(sockfd,buff,strlen(buff),0);
            char read_buff[1024] = {0};
            recv(sockfd,read_buff,1023,0);
            printf("%s",read_buff+3);
        }
    
      /*close(sockfd);
    

        send(sockfd,buff,strlen(buff),0);

        memset(buff,0,128);
        recv(sockfd,buff,127,0);
        printf("buff=%s\n",buff);*/
    }
    close(sockfd);
    exit(0);
}

