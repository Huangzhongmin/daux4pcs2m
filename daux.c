/*
 * @Author: zhongmin.huang
 * @Date: 2022-02-22 09:56:19
 * @LastEditors: zhongmin.huang
 * @LastEditTime: 2022-04-27 15:06:12
 * @FilePath: \新建文件夹 (2)\daux.c
 * @Description: 
 */
#include "daq_helper.h"
#include "rfm_helper.h"
#include "mds_helper.h"

#include "vscontrol.h"

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#define __USE_GNU
#include <sched.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


//global 
#define PORT 12298


void process(void* data){
//rfm transmitter
	result = RFM2gWrite(Handle, 0, (void *)ai_buffer, 10 * sizeof(short));

#ifdef DEBUG
	if (result == RFM2G_SUCCESS)
	{
		//  printf( "The data was written to Reflective Memory.  " );
	}
	else
	{
		printf("ERROR: Could not write data to Reflective Memory.\n");
		RFM2gClose(&Handle);
		return (-1);
	}
	if (verbose)
	{
		print_sample(sample, tl1);
	}
#endif
//vs cal


}

void run(void (*action)(void *),float time)
{
    pollcat = 0;
	while((tl1 = TLATCH(host_buffer)[0]) == tl0){
		//sched_yield();
		++pollcat;
	}
	memcpy(ai_buffer,host_buffer,VI_LEN);
    action(ai_buffer);
    tl0 = tl1;
}

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
    printf("*********************************************\n");
    printf("******██████╗  █████╗ ██╗   ██╗██╗  ██╗******\n");
    printf("******██╔══██╗██╔══██╗██║   ██║╚██╗██╔╝******\n");
    printf("******██║  ██║███████║██║   ██║ ╚███╔╝ ******\n");
    printf("******██║  ██║██╔══██║██║   ██║ ██╔██╗ ******\n");
    printf("******██████╔╝██║  ██║╚██████╔╝██╔╝ ██╗******\n");
    printf("******╚═════╝ ╚═╝  ╚═╝ ╚═════╝ ╚═╝  ╚═╝******\n");
    printf("***Distributed auxiliary for PCS of HL-2M ***\n");
    printf("*********************************************\n");
    printf("*******D-TACQ2106 Support & VS control*******\n");
    printf("*********************************************\n\n");

    int affinity = 1;
    int cycle = 100;
    char zfile[64];
    strcpy(zfile, "~/ztargrt.dat");
    
    int ch;
    while((ch=getopt(argc,argv,"a::c::z::"))!=-1){
        switch(ch){
            case 'a':
                affinity = atoi(optarg);
                printf("Affinity at core: %d\n ", affinity);
                break;
            case 'c':
                cycle = atoi(optarg);
                printf("Execution cycle :%d(microsecond)\n", cycle);
                break;
            case 'z':
                strcpy(zfile, optarg);
                printf("Zfile path: %s\n", zfile);
                break;
            case '?':
                printf("usage:\n");
                printf("-a          Programme CPU affinity. 01 by default.\n");
                printf("-c          Execution cycle count by microsecond. 100 by default.\n");
                printf("-z          Zfile path. ~/ztargrt.dat by default.\n");
                break;
        }
    }
    printf("Executing with affinity:%d, cycle: %d, zfile:%s\n", affinity, cycle, zfile);

    set_cpu(affinity);
    //TODO:?
    //handle SIGINT

    while(1){
        int shot;
        int total_time;

        int sockfd;
        struct sockaddr_in server;
        struct sockaddr_in client;
        socklen_t len;
        int msglen;
        char buf[100];

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

        msglen = recvfrom(sockfd, buf, 100, 0, (struct sockaddr *)&client, &len);
        if(msglen < 0)
        {
            perror("recvfrom() error.\n");
            exit(1);
        }
        buf[msglen] = '\0';
        sscanf(buf,"%d,%d",&shot,&total_time);
        printf("Got shot info from %s:%d. \n", inet_ntoa(client.sin_addr),htons(client.sin_port));
	    printf("SHOT: %d,Total Time: %d ms\n",shot,total_time);

        //init
        
        //zfile
        //FILE *fp = fopen(zfile, "r");
        //read zfile
 
        //init
        acq2106_init(cycle,4,32);
        rfm_init();
        //vscal_init();
	    //other prepare
        create_tree("mds-server", "fastz_hl2m", shot);

        double clk = 1.0/(EXT_CLK/CLK_DIV);
	    int nsamples = (64+total_time/1000000)*(EXT_CLK/CLK_DIV);
        float c_time;
        float *current_time = (float*)malloc(sizeof(float)*(nsamples));
        memset(host_buffer, 0, VI_LEN);
        
        
        mlockall(MCL_CURRENT);
        
        fprintf(stdout, "Initialization completed. Waiting for trigger...\n ");
        
        //process
        int sss;
        G_action = process;
        for (sss = 0; sss < nsamples;sss++){
            c_time = -7.0 + 1.0 * sss * clk;	
            run(G_action,c_time);
		    current_time[sss] = c_time;
        }
            

        //clean
        //vscal_clean();
        FILE *fpt = fopen("time.txt","w");
        for(sss = 0; sss < nsamples;sss++){
            fprintf(fpt,"%f\n",current_time[sss]);
        }
          
	    fclose(fpt);
        free(current_time); 

        fprintf(stdout, "Waiting for NEXT shot...\n ");

        // parper next shot;
        tl0 = 0xdeadbeef;
    }
    //clean
    rfm_end();
    acq2016_end();
    return 0;
}