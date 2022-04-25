/*
 * @Author: zhongmin.huang
 * @Date: 2022-04-25 14:15:25
 * @LastEditors: zhongmin.huang
 * @LastEditTime: 2022-04-25 19:58:00
 * @FilePath: \daux4pcs2m\rfm-transmitter.c
 * @Description: 
 */
#include  "rfm_helper.h"



int rfm_transmitter(void* data,int datalen,){
    return RFM2gWrite(Handle, OFFSET_DAQ, (void *)ai_buffer, );
}