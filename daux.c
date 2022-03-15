/*
 * @Author: zhongmin.huang
 * @Date: 2022-02-22 09:56:19
 * @LastEditors: zhongmin.huang
 * @LastEditTime: 2022-03-15 15:11:52
 * @FilePath: \undefinedc:\Users\Admin\Desktop\pcs2mdaux\daux.c
 * @Description: 
 */
#include "daq_helper.h"

#include <stdio.h>
#include <unistd.h>
#include <sched.h>


void set_cpu(int id)
{
  cpu_set_t mask;
  CPU_ZERO(&mask);
  CPU_SET(id,&mask);
  if(sched_setaffinity(0,sizeof(mask),&mask) == -1)
  {
	printf("set CPU affinity error!\n");
  }
  else
	printf("set CPU affinity ok!\n");
}


int main(int argc, char* argv[]){
    int raffinity = 1;
    int vaffinity = 2;
    int cycle = 100;
    char zfile[32];

    int getopt;
    while(getopt=getopt(argc,argv,"r::v::c::z::")){
        switch(getopt){
            case 'r':
                raffinity = atoi(optarg);
                printf("RFM repeater is deployed at core: %d\n", raffinity);
                break;
            case 'v':
                vaffinity = atoi(optarg);
                printf("VS controller is deployed at core: %d\n ", vaffinity);
                break;
            case 'c':
                cycle = atoi(optarg);
                printf("Execution cycle :%d(microsecond)", cycle);
                break;
            case 'z':
                strcpy(zfile, optarg);
                printf("Zfile path: %s", zfile);
                break;
            case '?':
                printf("usage:\n");
                printf("-r          RFM repeater affinity. 01 by default.");
                printf("-v          VS controller affinity. 02 by default.");
                printf("-c          Execution cycle count by microsecond. 100 by default.");
                printf("-z          Zfile path. ~/ztargrt.dat by default.");
                break;
        }
    }

    while(1){
        int sockfd;
        struct sockaddr_in server;
        struct sockaddr_in client;
        socklen_t len;
        int num;
        int shot;
        char buf[MAXDATASIZE];

        if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
        {
            perror("Creating socket failed.\n");
            exit(1);
        }
        bzero(&server, sizeof(server));
        server.sin_family = AF_INET;
        server.sin_port = htons(PORT);
        server.sin_addr.s_addr = htonl(INADDR_ANY);
        if(bind(sockfd, (struct sockaddr *)&server, sizeof(server)) == -1)
        {
            perror("Bind() error.\n");
            exit(1);
        }

        len = sizeof(client);
        printf("waitting for shot number...\n");

        num = recvfrom(sockfd, buf, MAXDATASIZE, 0, (struct sockaddr *)&client, &len);
        if(num < 0)
        {
            perror("recvfrom() error.\n");
            exit(1);
        }
        buf[num] = '\0';
        printf("You got a shotnumber and total_time <%s> from client. \nIt's ip is %s, port is %d. \n", buf, inet_ntoa(client.sin_addr),htons(client.sin_port));
       
        sscanf(buf,"%d,%d",&shot,&total_time);
	    printf("shot number is %d,total_time is %d ms\n",shot,total_time);

 

    }
    return 0;
}