#ifndef __DAQ_HELPER_H__
#define __DAQ_HELPER_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "afhba-llcontrol-common.h"

#define AO_OFFSET (HB_LEN / 2)

#define NSHORTS 160
#define DEF_AO_CHAN 32
#define VO_LEN (aochan * sizeof(short) + (has_do32 ? sizeof(unsigned) : 0))
#define DO_IX (16) /* longwords */
#define MV100 (32768 / 100)
#define PAGE_SHIFT 12
#define PAGE_MASK (~(PAGE_SIZE - 1))

#ifdef __ASSEMBLY__
#define PAGE_SIZE (1 << PAGE_SHIFT)
#else
#define PAGE_SIZE (1UL << PAGE_SHIFT)
#endif

void *host_buffer;
short *ai_buffer;
short *card_buffer;
int fd;
int CLK_DIV = 10;
int EXT_CLK = 1000000;

int rc = 0;

int samples_buffer = 1; /* set > 1 to decimate max 16*64bytes */
FILE *fp_log;
void (*G_action)(void *);
int devnum = 0;
/** potentially good for cache fill, but sets initial value zero */
int G_POLARITY = 1;
/** env POLARITY=-1 negates feedback this is usefult to know that the
 *  software is in fact doing something 					 */

short *ao_buffer;
int has_do32;
int DUP1 = 0; /* duplicate AI[DUP1], default 0 */
short *AO_IDENT;

unsigned tl0 = 0xdeadbeef;   // always run one dummy loop 
unsigned tl1;
unsigned sample;
int println = 0;
int pollcat = 0;
int sss;
void *pDMem = NULL;
unsigned int BUFFER = 160;

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

int FFNLUT;
void control_feedforward(short *ao, short *ai);

/*
 * *selectBoards()********************************************
 */
struct info
{
	char para[16];
	int data;
};

int calibdata_initialized = 0;
float calibdata[6][32]; // max boards * max channels

static void selectBoards(int num_boards)
{
	struct info cal[200];
	ncards = num_boards;

	if (!calibdata_initialized)
	{
		int crd_index;
		int chn;
		for (crd_index = 0; crd_index < ncards; crd_index++)
		{
			for (chn = 0; chn < channels; chn++)
			{
				calibdata[crd_index][chn] = 1.0;
			}
		}
		calibdata_initialized = 1;
	}
	printf("selectboards is running!\n");
}

/*
 **acq2106_init()**********************************************
 *
 */
void setup();

int acq2106_init(int cycle_time, int num_boards, int num_channels)
{
	/*frequency division*/
	CLK_DIV = cycle_time;
	char modifyDiv[128];
	sprintf(modifyDiv, "sed '25c EXTCLKDIV=${EXTCLKDIV:-%d}' -i llc-test-harness-AI123-AO56", cycle_time);
	system(modifyDiv);

	selectBoards(num_boards);

	rc = system("llc-test-harness-AI123-AO56");
	if(WIFEXITED(rc))
		printf("The init scripts is executed!\n");

	int i = 0;
	struct info params[10];
	FILE *fp;
	fp = fopen("parameter.txt", "r");
	if (fp == NULL)
	{
		printf("ERROR: parameter.txt missing!\n");
		return 0;
	}
	while (!feof(fp))
	{
		fscanf(fp, "%s %d\n", &params[i].para, &params[i].data);
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

	int ao_ident = 0;
	AO_IDENT = make_ao_ident(ao_ident);

	xllc_def.len = VI_LEN;

	setup();

	mlockall(MCL_CURRENT);

}

void setup()
{
	// printf("setup is running!\n");
	char logfile[80];
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

	xllc_def.pa += AO_OFFSET; // HB_LEN;
	xllc_def.len = VO_LEN;

	if (has_do32)
	{
		int ll = xllc_def.len / 64;
		xllc_def.len = ++ll * 64;
	}
	if (ioctl(fd, AFHBA_START_AO_LLC, &xllc_def))
	{
		perror("ioctl AFHBA_START_AO_LLC");
		exit(1);
	}

	ao_buffer = (short *)((void *)host_buffer + AO_OFFSET); // HB_LEN);

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
}

/*
 * *acq2106_dig_input()*******************************************
 */
u32 acq2106_dig_input(int crd_index)
{
	u32 *stats = (u32 *)((void *)host_buffer + AO_OFFSET);
	return stats[1];
}

unsigned sample;
int println = 0;
int pollcat = 0;
int sss;
void *pDMem = NULL;
unsigned int BUFFER = 160;

void run(void (*action)(void *))
{
	unsigned tl0 = 0xdeadbeef;	/* always run one dummy loop */
	unsigned tl1;
	mlockall(MCL_CURRENT);

}

/*
 *  *acq2106_get_tlatch()*********************************
 */
unsigned acq2106_get_tlatch(int crd_index)
{
	unsigned tl0;
	unsigned tl1;

	tl1 = TLATCH(ai_buffer)[0];
	printf("acq2106_get_tlatch is running!\n");
	return tl1;
}

/*
 * *acq2106_get_calibrations()********************************
 */
float *acq2106_get_calibrations(int card)
{
	return (&calibdata[card][0]);
}

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
int acq2106_map_data_to_array(int crd_index, short *digitizer_data, int num_to_map, short *digitizer_data_mapped)
{
	int ii;
	for (ii = 0; ii < num_to_map; ii++)
		digitizer_data_mapped[ii] = (short)((float)digitizer_data[ii] * calibdata[crd_index][ii]);
	return 0;
}

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
	printf("acq2106 end is running!\n");
	return 0;
}

#endif