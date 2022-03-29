
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
// int total_time;
// int nsamples = (total_time+7000)*1000/CLK_DIV

int samples_buffer = 1; /* set > 1 to decimate max 16*64bytes */
int verbose;
FILE *fp_log;
void (*G_action)(void *);
int devnum = 0;
int dummy_first_loop;
/** potentially good for cache fill, but sets initial value zero */
int G_POLARITY = 1;
/** env POLARITY=-1 negates feedback this is usefult to know that the
 *  software is in fact doing something 					 */

short *ao_buffer;
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

int FFNLUT;
void control_feedforward(short *ao, short *ai);

/*
 * *selectBoards()********************************************
 */
struct info
{
	char para[100];
	int data;
	float xs;
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
void run(void (*action)(void *));

float outbuffer[NSHORTS + 1]; /* Data written to another node      */
short inbuffer[NSHORTS];	  /* Data read from another node       */

struct head_msg
{
	char name[256];
	int offset;
	int num_channel;
};
struct head_msg info[2];
int rfmindex = 0, rfmflag = 1;

int acq2106_init(int cycle_time, int num_boards, int num_channels, int dtacq_control)
{
	/*frequency division*/
	CLK_DIV = cycle_time;
	int system(const char *command);
	FILE *fs;
	fs = fopen("llc-test-harness-AI123-AO56", "r+");
	if (fs == NULL)
	{
		printf("can not open file!\n");
		return 0;
	}
	printf("open success!\n");

	int k = 1, col = 25, c, c1, cnt;
	char ch;
	char str1[50] = "EXTCLKDIV=${EXTCLKDIV:-";
	char str2[2] = "}";
	char str[512];
	sprintf(str, "%s%d%s", str1, cycle_time, str2);

	while (k != col)
	{
		ch = fgetc(fs);
		if (ch == '\n')
			k++;
	}
	c = ftell(fs);
	while (fgetc(fs) != '\n')
	{
		c1 = ftell(fs);
	}
	fseek(fs, c, SEEK_SET);
	cnt = c1 - c;
	for (k = 0; k < cnt; k++)
	{
		fputc(str[k], fs);
	}
	fclose(fs);

	selectBoards(num_boards);

	FILE *stream;
	char bf[1024];
	stream = popen("./llc-test-harness-AI123-AO56", "r");
	if (stream == NULL)
	{
		printf("popen error!\n");
	}
	else
	{
		while (fgets(bf, sizeof(bf), stream) != NULL)
			printf("%s\n", bf);
		pclose(stream);
	}
	printf("the init scripts is running!\n");

	int i = 0;
	int j;
	struct info pm[100];
	FILE *fp;
	fp = fopen("parameter.txt", "r");
	if (fp == NULL)
	{
		printf("Can not open file!\n");
		return 0;
	}
	while (!feof(fp))
	{
		fscanf(fp, "%s %d\n", &pm[i].para, &pm[i].data);
		i++;
	}
	fclose(fp);

	for (j = 0; j < i; j++)
	{
		if (strcmp(pm[j].para, "DEVNUM") == 0)
		{
			devnum = pm[j].data;
		}

		if (strcmp(pm[j].para, "AOCHAN") == 0)
		{
			aochan = pm[j].data;
		}

		if (strcmp(pm[j].para, "DO32") == 0)
		{
			has_do32 = pm[j].data;
		}
		if (strcmp(pm[j].para, "DUP1") == 0)
		{
			DUP1 = pm[j].data;
		}
		if (strcmp(pm[j].para, "AICHAN") == 0)
		{
			nchan = pm[j].data;
		}
	}
	ncards = num_boards;
	channels = num_channels;
	nchan = ncards * channels;

	int ao_ident = 0;
	AO_IDENT = make_ao_ident(ao_ident);

	xllc_def.len = VI_LEN;
	// G_action = write_action;

	setup();

	mlockall(MCL_CURRENT);
	memset(host_buffer, 0, VI_LEN);
	if (!dummy_first_loop)
	{
		TLATCH(host_buffer)[0] = tl0; // change TLATCH(ai_buffer) to TLATCH(host_buffer)
	}
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