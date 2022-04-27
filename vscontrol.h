/*
 * @Author: zhongmin.huang
 * @Date: 2022-04-26 10:11:20
 * @LastEditors: zhongmin.huang
 * @LastEditTime: 2022-04-27 10:40:18
 * @FilePath: \新建文件夹 (2)\vscontrol.h
 * @Description: 
 */
#ifndef __VSCONTROL_H__
#define _VSCONTROL_H__

#include <stdlib.h>
#include <string.h>

#define E_MATRIX_LEN 93


typedef struct point
{
    double x;
    double y;
} Point;

/*
 * GLOBAL: ztarget file data
 */
extern int shot;
int icmode;

//vs voltage current
int ivs1_len,ivs2_len,vvs1_len,vvs2_len;
Point *ivs1,*ivs2,*vvs1,*vvs2;

//limit alg duration
double limit_st,limit_et;

//rz ref
int rx1ref_len, zx1ref_len, rx2ref_len, zx2ref_len,ipnorm_len;
Point *rx1ref,*zx1ref,*rx2ref,*zx2ref,*ipnorm;

//MMatrix
int WhichM_len;
Point *WhichM;
double *MMatrix;

//EMatrix
int WhichE_len;
Point *WhichE;
double **EMatrix;//[][93]

//gp,gi,gd,tp,ti,td
int rx1gp_len, zx1gp_len, rx2gp_len, zx2gp_len, rx1gd_len, zx1gd_len, rx2gd_len, zx2gd_len, rx1gi_len, zx1gi_len, rx2_gi_len, zx2gi_len;
Point *rx1gp, *zx1gp, *rx2gp, *zx2gp, *rx1gd, *zx1gd, *rx2gd, *zx2gd, *rx1gi, *zx1gi, *rx2gi, *zx2gi;
int rx1taup_len, zx1taup_len, rx2taup_len, zx2taup_len, rx1taud_len, zx1taud_len, rx2taud_len, zx2taud_len, rx1taui_len, zx1taui_len, rx2taui_len, zx2taui_len;
Point *rx1taup, *zx1taup, *rx2taup, *zx2taup, *rx1taud, *zx1taud, *rx2taud, *zx2taud, *rx1taui, *zx1taui, *rx2taui, *zx2taui;

//Slow Z time const,Slow Z delay time
int sztc_len,szdt_len,ipref_len;
Point *sztc, *szdt, *ipref;

int vs1mode, vs2mode;
double vs1_st, vs1_et, vs1_st, vs2_et;

//buffer for read ztarget 
#define ZFBUFLEN 1024 * 2
char zfbuf[ZFBUFLEN];

/*
 * GLOBAL: real target
 */

double *t_zx1ref,;

#include "math.h"

int liner(double dx, Point* vertics, int len, double starttime, double endtime, double *data)
{
    int stages = len - 1;
    double k, b;
    int i = 0;
    int s;
    double time,lastt=-10086;
    for (s = 0; s < stages;s++){
        k = (vertics[s + 1].y - vertics[s].y) / (vertics[s + 1].x - vertics[s].x);
        b= vertics[s].y - k * vertics[s].x;

        for (time = vertics[s].x; time < vertics[s + 1].x; time+=dx){
            if((time-lastt)<1e-6){ //time==lastt,skip a point when inflexion
                continue;
            }
            if(time>=starttime && time<=endtime){
                data[i] = k * time + b;
                lastt=time;
                i++;
            }
        }
        if(s==stages-1){//fix the last closed interval
                data[i] = k * time + b;
        }
    }
    return i;
}

void time_cal(double dx, double starttime, double endtime, double *time)
{
    int j;
    j = 0;
    double i;
    for (i = starttime; i <= endtime; i += dx){
        time[j] = i;
        j++;
    }
}
////////////////////////////////////////////////////////
void get_wave(Point** wave,unsigned int* wnum,FILE* fp){
    int i;
    double x,y;
    fgets(zfbuf, ZFBUFLEN, fp);
    sscanf(zfbuf, "%d", wnum);
    *wave = (Point *)malloc(*wnum * sizeof(Point));
    for (i = 0;i< *wnum; i++){
        fgets(zfbuf, ZFBUFLEN, fp);
        sscanf(zfbuf, "%lf, %lf\n", &((*wave)[i].x), &((*wave)[i].y));
    }
}

void read_ztarget(char *zfile)
{
    int i;
    double tmp;

    FILE *zfp = fopen(zfile,"r");
    if (zfp == NULL){
        fprintf(stderr, "ERROR:Ztarget file missing.\n");
        exit(1);
    }

    fgets(zfbuf, ZFBUFLEN, zfp);
    sscanf(zfbuf, "%d", &shot);
    
    fgets(zfbuf, ZFBUFLEN, zfp);
    sscanf(zfbuf, "%d", &icmode);

    get_wave(&ivs1, &ivs1_len, zfp);
    get_wave(&ivs2, &ivs2_len, zfp);
    get_wave(&vvs1, &vvs1_len, zfp);
    get_wave(&vvs2, &vvs2_len, zfp);

    fgets(zfbuf, ZFBUFLEN, zfp);
    sscanf(zfbuf, "%lf %lf", &limit_st, &limit_et);

    get_wave(&rx1ref, &rx1ref_len, zfp);
    get_wave(&zx1ref, &zx1ref_len, zfp);
    get_wave(&rx2ref, &rx2ref_len, zfp);
    get_wave(&zx2ref, &zx2ref_len, zfp);
    get_wave(&ipnorm, &ipnorm_len, zfp);

    get_wave(&WhichM, &WhichM_len, zfp);
    
    MMatrix = (double *)malloc(WhichM_len * sizeof(double));
    for (i = 0; i < WhichM_len;i++){
        fgets(zfbuf, ZFBUFLEN, zfp);
        sscanf(zfbuf, "%lf", &(MMatrix[i]));
    }
    
    get_wave(&WhichE, &WhichE_len, zfp);
    
    EMatrix = (double **)malloc(WhichE_len * sizeof(double*));
    for (i = 0; i < WhichE_len;i++){
        EMatrix[i] = (double *)malloc(E_MATRIX_LEN * sizeof(double));
        fgets(zfbuf, ZFBUFLEN, zfp);
        int c = 0;
        char *token = strtok(zfbuf, ", ");
        while(token){
            sscanf(token, "%lf", &EMatrix[i][c++]);
            token = strtok(NULL, ", ");
        }
    }

    get_wave(&rx1gp, &rx1gp_len, zfp);
    get_wave(&zx1gp, &zx1gp_len, zfp);
    get_wave(&rx2gp, &rx2gp_len, zfp);
    get_wave(&zx2gp, &zx2gp_len, zfp);

    get_wave(&rx1gd, &rx1gd_len, zfp);
    get_wave(&zx1gd, &zx1gd_len, zfp);
    get_wave(&rx2gd, &rx2gd_len, zfp);
    get_wave(&zx2gd, &zx2gd_len, zfp);

    get_wave(&rx1gi, &rx1gi_len, zfp);
    get_wave(&zx1gi, &zx1gi_len, zfp);
    get_wave(&rx2gi, &rx2gi_len, zfp);
    get_wave(&zx2gi, &zx2gi_len, zfp);

    get_wave(&rx1taup, &rx1taup_len, zfp);
    get_wave(&zx1taup, &zx1taup_len, zfp);
    get_wave(&rx2taup, &rx2taup_len, zfp);
    get_wave(&zx2taup, &zx2taup_len, zfp);

    get_wave(&rx1taud, &rx1taud_len, zfp);
    get_wave(&zx1taud, &zx1taud_len, zfp);
    get_wave(&rx2taud, &rx2taud_len, zfp);
    get_wave(&zx2taud, &zx2taud_len, zfp);

    get_wave(&rx1taui, &rx1taui_len, zfp);
    get_wave(&zx1taui, &zx1taui_len, zfp);
    get_wave(&rx2taui, &rx2taui_len, zfp);
    get_wave(&zx2taui, &zx2taui_len, zfp);

    get_wave(&sztc, &sztc_len, zfp);
    get_wave(&szdt, &szdt_len, zfp);
    get_wave(&ipref, &ipref_len, zfp);

    fgets(zfbuf, ZFBUFLEN, zfp);
    sscanf(zfbuf, "%d", &vs1mode);
    fgets(zfbuf, ZFBUFLEN, zfp);
    sscanf(zfbuf, "%d", &vs2mode);

    fgets(zfbuf, ZFBUFLEN, zfp);
    sscanf(zfbuf, "%lf %lf", &vs1_st, &vs1_et);
    fgets(zfbuf, ZFBUFLEN, zfp);
    sscanf(zfbuf, "%lf %lf", &vs2_st, &vs2_et);

    fclose(zfp);
}

void interpolation_data(double start_time,double end_time,double clk,){

    int total_points = ((end_time - start_time) / clk) + 1;

    t_zx1ref = (double *)malloc(total_points * sizeof(double));
    liner(clk, zx1ref, zx1ref_len, limit_st, limit_et, t_zx1ref);
}


void clean_ztarget(){
    free(ivs1);
    free(ivs2);
    free(vvs1);
    free(vvs2);
    free(rx1ref);
    free(zx1ref);
    free(rx2ref);
    free(zx2ref);
    free(ipnorm);
    free(rx1gp);
    free(zx1gp);
    free(rx2gp);
    free(zx2gp);
    free(rx1gd);
    free(zx1gd);
    free(rx2gd);
    free(zx2gd);
    free(rx1gi);
    free(zx1gi);
    free(rx2gi);
    free(zx2gi);
    free(rx1taup);
    free(zx1taup);
    free(rx2taup);
    free(zx2taup);
    free(rx1taud);
    free(zx1taud);
    free(rx2taud);
    free(zx2taud);
    free(rx1taui);
    free(zx1taui);
    free(rx2taui);
    free(zx2taui);
    free(sztc);
    free(szdt);
    free(ipref);

    free(MMatrix);
    int i;
    for (i = 0; i < WhichE_len;i++){
        free(EMatrix[i]);
    }
    free(EMatrix);
}
#endif