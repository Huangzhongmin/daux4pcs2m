/* 11-17-2016
******************************************************************************
FILE: east_rfmrt.h
******************************************************************************
*/


#include <stdlib.h>
#include <string.h>
//#define CONFIG_X86_64
//#include <asm/page_types.h>
//#undef CONFIG_X86_64
#include "/home/ltc/drv/rfm2g/include/rfm2g_api.h"

#define RFM_DEVICE0 "/dev/rfm2g0"
#define MAPBUFSIZE  0x00100000 /* 1MB DMA Buffer Size */
#define PCSADDRESS  0x4000000
#define RFM_SCOPEDATA  300
#define OFFSET_1        0x1000
#define OFFSET_2        0x2000
#define RFM_COMMANDS  90



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
extern int east_rfm_close();
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

int east_rfm_open(int bufsize)
{

    RFM2G_STATUS result=0;
//#ifndef NORFM
    RFM2G_UINT64 Offset = 0x140000000 | RFM2G_DMA_MMAP_OFFSET;//useful
    //RFM2G_UINT64 Offset = 0x3200000 | RFM2G_DMA_MMAP_OFFSET;
    //RFM2G_UINT64 Offset = 0x1fc000000 | RFM2G_DMA_MMAP_OFFSET;//pcs

    fprintf(stderr,"in east_rfm_open\n");
    /* Open the Reflective Memory device */
    result = RFM2gOpen(RFM_DEVICE0,&rfmhndl);
       if( result != RFM2G_SUCCESS )
       {
         fprintf(stderr,"ERROR: RFM2gOpen() failed. %s\n",RFM2gErrorMsg(result) );
         //rtprintf("ERROR: RFM2gOpen() failed. %s\n",RFM2gErrorMsg(result) );
            return(-1);
       } 
     //result = RFM2gUserMemory(rfmhndl,(void *)&pDmaMemory,0,0x100000/0x400);
     result = RFM2gUserMemoryBytes(rfmhndl,(volatile void  *)&pDmaMemory,Offset,0x100000); //the result is 256*n
     //result = RFM2gUserMemory(rfmhndl,(void *)&pDmaMemory,0,bufsize/PAGE_SIZE);//this result is right,but can not stop
     if( result != RFM2G_SUCCESS )
      {
            printf( "ERROR: RFM2gUserMemory() failed.\n" );
            printf( "ERROR MSG: %s\n", RFM2gErrorMsg(result));
            RFM2gClose(&rfmhndl);
            return(-3);
      }
    result = RFM2gSetDMAThreshold(rfmhndl, 16); //16 -> DMA transfer for the data larger than 16 Bytes!
    if (result != RFM2G_SUCCESS) {
                fprintf(stderr,"ERROR: RFM2gSetDMAThreshold() failed.\n" );
                fprintf(stderr,"ERROR MSG: %s\n", RFM2gErrorMsg(result));
                RFM2gClose(&rfmhndl);
                return (-1);
    }
//#endif

 /* #ifndef NODEF */
      rfm_init_status=1;
      return(result);
}
                                                             
int east_rfm_clear(unsigned int offset,unsigned int buffersize)
{
    RFM2G_STATUS result=0;
    float* outbuffer=NULL;
#ifndef NORFM
    outbuffer=(float *)malloc(buffersize*sizeof(float));
    fprintf(stderr,"in east_rfm_clear\n");
    memset(outbuffer,0,buffersize*sizeof(float));
    //result = RFM2gWrite(rfmhndl, offset, (void *)outbuffer, buffersize*4 );
    memcpy(pDmaMemory,outbuffer,buffersize*sizeof(float));
   result = RFM2gWrite(rfmhndl, offset, (void *)pDmaMemory, buffersize*sizeof(float) );
    if( result != RFM2G_SUCCESS )
    {
        fprintf(stderr,"ERROR: RFM2gWrite() failed.\n" );
        RFM2gClose( &rfmhndl );
        return(-1);
    }
    free(outbuffer);
#endif /* #ifndef NODEF */
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
#ifndef NORFM
    int i;
    float outbuffer[RFM_SCOPEDATA];
#endif

    //fprintf(stderr,"the status before close is %d\n",rfm_init_status);
    if(rfm_init_status==0) return(0);
    rfm_init_status=0;
#ifndef NORFM
    memset(outbuffer,0,RFM_SCOPEDATA*sizeof(float));
    fprintf(stderr,"in east_rfm_close\n");
    //result = RFM2gWrite(rfmhndl, OFFSET_1, (void *)outbuffer, RFM_COMMANDS*4 );
    memcpy(pDmaMemory,outbuffer,RFM_SCOPEDATA*sizeof(float));
   result = RFM2gWrite(rfmhndl, OFFSET_1, (void *)pDmaMemory, RFM_COMMANDS*sizeof(float) );
    if( result != RFM2G_SUCCESS )
    {
        fprintf(stderr,"ERROR: RFM2gWrite() failed.\n" );
        RFM2gClose( &rfmhndl );
        return(-1);
    }
    result = RFM2gWrite(rfmhndl, OFFSET_2, (void *)pDmaMemory, RFM_SCOPEDATA*sizeof(float) );
    if( result != RFM2G_SUCCESS )
    {
        fprintf(stderr,"ERROR: Could not write data to Reflective Memory.\n" );
        RFM2gClose( &rfmhndl );
        return(-1);
    }
    //result = RFM2gUnMapUserMemory(rfmhndl, (volatile void**)&pDmaMemory, 0x100000/0x400);
    result = RFM2gUnMapUserMemoryBytes(rfmhndl, (volatile void **)&pDmaMemory, 0x100000);
    if(result != RFM2G_SUCCESS)
    {
            //rtprintf("Unmap failed.\n");
            fprintf(stderr,"Unmap failed. 1\n");
    }

     RFM2gClose(&rfmhndl);
#endif /* #ifndef NODEF */
     return result;
}

int get_rfm_status()
{
   return rfm_init_status;
}

int east_rfm_write_float(unsigned int offset,float* data,unsigned int buffersize)
{
#ifndef NORFM
   RFM2G_STATUS result;
   float *outbuffer;
   unsigned int BUFFER_SIZE;
   unsigned int OFFSET;
   BUFFER_SIZE=buffersize;
   outbuffer=data;
   OFFSET=offset;

   memcpy(pDmaMemory,data,buffersize*sizeof(float));
   result = RFM2gWrite(rfmhndl, offset, (void *)pDmaMemory, BUFFER_SIZE*sizeof(float) );
    if( result == RFM2G_SUCCESS )
    {
        //printf( "The data was written to Reflective Memory.  " );
        return 0;
    }
    else
    {
        //printf( "ERROR: Could not write data to Reflective Memory.\n" );
        RFM2gClose( &rfmhndl );
        return(-1);
    }
#endif /* #ifndef NODEF */
    return 0;
}

int east_rfm_write_int(unsigned int offset,int* data,unsigned int buffersize)
{
#ifndef NORFM
   RFM2G_STATUS result;
   int *outbuffer;
   unsigned int BUFFER_SIZE;
   unsigned int OFFSET;
   BUFFER_SIZE=buffersize;
   outbuffer=data;
   OFFSET=offset;
   memcpy(pDmaMemory,data,buffersize*sizeof(int));
   result = RFM2gWrite(rfmhndl, offset, (void *)pDmaMemory, BUFFER_SIZE*sizeof(int) );
    if( result == RFM2G_SUCCESS )
    {
        //printf( "The data was written to Reflective Memory.  " );
        return 0;
    }
    else
    {
        //printf( "ERROR: Could not write data to Reflective Memory.\n" );
        RFM2gClose( &rfmhndl );
        return(-1);
    }
#endif /* #ifndef NODEF */
    return 0;
}

int east_rfm_write_short(unsigned int offset,short* data,unsigned int buffersize)
{
//#ifndef NORFM
   RFM2G_STATUS result;
   short *outbuffer;
   unsigned int BUFFER_SIZE;
   unsigned int OFFSET;
   BUFFER_SIZE=buffersize;
   outbuffer=data;
   OFFSET=offset;
   memcpy(pDmaMemory,data,buffersize*sizeof(short));
   result = RFM2gWrite(rfmhndl, OFFSET, (void *)pDmaMemory, BUFFER_SIZE*sizeof(short) );
    if( result == RFM2G_SUCCESS )
    {
    //    printf( "The data was written to Reflective Memory. \n " );
      //  return 0;
    }
    else
    {
        //printf( "ERROR: Could not write data to Reflective Memory.\n" );
        RFM2gClose( &rfmhndl );
        return(-1);
    }
//#endif /* #ifndef NODEF */
    return 0;
}


int east_rfm_read_short(unsigned int offset,short* data,unsigned int buffersize)
{
#ifndef NORFM
   RFM2G_STATUS result;
   short *inbuffer;
   unsigned int BUFFER_SIZE;
   unsigned int OFFSET;
   BUFFER_SIZE=buffersize;
   inbuffer=data;
   OFFSET=offset;
   memset(pDmaMemory,0,buffersize*sizeof(short));
   result = RFM2gRead(rfmhndl, OFFSET, (void *)pDmaMemory, BUFFER_SIZE*sizeof(short) );
   memcpy(data,pDmaMemory,buffersize*sizeof(short));
    if( result == RFM2G_SUCCESS )
    {
        //printf( "The data was written to Reflective Memory.  " );
        return 0;
    }
    else
    {
        //printf( "ERROR: Could not write data to Reflective Memory.\n" );
        RFM2gClose(&rfmhndl);
        return(-1);
    }
#endif /* #ifndef NODEF */
    return 0;
}

int east_rfm_read_float(unsigned int offset,float* data,unsigned int buffersize)
{
#ifndef NORFM
   RFM2G_STATUS result;
   float *inbuffer;
   unsigned int BUFFER_SIZE;
   unsigned int OFFSET;
   BUFFER_SIZE=buffersize;
   inbuffer=data;
   OFFSET=offset;
   memset(pDmaMemory,0,buffersize*sizeof(float));
   result = RFM2gRead(rfmhndl, OFFSET, (void *)pDmaMemory, BUFFER_SIZE*sizeof(float) );
   memcpy(data,pDmaMemory,buffersize*sizeof(float));

   //result = RFM2gRead(rfmhndl, OFFSET, (void *)inbuffer, BUFFER_SIZE*sizeof(float) );
    if( result == RFM2G_SUCCESS )
    {
        //printf( "The data was written to Reflective Memory.  " );
        return 0;
    }
    else
    {
        //printf( "ERROR: Could not write data to Reflective Memory.\n" );
        RFM2gClose(&rfmhndl);
        return(-1);
    }
#endif /* #ifndef NODEF */
    return 0;
}

int east_rfm_read_int(unsigned int offset,int* data,unsigned int buffersize)
{
#ifndef NORFM
   RFM2G_STATUS result;
   int *inbuffer;
   unsigned int BUFFER_SIZE;
   unsigned int OFFSET;
   BUFFER_SIZE=buffersize;
   inbuffer=data;
   OFFSET=offset;
   memset(pDmaMemory,0,buffersize*sizeof(int));
   result = RFM2gRead(rfmhndl, OFFSET, (void *)pDmaMemory, BUFFER_SIZE*sizeof(int) );
   memcpy(data,pDmaMemory,buffersize*sizeof(int));
   //result = RFM2gRead(rfmhndl, OFFSET, (void *)inbuffer, BUFFER_SIZE*sizeof(int) );
    if( result == RFM2G_SUCCESS )
    {
        //printf( "The data was written to Reflective Memory.  " );
        return 0;
    }
    else
    {
        //printf( "ERROR: Could not write data to Reflective Memory.\n" );
        RFM2gClose(&rfmhndl);
        return(-1);
    }
#endif /* #ifndef NODEF */
    return 0;
}
