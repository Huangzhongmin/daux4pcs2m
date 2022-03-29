/* @(#)east_rfmrt.h	1.3 05/17/08
******************************************************************************
FILE: east_rfmrt.h
******************************************************************************
*/


#include <stdlib.h>
#include <string.h>
//#include <asm/page.h>
#include "/home/ltc/drv/rfm2g/include/rfm2g_api.h"
//#include "GK.h"


#define RFM_DEVICE "/dev/rfm2g0"
#define MAPBUFSIZE  0x00100000 /* 1MB DMA Buffer Size */


#define PAGE_SHIFT      12
#ifdef __ASSEMBLY__
#define PAGE_SIZE       (1 << PAGE_SHIFT)
#else
#define PAGE_SIZE       (1UL << PAGE_SHIFT)
#endif
#define PAGE_MASK       (~(PAGE_SIZE-1))




RFM2GHANDLE rfmhndl;
float *pDmaMemory=NULL;
//extern int east_rfm_init(int bufsize);
//extern int east_rfm_close();
extern int get_rfm_status();
//extern float *get_rfm_map();
/*
******************************************************************************
SUBROUTINE: east_rfm_init
Prepare DMA transfer for rtcipc
INPUT: DmaBaseAddr = address of the physical memory reserved for DMA buffer
        nblock = the number of packets assigned for DMA buffer
OUTPUT: pDmaMemory = the virtual address of the DMA buffer
                the physical address of pDmaMemory is DmaBaseAddr
******************************************************************************
*/
int rfm_init_status=0;
float *rfm_ints=NULL;
int east_rfm_init(int bufsize)
{
    RFM2G_STATUS result=0;

    fprintf(stderr,"in east_rfm_init\n");
    /* Open the Reflective Memory device */
    result = RFM2gOpen(RFM_DEVICE,&rfmhndl);
    if( result != RFM2G_SUCCESS )
       {
         printf("ERROR: RFM2gOpen() failed. %s\n",RFM2gErrorMsg(result) );
            return(-1);
       }

     result = RFM2gUserMemory(rfmhndl,(void *)&pDmaMemory,0,bufsize/PAGE_SIZE);
     if( result != RFM2G_SUCCESS )
      {
            printf( "ERROR: RFM2gUserMemory() failed.\n" );
            printf( "ERROR MSG: %s\n", RFM2gErrorMsg(result));
            RFM2gClose(&rfmhndl);
            return(-1);
      }
    result = RFM2gSetDMAThreshold(rfmhndl, 16); //16 -> DMA transfer for the data larger than 16 Bytes!
    if (result != RFM2G_SUCCESS) {
                printf("ERROR: RFM2gSetDMAThreshold() failed.\n" );
                printf("ERROR MSG: %s\n", RFM2gErrorMsg(result));
                RFM2gClose(&rfmhndl);
                return (-1);
    }
      rfm_init_status=1;
      return(result);
}

/*  
******************************************************************************
SUBROUTINE: east_rfm_close()
******************************************************************************
*/

int east_rfm_close()
{
    RFM2G_STATUS result=0;

    //fprintf(stderr,"the status before close is %d\n",rfm_init_status);
    if(rfm_init_status==0) return;
    rfm_init_status=0;
    /*result = RFM2gUnMapUserMemory(rfmhndl, (void**)&pDmaMemory, MAPBUFSIZE/PAGE_SIZE);
    if(result != RFM2G_SUCCESS)
    {
            //rtprintf("Unmap failed.\n");
            printf("Unmap failed. 1\n");
    }*/
printf("rfm is closed!\n");
     RFM2gClose(&rfmhndl);
     return result;

}
int get_rfm_status()
{
   return rfm_init_status;
}

float *get_rfm_map()
{
   return pDmaMemory;
}

int east_rfm_write(unsigned int offset,float* data,unsigned int buffersize)
{
   RFM2G_STATUS result;
   float *outbuffer;
   unsigned int BUFFER_SIZE;
   unsigned int OFFSET;
   BUFFER_SIZE=buffersize;
   outbuffer=data;
   OFFSET=offset;

   result = RFM2gWrite(rfmhndl, offset, (void *)outbuffer, BUFFER_SIZE*4 );
    if( result == RFM2G_SUCCESS )
    {
        //printf( "The data was written to Reflective Memory.  " );
        return 0;
    }
    else
    {
        //printf( "ERROR: Could not write data to Reflective Memory.\n" );
        //RFM2gClose( &rfmhndl );
        return(-1);
    }
    return 0;
}

int east_rfm_write_short(unsigned int offset,short* data,unsigned int buffersize)
{
   RFM2G_STATUS result;
   short *outbuffer;
   unsigned int BUFFER_SIZE;
   unsigned int OFFSET;
   BUFFER_SIZE=buffersize;
   outbuffer=data;
   OFFSET=offset;
 //  memcpy(pDmaMemory,data,buffersize*sizeof(short));
   result = RFM2gWrite(rfmhndl, offset, (void *)outbuffer, BUFFER_SIZE*sizeof(short) );
    if( result == RFM2G_SUCCESS )
    {
      //  printf( "The data was written to Reflective Memory.\n  " );
       //      return 0;
    }
    else
    {
      //printf( "ERROR: Could not write data to Reflective Memory.\n" );
     //  RFM2gClose( &rfmhndl );
       return(-1);
    }
      return 0;
}
int rfm_read_int(unsigned int offset,int* data,unsigned int buffersize)
{
   
   RFM2G_STATUS  result; 

   result = RFM2gRead(rfmhndl, offset, (void *)data, buffersize*sizeof(int) );
    if( result != RFM2G_SUCCESS )
    {
        printf( "\nERROR: Could not read int data from Reflective Memory.\n" );
        //RFM2gClose( &rfmhndl );
        return(-1);
    }

 
   return 0;
}

int rfm_read_float(unsigned int offset,float* data,unsigned int buffersize)
{
   RFM2G_STATUS  result; 

   result = RFM2gRead(rfmhndl, offset, (void *)data, buffersize*sizeof(float) );
    if( result != RFM2G_SUCCESS )
    {
        printf( "\nERROR: Could not read float data from Reflective Memory.\n" );
        //RFM2gClose( &rfmhndl );
        return(-1);
    }
    
   return 0;
}
