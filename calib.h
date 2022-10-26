/*
 * @Author: zhongmin.huang
 * @Date: 2022-04-28 16:57:32
 * @LastEditors: zhongmin.huang
 * @LastEditTime: 2022-05-06 15:33:34
 * @FilePath: \新建文件夹 (2)\calib.h
 * @Description: 
 */

#ifndef __CALIB_H__
#define __CALIB_H__
/*
 * GLOBAL: caliba
 */
#define CHNS 96
#define CALIBSFILE "/link/ops/data_hl2m/calib.dat"
#define DAQ_IPF7U 6
#define DAQ_IVS1 16
#define DAQ_RL01 22
#define DAQ_FL01 24
#define DAQ_MP20N 95


#define EM_FL01 19

float calibfactor[CHNS];
float nominaldata[CHNS];

float baseline[CHNS];

#endif