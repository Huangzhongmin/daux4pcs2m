/*
 * @Author: zhongmin.huang
 * @Date: 2022-02-22 09:56:19
 * @LastEditors: zhongmin.huang
 * @LastEditTime: 2022-03-15 15:11:52
 * @FilePath: \undefinedc:\Users\Admin\Desktop\pcs2mdaux\daux.c
 * @Description: 
 */
#include "daq_helper.h"
#include "rfm_helper.h"

#include <stdio.h>
#include <unistd.h>
#include <sched.h>

void process(void* data){
    //rfm transmitter
    if (fg2 > 0)
	{
		tl0 = temp;
		pollcat = 0;
	}

	while ((tl1 = TLATCH(host_buffer)[0]) == tl0)
	{
		sched_yield();
		++pollcat;
	}
	memcpy(ai_buffer, host_buffer, VI_LEN);
	action(ai_buffer);
	outbuffer[0] = sss * 0.00002;
	int iii;
	for (iii = 1; iii <= NSHORTS; iii++)
		outbuffer[iii] = (float)ai_buffer[iii - 1] * 1.0;
	//		result = RFM2gWrite(Handle,info[rfmindex].offset+info[rfmindex].num_channel*sizeof(short),(void *)ai_buffer,info[rfmindex].num_channel*sizeof(short));

	info[0].offset = 0x100;
	info[0].num_channel = 96;
	result = RFM2gWrite(Handle, info[0].offset + info[0].num_channel * sizeof(short), (void *)ai_buffer, info[0].num_channel * sizeof(short));

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

	temp = tl1;
	fg2++;
}  
    //VS Cal
}


int main(int argc, char* argv[]){
    int affinity = 1;
    int cycle = 100;
    char zfile[32];

    int getopt;
    while(getopt=getopt(argc,argv,"a::c::z::")){
        switch(getopt){
            case 'a':
                affinity = atoi(optarg);
                printf("Affinity at core: %d\n ", affinity);
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
                printf("-a          Programme CPU affinity. 01 by default.");
                printf("-c          Execution cycle count by microsecond. 100 by default.");
                printf("-z          Zfile path. ~/ztargrt.dat by default.");
                break;
        }
    }

    setAffinity(affinity);

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

        //init
        
        //zfile
        FILE *fp = fopen(zfile, "r");

        //card init
        acq2106_init(100,4,32,1);
        rfm_init();
        G_action = process;

        while(1){
            run(G_action);
        }

        //clean

        
    }
    //clean
    return 0;
}