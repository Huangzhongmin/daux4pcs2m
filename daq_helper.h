#ifndef __DAQ_HELPER_H__
#define __DAQ_HELPER_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "calib.h"
#include "afhba-llcontrol-common.h"

/************** Device config ****************/
char LLCHOST[] = "acq2106_110";
int devnum = 0;
#ifdef DO
char DOHOST[] = "acq2106_348";
int do_devnum = 1;
#endif
/********************************************/

#define AO_OFFSET (HB_LEN / 2)
#define BITS2RAW 20.F/65536.F

#define NSHORTS 160
#define DEF_AO_CHAN 32
//#define VO_LEN (aochan * sizeof(short) + (has_do32 ? sizeof(unsigned) : 0))
#define VO_LEN 64 //1 channel(4 bytes) round to 64
//#define DO_IX (16) /* longwords */
#define DO_IX 0
#define MV100 (32768 / 100)
#define PAGE_SHIFT 12
#define PAGE_MASK (~(PAGE_SIZE - 1))

#ifdef __ASSEMBLY__
#define PAGE_SIZE (1 << PAGE_SHIFT)
#else
#define PAGE_SIZE (1UL << PAGE_SHIFT)
#endif

struct info
{
	char para[16];
	int data;
};

void *host_buffer;
void *host_do_buffer;

short *ai_buffer;
short *card_buffer;
int fd, do_fd;
int CLK_DIV = 10;
int EXT_CLK = 1000000;//ext CLK must be 1MHz

int rc = 0;

int samples_buffer = 1; /* set > 1 to decimate max 16*64bytes */
FILE *fp_log;
void (*G_action)(void *);
/** potentially good for cache fill, but sets initial value zero */
int G_POLARITY = 1;
/** env POLARITY=-1 negates feedback this is usefult to know that the
 *  software is in fact doing something 					 */

short *ao_buffer;
unsigned *do_buffer;

int has_do32;
int DUP1 = 0; /* duplicate AI[DUP1], default 0 */
short *AO_IDENT;

struct XLLC_DEF xllc_def = {
	.pa = RTM_T_USE_HOSTBUF,
};

int aochan = DEF_AO_CHAN;

/* SPLIT single HB into 2
 * [0] : AI
 * [1] : AO
 */

short *make_ao_ident(int ao_ident)
{
	short *ids = calloc(aochan, sizeof(short));
	if (ao_ident)
	{
		int ic;

		for (ic = 0; ic < aochan; ++ic)
		{
			ids[ic] = ic * MV100 * ao_ident;
		}
	}
	return ids;
}

/*
 **acq2106_init()**********************************************
 *
 */
void setup();
void acq2106_start(int cycle_time){
	char scriptcmd[128];
	sprintf(scriptcmd, "REMIP=%s EXTCLKDIV=%d ./llc-test-harness-AI1234-AO5-DO6 start_stream > /dev/null", LLCHOST, cycle_time);
	rc = system(scriptcmd);

	#ifdef DO
	sprintf(scriptcmd, "REMIP=%s EXTCLKDIV=%d ./llc-test-harness-AI1-DO6 start_stream > /dev/null", DOHOST, cycle_time);
	rc = system(scriptcmd);
    #endif
	
}

int acq2106_init(int cycle_time, int num_boards, int num_channels)
{
	printf("ACQ2106 initializing ... ");
	/*frequency division*/
	CLK_DIV = cycle_time;
	
	char scriptcmd[128];
	sprintf(scriptcmd, "REMIP=%s EXTCLKDIV=%d ./llc-test-harness-AI1234-AO5-DO6 init_2106 > /dev/null", LLCHOST, cycle_time);
	rc = system(scriptcmd);
	
	#ifdef DO
	sprintf(scriptcmd, "REMIP=%s EXTCLKDIV=%d ./llc-test-harness-AI1-DO6 init_2106 > /dev/null", DOHOST, cycle_time);
	rc = system(scriptcmd);
    #endif

	if(!WIFEXITED(rc))
		printf("ERROR:The init scripts executing failed! code: %d\n",rc);

	int i = 0;
	struct info params[10];
	FILE *fp;
	fp = fopen("parameter.txt", "r");
	if (fp == NULL)
	{
		printf("\nERROR: parameter.txt missing!\n");
		return 0;
	}
	while (!feof(fp))
	{
		fscanf(fp, "%s %d\n", params[i].para, &params[i].data);
		if (strcmp(params[i].para, "DEVNUM") == 0)
			devnum = params[i].data;
		else if (strcmp(params[i].para, "AOCHAN") == 0)
			aochan = params[i].data;
		else if (strcmp(params[i].para, "DO32") == 0)
			has_do32 = params[i].data;
		else if (strcmp(params[i].para, "DUP1") == 0)
			DUP1 = params[i].data;
		else if (strcmp(params[i].para, "AICHAN") == 0)
			nchan = params[i].data;
		i++;
	}
	fclose(fp);

	ncards = num_boards;
	channels = num_channels;
	nchan = ncards * channels;

	//int ao_ident = 0;
	//AO_IDENT = make_ao_ident(ao_ident);

	setup();

	printf("Done.\n");

	printf("ACQ2106 AI Device Host: %s. Sampling cycle: %d microseconds.\n", LLCHOST, CLK_DIV);
	
	#ifdef DO
	printf("ACQ2106 DO Device Host: %s.\n", DOHOST);
	#endif
	return 0;
}

void setup()
{
	setup_logging(devnum);

	ai_buffer = calloc(NSHORTS, sizeof(short));
	card_buffer = calloc(channels, sizeof(short));

	host_buffer = get_mapping(devnum, &fd);
	xllc_def.len = samples_buffer * VI_LEN;

	if (xllc_def.len > 16 * 64)
	{
		xllc_def.len = 16 * 64;
		samples_buffer = xllc_def.len / VI_LEN;
		fprintf(stderr, "WARNING: samples_buffer clipped to %d\n", samples_buffer);
	}
	if (ioctl(fd, AFHBA_START_AI_LLC, &xllc_def))
	{
		perror("ioctl AFHBA_START_AI_LLC");
		exit(1);
	}

	#ifdef DO
	host_do_buffer = get_mapping(do_devnum, &do_fd);
	xllc_def.len = VO_LEN;

	if (has_do32)
	{
		int ll = xllc_def.len / 64;
		xllc_def.len = ++ll * 64;
	}
	if (ioctl(do_fd, AFHBA_START_AO_LLC, &xllc_def))
	{
		perror("ioctl AFHBA_START_AO_LLC");
		exit(1);
	}
	#endif

	/*
	if (has_do32)
	{
		// marker pattern for the PAD area for hardware trace
		unsigned *dox = (unsigned *)ao_buffer;
		int ii;
		for (ii = 0; ii <= 0xf; ++ii)
		{
			dox[DO_IX + ii] = (ii << 24) | (ii << 16) | (ii << 8) | ii;
		}
	}
	*/

	do_buffer = (unsigned *)(host_do_buffer);

	// printf("setup run over!\n");
}

void print_sample(unsigned sample, unsigned tl)
{
	if (sample % 10000 == 0)
	{
		printf("[%10u] %10u\n", sample, tl);
	}
	printf("print sample is running!\n");
}

/*
 * *acq2106_dadig_output()****************************************
 */
int acq2106_dadig_output()
{
	ao_buffer = (short *)((void *)host_buffer + AO_OFFSET); // HB_LEN);
	unsigned *dox = (unsigned *)ao_buffer;
	int i;
	for (i = 0; i < ncards; i++)
		//	dox[DO_IX] = 100;
		dox[i] = 0;
	return 0;
}

/*
 * *acq2106_dig_input()*******************************************
 */
u32 acq2106_dig_input(int crd_index)
{
	u32 *stats = (u32 *)((void *)host_buffer + AO_OFFSET);
	return stats[1];
}

/*
 *  *acq2106_get_tlatch()*********************************
 */
unsigned acq2106_get_tlatch(int crd_index)
{
	//unsigned tl0;
	unsigned tl1;

	tl1 = TLATCH(ai_buffer)[0];
	printf("acq2106_get_tlatch is running!\n");
	return tl1;
}

/*
 * *acq2106_get_calibrations()********************************
 */
/*float *acq2106_get_calibrations(int card)
{
	return (&calibdata[card][0]);
}
*/

/*
 * *acq2106_get_data_pointer()****************************************
 */
short *acq2106_get_data_pointer(int crd_index)
{
	memcpy(card_buffer, ai_buffer + crd_index * channels, channels);
	return card_buffer;
}

/*
 * *acq2106_map_data_to_array()********************************
 */
/*int acq2106_map_data_to_array(int crd_index, short *digitizer_data, int num_to_map, short *digitizer_data_mapped)
{
	int ii;
	for (ii = 0; ii < num_to_map; ii++)
		digitizer_data_mapped[ii] = (short)((float)digitizer_data[ii] * calibdata[crd_index][ii]);
	return 0;
}
*/

/*
 * *acq2106_poll_check()***************************************
 */
int acq2106_poll_check()
{
	return ncards;
}

/*
 * *acq2106_end()**********************************************
 */
int acq2106_end()
{
	munmap(host_buffer, HB_LEN);
	close(fd);
	fclose(fp_log);

#if DO
	munmap(host_do_buffer, VO_LEN);
	close(do_fd);
#endif

	return 0;
}

void acq2106_collect_baseline(int cycle, int total){
	if(cycle == 0)
		memset(baseline, 0, CHNS * sizeof(float));

	int j;
	for (j = 0; j < CHNS; j++)
	{
		baseline[j] += ai_buffer[j] * BITS2RAW / total;
	}

	if(cycle == total - 1){
		for (j = 0; j < CHNS; j++)
			printf("%f ",baseline[j]);
		printf("\n ");
	}
}

#endif