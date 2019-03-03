#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<pthread.h>
#include<assert.h>
void thread_start(int c);
/*void get_argv(char buff[],char *myargv[]);*/

void* work_thread(void* arg);
void get_argv(char buff[],char * myargv[]);

void send_file(int c,char* myargv[]);

int recv_file(int sockfd,char *name);
