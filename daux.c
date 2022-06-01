#include "daq_helper.h"
#include "rfm_helper.h"
#include "mds_helper.h"
#include "calib.h"
#include "vscontrol.h"

#include <signal.h>
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
volatile sig_atomic_t sigint_flag = 0;
double tick;
unsigned tl0 = 0xdeadbeef;
unsigned tl1;
int pollcat = 0;
int sss;

int istest = 0;
int saveall = 1;

#define PORT 12298
float current_time;

//daq data
int nsamples;
float *timebase;
float *signal_data[CHNS];
char* signal_list[CHNS] = {
"PCIPF1U","PCIPF2U","PCIPF3U","PCIPF4U","PCIPF5U","PCIPF6U","PCIPF7U","PCIPF8U",
"PCIPF1L","PCIPF2L","PCIPF3L","PCIPF4L","PCIPF5L","PCIPF6L","PCIPF7L","PCIPF8L",
"PCIVS1","PCIVS2","PCICSA","PCICSB","PCITFA","PCITFB","PCRL01","PCRL02",
"PCFL1","PCFL2","PCFL3","PCFL4","PCFL5","PCFL6","PCFL7","PCFL8",
"PCFL9","PCFL10","PCFL11","PCFL12","PCFL13","PCFL14","PCFL15","PCFL16",
"PCFL17","PCFL18","PCFL19","PCFL20","PCFL21","PCFL22","PCFL23","PCFL24",
"PCFL25","PCFL26","PCFL27","PCFL28","PCFL29","PCFL30",
"PCMP01T","PCMP02T","PCMP03T","PCMP04T","PCMP05T","PCMP06T","PCMP07T","PCMP08T","PCMP09T","PCMP10T",
"PCMP11T","PCMP12T","PCMP13T","PCMP14T","PCMP15T","PCMP16T","PCMP17T","PCMP18T","PCMP19T","PCMP20T",
"PCMP01N","PCMP02N","PCMP03N","PCMP04N","PCMP05N","PCMP06N","PCMP07N","PCMP08N","PCMP09N","PCMP10N",
"PCMP11N","PCMP12N","PCMP13N","PCMP14N","PCMP15N","PCMP16N","PCMP17N","PCMP18N","PCMP19N","PCMP20N",
"PCNE1","PCNE2"
};


void process(void* data){
//rfm transmitter
    result = RFM2gWrite(Handle, OFFSET_DAQ, (void *)data, CHNS * sizeof(short));
//fastz
    //fastz(data, current_time);
}

void run(void (*action)(void *))
{
    int idx;
    pollcat = 0;
	while((tl1 = TLATCH(host_buffer)[0]) == tl0){
		//sched_yield();
		++pollcat;
	}
	memcpy(ai_buffer,host_buffer,VI_LEN);
    //write_action(ai_buffer);
    if(saveall){
        for(idx = 0;idx < CHNS;idx++)
            signal_data[idx][sss] = ai_buffer[idx] * calibfactor[idx] * BITS2RAW;
        ;
    }

    action(ai_buffer);
    tl0 = tl1;
}

void save_signal(int shot){
    int i;
    for(i = 0;i < CHNS;i++){
        write_signal(signal_list[i], timebase, signal_data[i], nsamples);
    }
}

void set_cpu(int id){
  cpu_set_t mask;
  CPU_ZERO(&mask);
  CPU_SET(id,&mask);
  pid_t pid = getpid();
  if (sched_setaffinity(0, sizeof(mask), &mask) == -1)
  {
	printf("Set CPU affinity error!\n");
  }
  else
	printf("Set Process %d's affinity to %d OK!\n",pid,id);
}

int readcalib(char* file,float* calibs){
    FILE* fp = fopen(file, "r");
    if(!fp)
        fprintf(stderr, "ERROR: Missing calib.dat\n");
    char buf[128];
    char* lasts;
    int c = 0;
    while(fgets(buf,sizeof(buf),fp)!=NULL){
        if(buf[0]=='!' || isspace(buf[0]))
            continue;
        else{
            lasts = strrchr(buf,' ');
            if(++lasts)
                sscanf(lasts,"%f",&calibs[c++]);
        }
    }
    fclose(fp);
    return c;
}

static void sigint_action(int sig){
    if(sss!=0){
        if(sigint_flag == 0){
            write(STDERR_FILENO, "\nDuring Shot! Press Ctrl-C again to force quit.\n", 48);
            sigint_flag = 1;
        }
        else{
            if(fp_log)
                fclose(fp_log);
            write(STDERR_FILENO, "\nForce Quit.\n", 13);
            exit(0);
        }
    }
    else{
        exit(0);
    }
}

int main(int argc, char* argv[]){
    //Raises an segment fault with unknown impact at MDS disconnect when quit 
    //signal(SIGINT, sigint_action);

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
    int cycle = 50;
    char zfile[64];
    strcpy(zfile, "/link/ops/data_hl2m/ztargrt.dat");

    int ch;
    while((ch=getopt(argc,argv,"a:c:z:nt"))!=-1){
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
            case 'n':
                saveall = 0;
                printf("NOT save daq data.\n");
                break;
            case 't':
                istest = 1;
                strcpy(zfile, "/link/ops/data_hl2m/test_ztarget.dat");
                printf("TEST MODE.\n");
                break;
            case '?':
                printf("usage:\n");
                printf("-a          Programme CPU affinity. 01 by default.\n");
                printf("-c          Execution cycle count by microsecond. 100 by default.\n");
                printf("-z          Zfile path. /link/ops/data_hl2m/ztargrt.dat by default.\n");
                printf("-n          Only save fastz data, do NOT save daq data.\n");
                printf("-t          Test mode. Using /link/ops/data_hl2m/test_ztarget.dat and dummy data to call fastz.\n");
                exit(0);
        }
    }
    if(saveall)
        printf("Save DAQ data.\n");
    printf("Executing with affinity:%d, cycle: %d, zfile:%s\n\n", affinity, cycle, zfile);

    set_cpu(affinity);
    acq2106_init(cycle,4,32);
    rfm_init();
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

    //Main cycle
    while(1){
        printf("\nWaiting for shot info...\n");
        msglen = recvfrom(sockfd, buf, 100, 0, (struct sockaddr *)&client, &len);
        if(msglen < 0)
        {
            perror("recvfrom() error.\n");
            exit(1);
        }
        buf[msglen] = '\0';
        sscanf(buf,"%d,%d",&shot,&total_time);
        printf("Got shot info from %s:%d. \n", inet_ntoa(client.sin_addr),htons(client.sin_port));
	    printf("Shot: %d,Shot Time: %f s\n",shot,total_time*1e-6);
        printf("Shot %d initializing ... \n",shot);
    
        //init
        tick = 1.0 / (EXT_CLK / CLK_DIV);
        nsamples = (3 + 9 + total_time / 1000000) * (EXT_CLK / CLK_DIV);
        timebase = (float *)calloc(nsamples,sizeof(float));

        printf("\tCreate MDSPlus tree %d.\n",shot);
        create_tree("mds-server", "fastz_hl2m", shot);
        
        printf("\tFastZ init.\n");
        fastz_init(tick, zfile);

        printf("\tDAQ starting.");
        readcalib(CALIBSFILE, calibfactor);
        
        if(saveall){
            signal_data[0] = (float *)malloc(sizeof(float) * CHNS * nsamples);
            int dx = 0;
            for(dx = 1; dx < CHNS; dx++)
                signal_data[dx] = signal_data[dx-1] + nsamples;
        }
        memset(host_buffer, 0, VI_LEN);
        TLATCH(host_buffer)[0] = tl0;
        G_action = process;
        printf("Total samples:%d.\n",nsamples);
        acq2106_start(cycle);
        
        mlockall(MCL_CURRENT);
        
        printf("Initialization completed. Waiting for trigger...\n");
        //process
   
        for (sss = 0; sss < nsamples;sss++){
            if(sss==0){
                printf("Processing...");
                fflush(stdout);
            }
            current_time = -9.0 + 1.0 * sss * tick;
            run(G_action);
		    timebase[sss] = current_time;
        }
        sss = 0;
        printf("Shot completed.\n");

        printf("Archiving data ... ");
        open_tree("mds-server", "fastz_hl2m", shot);
        if(saveall)
            save_signal(shot);
        save_fastz(shot);
        MdsClose("fastz_hl2m", &shot);
        printf("Done.\n");
        
        printf("Cleaning shot ... ");
        free(timebase);
        tl0 = 0xdeadbeef;
        if(saveall)
            free(signal_data[0]);
        fastz_clean();
        printf("Done.\n\nNEXT SHOT will be %d:\n", shot + 1);
        
        sigint_flag = 0;
    }
    //clean
    acq2106_end();
    rfm_end();
    return 0;
}
