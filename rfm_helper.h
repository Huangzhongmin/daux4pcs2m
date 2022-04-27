/*
 * @Author: zhongmin.huang
 * @Date: 2022-03-29 15:18:54
 * @LastEditors: zhongmin.huang
 * @LastEditTime: 2022-04-25 21:24:57
 * @FilePath: \新建文件夹 (2)\rfm_helper.h
 * @Description: 
 */
#ifndef __RFM_HELPER_H__
#define __RFM_HELPER_H__

#include "include/rfm2g_api.h"

#define RFM_DEVICE0 "/dev/rfm2g0"
#define DEVICE_PREFIX "/dev/rfm2g/"
#define PROCFILE "/proc/rfm2g"
#define MAPBUFSIZE 0x00100000 /* 1MB DMA Buffer Size */
#define BUFFER_SIZE 160

#define OFFSET_DAQ        0x100
#define OFFSET_head 0x0
#define RFM_RESERVED 500
#define OFFSET_2 0x2000
#define TIMEOUT 60000

struct head_msg
{
    char name[256];
    int offset;
    int num_channel;
};

RFM2G_STATUS result;      /* Return codes from RFM2g API calls */
RFM2G_INT32 i;            /* Loop variable                     */
RFM2G_NODE otherNodeId;   /* Node ID of the other RFM board    */
RFM2G_CHAR string[40];    /* User input                        */
RFM2GEVENTINFO EventInfo; /* Info about received interrupts    */
RFM2G_CHAR device[40];    /* Name of PCI RFM2G device to use   */
RFM2G_INT32 numDevice = 0;
RFM2G_CHAR selection[10];
RFM2G_BOOL loopAgain;
RFM2G_BOOL verbose1;
RFM2GHANDLE Handle = 0;


float outbuffer[NSHORTS + 1]; /* Data written to another node      */
short inbuffer[NSHORTS];	  /* Data read from another node       */

int rfm_init()
{
    printf("RFM init ... ");
    result = RFM2gOpen(RFM_DEVICE0, &Handle);
    if (result != RFM2G_SUCCESS)
    {
        printf("ERROR: RFM2gOpen() failed.\n");
        printf("Error: %s.\n", RFM2gErrorMsg(result));
        return (-1);
    }

    RFM2G_UINT64 offset = 0x140000000 | RFM2G_DMA_MMAP_OFFSET;
    result = RFM2gSetDMAThreshold(Handle, 16); // 16 -> DMA transfer for the data larger than 16 Bytes!
    if (result != RFM2G_SUCCESS)
    {
        fprintf(stderr, "ERROR: RFM2gSetDMAThreshold() failed.\n");
        fprintf(stderr, "ERROR MSG: %s\n", RFM2gErrorMsg(result));
        RFM2gClose(&Handle);
        return (-1);
    }
    printf("Done.\n");
}

int rfm_end()
{
	return RFM2gClose(&Handle);
}


#endif