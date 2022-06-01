#ifndef __VSCONTROL_H__
#define __VSCONTROL_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "daq_helper.h"
#include "calib.h"
#include "low_pass_filter.h"
#include "pidv4_cal.h"
#include "HL2MCal.h"

#define min(a,b) (((a) < (b)) ? (a) : (b))
#define max(a,b) (((a) > (b)) ? (a) : (b))

#define VS_DISABLE 0
#define VS_VOLTAGE 1
#define VS_CURRENT 2

#define E_MATRIX_LEN 93

#define WAVE_CONTINUOUS 0
#define WAVE_SETP 1

void fastz_do32(unsigned *xo, unsigned short cmd, unsigned long status);

typedef struct point
{
    float x;
    float y;
} Point;

/*
 * VS GLOBAL
 */
extern int istest;
int vs_cycle;
float vs_time;
float vs_interval;

int vs_samples;
float *vs_timebase;

//for HL2MCal
VS_Stc_In_Param s_in;
VS_Var_In_Param v_in;
VS_OUT_PARAM out;

/*
 * VS GLOBAL: ztarget file data
 */
int onoff;

//alg duration
float limit_st,limit_et; 
float cc_st, cc_et;

unsigned int ivs1_len, ivs2_len, vvs1_len, vvs2_len;
Point *ivs1, *ivs2, *vvs1, *vvs2;

//rz ref
unsigned int zx1ref_len;
Point *zx1ref;
//gp,gi,gd,tp,ti,td
unsigned int zx1gp_len, zx1gd_len, zx1gi_len, zx1tp_len, zx1td_len, zx1ti_len;
Point *zx1gp, *zx1gd, *zx1gi, *zx1tp, *zx1td, *zx1ti;

//MMatrix
unsigned int WhichM_len;
Point *WhichM;
float *MMatrix;

//EMatrix
unsigned int WhichE_len;
Point *WhichE;
float **EMatrix;//[][E_MATRIX_LEN]

//Slow Z time const,Slow Z delay time
unsigned int sztc_len,szdt_len;
Point *sztc, *szdt;

// IPREF, IpError On/Off, IpErrorTriplevel;
unsigned int ipref_len, ipeoo_len, ipetl_len;
Point *ipref, *ipeoo, *ipetl;

//VS Power configration
int vs1mode, vs2mode;
float vs1_st, vs1_et, vs2_st, vs2_et;
float ip_threshold;

//Test
unsigned int dummyZX1_len, dummyIP_len, dummyIVS1_len;
Point *dummyZX1, *dummyIP, *dummyIVS1;

//buffer for reading ztarget 
#define ZFBUFLEN 1024 * 2
char zfbuf[ZFBUFLEN];

/*
 * VS GLOBAL: real target
 */
//ztarget file data
float *ccivs1, *ccivs2, *ccvvs1, *ccvvs2;

float *lmzx1ref, *lmzx1gp, *lmzx1gd, *lmzx1gi, *lmzx1tp, *lmzx1td, *lmzx1ti, *lmszdt, *lmsztc, *lmWhichE, *lmWhichM, *lmipref, *lmipeoo, *lmipetl;
float *lmdummyZX1, *lmdummyIP, *lmdummyIVS1;
//middle var
float *zpos, *error, *derror, *pvector, *precmd;
//HL2MCal cmd
unsigned short *fincmd;
unsigned long *fincmd_bit;

#include "math.h"


/*
* FUNCTION: linear interpolation 
* linear interpolation and cut or extend to starttime to endtime section
* return length of data
*/
int linear(float dx, Point* vertics, unsigned int* len, float starttime, float endtime, float* data,int ADD_VERTICS)
{
    int i;

    if (vertics[0].x > endtime) {
        for (i = 0; i < vs_samples; i++) {
            data[i] = vertics[0].y;
        }
        return vs_samples;
    }
    if (vertics[*len - 1].x < starttime) {
        for (i = 0; i < vs_samples; i++) {
            data[i] = vertics[*len - 1].y;
        }
        return vs_samples;
    }
    int a;
    float k, b;

    if (starttime < vertics[0].x) {
        *len += 1;
        Point* newv = (Point*)malloc((*len) * sizeof(Point));
        for (i = 0; i < *len - 1; i++)
            newv[i + 1] = vertics[i];
        newv[0].x = starttime;
        newv[0].y = vertics[0].y;
        free(vertics);
        vertics = newv;
    }
    if (endtime > vertics[*len - 1].x) {
        Point* newv = (Point*)malloc((*len + 1) * sizeof(Point));
        for (i = 0; i < *len; i++)
            newv[i] = vertics[i];
        newv[*len].x = endtime;
        newv[*len].y = vertics[*len].y;
        free(vertics);
        vertics = newv;
        *len += 1;
    }
    if (starttime > vertics[0].x) {
        for (a = 0; a < *len; a++) {
            if (starttime < vertics[a + 1].x)
                break;
        }
        Point* newv = (Point*)malloc((*len - a) * sizeof(Point));

        for (i = 1; i < *len - a; i++) {
            newv[i] = vertics[a + i];
        }
        newv[0].x = starttime;
        if(ADD_VERTICS){
            newv[0].y=vertics[a].y;
        }        if(ADD_VERTICS){
            newv[0].y=vertics[a].y;
        }
        else{
            k = (vertics[a + 1].y - vertics[a].y) / (vertics[a + 1].x - vertics[a].x);
            b = vertics[a].y - k * vertics[a].x;
            newv[0].y = starttime * k + b;
        }


        free(vertics);
        vertics = newv;
        *len = *len - a;
    }
    if (endtime < vertics[*len - 1].x) {
        for (a = *len - 1; a > 0; a--) {
            if (endtime > vertics[a - 1].x)
                break;
        }
        Point* newv = (Point*)malloc((a + 1) * sizeof(Point));
        for (i = 0; i < a; i++) {
            newv[i] = vertics[i];
        }
        newv[a].x = endtime;
        if(ADD_VERTICS){
            newv[a].y = vertics[a - 1].y;
        }
        else{
            k = (vertics[a].y - vertics[a - 1].y) / (vertics[a].x - vertics[a - 1].x);
            b = vertics[a].y - k * vertics[a].x;
            newv[a].y = endtime * k + b;
        }

        free(vertics);
        vertics = newv;
        *len = a + 1;
    }

    int stages = *len - 1;
    int s;
    double time;
    double lastt = -10086;
    i = 0;
    for (s = 0; s < stages; s++) {
        if(ADD_VERTICS){
            k = 0;
            b = vertics[s].y;
        }
        else{
            k = (vertics[s + 1].y - vertics[s].y) / (vertics[s + 1].x - vertics[s].x);
            b = vertics[s].y - k * vertics[s].x;
        }

        for (time = vertics[s].x; time < vertics[s + 1].x; time += dx) {
            if ((time - lastt) < 1e-6) {
                continue;
            }
            data[i] = k * time + b;
            lastt = time;
            i++;
        }
        if (s == stages - 1) {
            data[i] = k * time + b;
        }
    }
    return i + 1;
}

/*
* FUNCTION: read a wave vertices set from file stream
*/
void get_wave(Point** wave,unsigned int* wnum,FILE* fp){
    int i;
    fgets(zfbuf, ZFBUFLEN, fp);
    sscanf(zfbuf, "%d", wnum);
    *wave = (Point *)malloc(*wnum * sizeof(Point));
    for (i = 0;i< *wnum; i++){
        fgets(zfbuf, ZFBUFLEN, fp);
        sscanf(zfbuf, "%f, %f\n", &((*wave)[i].x), &((*wave)[i].y));
    }
}

/*
* FUNCTION: read ztarget file
* return : is a limited algorithm exist
*/
int read_ztarget(char *zfile)
{
    int i;
    //printf("Open Z Target file : %s\n",zfile);
    FILE *zfp = fopen(zfile,"r");
    if (zfp == NULL){
        fprintf(stderr, "ERROR:Ztarget file missing.\n");
        exit(1);
    }
    
    fgets(zfbuf, ZFBUFLEN, zfp);
    sscanf(zfbuf, "%d", &onoff);
    fgets(zfbuf, ZFBUFLEN, zfp);
    sscanf(zfbuf, "%f, %f", &cc_st, &cc_et);
    get_wave(&ivs1, &ivs1_len, zfp);
    get_wave(&ivs2, &ivs2_len, zfp);
    get_wave(&vvs1, &vvs1_len, zfp);
    get_wave(&vvs2, &vvs2_len, zfp);

    if(onoff==0)
        ; //Skipping limit duration because it will not exist if there is no limit algorithm
    else{
        fgets(zfbuf, ZFBUFLEN, zfp);
        sscanf(zfbuf, "%f, %f", &limit_st, &limit_et);

        get_wave(&zx1ref, &zx1ref_len, zfp);

        get_wave(&WhichM, &WhichM_len, zfp);
        MMatrix = (float *)malloc(WhichM_len * sizeof(float));
        for (i = 0; i < WhichM_len;i++){
            fgets(zfbuf, ZFBUFLEN, zfp);
            sscanf(zfbuf, "%f", &(MMatrix[i]));
        }
    
        get_wave(&WhichE, &WhichE_len, zfp);
        EMatrix = (float **)malloc(WhichE_len * sizeof(float*));
        for (i = 0; i < WhichE_len;i++){
            EMatrix[i] = (float *)malloc(E_MATRIX_LEN * sizeof(float));
            fgets(zfbuf, ZFBUFLEN, zfp);
            int col = 0;
            char *token = strtok(zfbuf, ", ");
            while(token){
                sscanf(token, "%f", &EMatrix[i][col++]);
                token = strtok(NULL, ", ");
            }
        }
        
        get_wave(&zx1gp, &zx1gp_len, zfp);
        get_wave(&zx1gd, &zx1gd_len, zfp);
        get_wave(&zx1gi, &zx1gi_len, zfp);
        get_wave(&zx1tp, &zx1tp_len, zfp);
        get_wave(&zx1td, &zx1td_len, zfp);
        get_wave(&zx1ti, &zx1ti_len, zfp);

        get_wave(&sztc, &sztc_len, zfp);
        get_wave(&szdt, &szdt_len, zfp);
        get_wave(&ipref, &ipref_len, zfp);
        get_wave(&ipeoo, &ipeoo_len, zfp);
        get_wave(&ipetl, &ipetl_len, zfp);

        fgets(zfbuf, ZFBUFLEN, zfp);
        sscanf(zfbuf, "%d", &vs1mode);
        fgets(zfbuf, ZFBUFLEN, zfp);
        sscanf(zfbuf, "%d", &vs2mode);

        fgets(zfbuf, ZFBUFLEN, zfp);
        sscanf(zfbuf, "%f, %f", &vs1_st, &vs1_et);
        fgets(zfbuf, ZFBUFLEN, zfp);
        sscanf(zfbuf, "%f, %f", &vs2_st, &vs2_et);
        fgets(zfbuf, ZFBUFLEN, zfp);
        sscanf(zfbuf, "%f", &ip_threshold);

        get_wave(&dummyIP, &dummyIP_len, zfp);
        get_wave(&dummyZX1, &dummyZX1_len, zfp);
        get_wave(&dummyIVS1, &dummyIVS1_len, zfp);
    }
    fclose(zfp);
    return onoff;
}

/*
* FUNCTION: generate real data according vertices
*/

void cc_interpolation_data(float start_time,float end_time,float interval){
    int cc_total = (int)((vs1_et - vs1_st) / interval );

    ccivs1 = (float *)calloc(cc_total, sizeof(float));
    linear(interval, ivs1, &ivs1_len, cc_st, cc_et, ccivs1, WAVE_CONTINUOUS);
    
    ccivs2 = (float *)calloc(cc_total, sizeof(float));
    linear(interval, ivs2, &ivs2_len, cc_st, cc_et, ccivs2, WAVE_CONTINUOUS);
    
    ccvvs1 = (float *)calloc(cc_total, sizeof(float));
    linear(interval, vvs1, &vvs1_len, cc_st, cc_et, ccvvs1, WAVE_CONTINUOUS);
    
    ccvvs2 = (float *)calloc(cc_total, sizeof(float));
    linear(interval, vvs2, &vvs2_len, cc_st, cc_et, ccvvs2, WAVE_CONTINUOUS);
}

void lm_interpolation_data(float start_time,float end_time,float interval){
    int limited_total = (int)((vs1_et - vs1_st) / interval );

    lmzx1ref = (float *)calloc(limited_total, sizeof(float));
    linear(interval, zx1ref, &zx1ref_len, limit_st, limit_et, lmzx1ref, WAVE_CONTINUOUS);

    lmWhichE = (float *)calloc(limited_total, sizeof(float));
    linear(interval, WhichE, &WhichE_len, limit_st, limit_et, lmWhichE, WAVE_SETP);

    lmWhichM = (float *)calloc(limited_total, sizeof(float));
    linear(interval, WhichM, &WhichM_len, limit_st, limit_et, lmWhichM, WAVE_SETP);

    lmzx1gp = (float *)calloc(limited_total, sizeof(float));
    linear(interval, zx1gp, &zx1gp_len, limit_st, limit_et, lmzx1gp, WAVE_CONTINUOUS);
    
    lmzx1gd = (float *)calloc(limited_total, sizeof(float));
    linear(interval, zx1gd, &zx1gd_len, limit_st, limit_et, lmzx1gd, WAVE_CONTINUOUS);
    
    lmzx1gi = (float *)calloc(limited_total, sizeof(float));
    linear(interval, zx1gi, &zx1gi_len, limit_st, limit_et, lmzx1gi, WAVE_CONTINUOUS);
    
    lmzx1tp = (float *)calloc(limited_total, sizeof(float));
    linear(interval, zx1tp, &zx1tp_len, limit_st, limit_et, lmzx1tp, WAVE_CONTINUOUS);
    
    lmzx1td = (float *)calloc(limited_total, sizeof(float));
    linear(interval, zx1td, &zx1td_len, limit_st, limit_et, lmzx1td, WAVE_CONTINUOUS);
    
    lmzx1ti = (float *)calloc(limited_total, sizeof(float));
    linear(interval, zx1ti, &zx1ti_len, limit_st, limit_et, lmzx1ti, WAVE_CONTINUOUS);
    
    lmsztc = (float *)calloc(limited_total, sizeof(float));
    linear(interval, sztc, &sztc_len, limit_st, limit_et, lmsztc, WAVE_CONTINUOUS);
    
    lmszdt = (float *)calloc(limited_total, sizeof(float));
    linear(interval, szdt, &szdt_len, limit_st, limit_et, lmszdt, WAVE_CONTINUOUS);
    
    lmipref = (float *)calloc(limited_total, sizeof(float));
    linear(interval, ipref, &ipref_len, limit_st, limit_et, lmipref, WAVE_CONTINUOUS);
    
    lmipeoo = (float *)calloc(limited_total, sizeof(float));
    linear(interval, ipeoo, &ipeoo_len, limit_st, limit_et, lmipeoo, WAVE_SETP);
    
    lmipetl = (float *)calloc(limited_total, sizeof(float));
    linear(interval, ipetl, &ipetl_len, limit_st, limit_et, lmipetl, WAVE_CONTINUOUS);   
    
    lmdummyIP = (float *)calloc(limited_total, sizeof(float));
    linear(interval, dummyIP, &dummyIP_len, limit_st, limit_et, lmdummyIP, WAVE_CONTINUOUS);
    
    lmdummyZX1 = (float *)calloc(limited_total, sizeof(float));
    linear(interval, dummyZX1, &dummyZX1_len, limit_st, limit_et, lmdummyZX1, WAVE_CONTINUOUS);
    
    lmdummyIVS1 = (float *)calloc(limited_total, sizeof(float));
    linear(interval, dummyIVS1, &dummyIVS1_len, limit_st, limit_et, lmdummyIVS1, WAVE_CONTINUOUS);
}

/*
* FUNCTION: initiate fastz shot
*/
int fastz_init(double interval,char* zfile){
    read_ztarget(zfile);

    vs_interval = interval;
    vs_samples = (vs1_et - vs1_st) / interval;
    vs_timebase = (float *)malloc(vs_samples * sizeof(float));
    vs_cycle = 0;
    vs_time = 0;

    vs1_st = cc_st;
    vs1_et = cc_et;
    if(onoff){
        vs1_st = min(cc_st, limit_st);
        vs1_et = max(cc_et, limit_et);
    }

    printf("\tA CoilCurrent algorithm phase is from %fs to %fs.\n",cc_st,cc_et);
    cc_interpolation_data(vs1_st, vs1_et, interval);

    if(onoff){
        printf("\tA limited algorithm phase is from %fs to %fs.\n",limit_st,limit_et);
        lm_interpolation_data(vs1_st, vs1_et, interval);
    }

    zpos = (float *)calloc(vs_samples * sizeof(float));
    error = (float *)calloc(vs_samples * sizeof(float));
    derror = (float *)calloc(vs_samples * sizeof(float));
    pvector = (float *)calloc(vs_samples * sizeof(float));
    precmd = (float *)calloc(vs_samples * sizeof(float));
    scratch = (struct scratch_area *)malloc(sizeof(struct scratch_area));

    fincmd = (unsigned short *)calloc(vs_samples * sizeof(unsigned short));
    fincmd_bit = (unsigned long *)calloc(vs_samples * sizeof(unsigned long));

    // for HL2MCal
    CreatePwrAlgoInst();
    s_in.Start = (int)(vs1_st * 1000);
    s_in.End = (int)(vs1_et * 1000);
    s_in.Op_Mode = vs1mode;

    return onoff;
}

void fastz(short * rawdata,float ctime){
    int i;
    int ip_error;
    vs_time = ctime - limit_st;
    vs_timebase[vs_cycle] = ctime;
           
    for (i = 0; i < CHNS; i++){
        nominaldata[i] = BITS2RAW * calibfactor[i] * rawdata[i];
    }

    //Do CoilCurrent
    if(ctime > cc_st && ctime < cc_et){
        // VS1
        if(vs1mode==VS_DISABLE)
            precmd[vs_cycle] = 0;
        else if(vs1mode==VS_VOLTAGE)
            precmd[vs_cycle] = ccvvs1[vs_cycle];
        else if(vs1mode==VS_CURRENT)
            precmd[vs_cycle] = ccivs1[vs_cycle];
        ip_error = 0;
    }

    //Do Limited
    if(onoff){
        float ip;
        if(ctime > limit_st && ctime < limit_et){
            if(istest){//test mode
                ip = lmdummyIP[vs_cycle];
                precmd[vs_cycle] = lmdummyZX1[vs_cycle];
            }
            else{//normal mode
                int e = lmWhichE[vs_cycle];
                int m = lmWhichM[vs_cycle];
                ip = nominaldata[DAQ_RL01];

                if (fabs(ip) >= ip_threshold)
                    for (i = DAQ_FL01; i <= DAQ_MP20N;i++)
                       zpos[vs_cycle] += nominaldata[i] * EMatrix[e][i - DAQ_FL01 + EM_FL01];
                else
                   zpos[vs_cycle] = 0;

                zpos[vs_cycle] = zpos[vs_cycle] / ip;

                error[vs_cycle] = lmzx1ref[vs_cycle] - zpos[vs_cycle];
                derror[vs_cycle] = low_pass_filter(error[vs_cycle], lmsztc[vs_cycle], ctime);
                if ((lmsztc[vs_cycle] * 0.001) >= 1.0e-4){
                    float duration;
                    if (lbbtime == 0)
                        lbbtime = ctime;
                    duration = (ctime - lbbtime);
                    if (duration < (lmszdt[vs_cycle] * 0.001))
                        error[vs_cycle] = error[vs_cycle] - duration / (lmszdt[vs_cycle] * 0.001) * derror[vs_cycle];
                    else
                        error[vs_cycle] = error[vs_cycle] - derror[vs_cycle];
                }
                else
                    lbbtime = 0;
                pidv4(1, ctime, &in_calculate, &in_use_gi, &pvector[vs_cycle], in_shape, &error[vs_cycle], lmzx1gp[vs_cycle], lmzx1gd[vs_cycle],lmzx1gi[vs_cycle],  lmzx1tp[vs_cycle], lmzx1td[vs_cycle], lmzx1ti[vs_cycle], scratch);

                precmd[vs_cycle] = MMatrix[m] * pvector[vs_cycle];

                if(lmipeoo[vs_cycle]!=0)
                    if(fabs(lmipref[vs_cycle] - ip) >= lmipetl[vs_cycle])
                        ip_error = 1;
            }
        }
    }
    
    if(istest)
        v_in.IVS1 = lmdummyIVS1[vs_cycle];
    else
        v_in.IVS1 = nominaldata[DAQ_IVS1];
    v_in.Time = ctime * 1000;
    v_in.VS1_Ctrl_Cmd = precmd[vs_cycle];
    v_in.Error = ip_error;
    CalcVSParams(&s_in, &v_in, &out);

    fincmd[vs_cycle] = out.CmdVal;
    fincmd_bit[vs_cycle] = out.Status.vsVal;

    //fastz_do32(do_buffer, fincmd[vs_cycle], fincmd_bit[vs_cycle]);
    vs_cycle++; 
}


void save_fastz(int shot){
    //open_tree("mds-server", "fastz_hl2m", shot);
    int i;
     
    write_signal("VSEZX1", vs_timebase, error, vs_samples);
    write_signal("VSPZX1", vs_timebase, pvector, vs_samples);
    write_signal("VSCZX1", vs_timebase, precmd, vs_samples);

    float *tmp = (float *)malloc(vs_cycle * sizeof(float));
    for (i = 0; i < vs_cycle;i++){
        tmp[i] = fincmd[i];
    }
    write_signal("VSCVS1", vs_timebase, tmp, vs_samples);

    for (i = 0; i < vs_cycle;i++){
        tmp[i] = (fincmd_bit[i]|0x01);
    }
    write_signal("VSCVS1B",vs_timebase, tmp, vs_samples);

    for (i = 0; i < vs_cycle;i++){
        tmp[i] = (fincmd_bit[i]|0x02)>>1;
    }
    write_signal("VSCVS1V",vs_timebase, tmp, vs_samples);

    for (i = 0; i < vs_cycle;i++){
        tmp[i] = (fincmd_bit[i]|0x04)>>2;
    }
    write_signal("VSCVS1I",vs_timebase, tmp, vs_samples);
}

void fastz_clean(){
    //zfile
    free(ivs1);
    free(ivs2);
    free(vvs1);
    free(vvs2);
    free(zx1ref); 
    free(WhichE);
    free(WhichM); 
    free(zx1gp);
    free(zx1gd);
    free(zx1gi);
    free(zx1tp);
    free(zx1td);
    free(zx1ti);
    free(sztc);
    free(szdt);
    free(ipref);
    free(ipeoo);
    free(ipetl);
    free(dummyZX1);
    free(dummyIP);
    free(dummyIVS1);
    free(MMatrix);
   
    int i;
    for (i = 0; i < WhichE_len;i++){
        free(EMatrix[i]);
    }
    free(EMatrix);
    
    //real data
    free(ccivs1);
    free(ccivs2);
    free(ccvvs1);
    free(ccvvs2);
    free(lmzx1ref);
    free(lmWhichE);
    free(lmWhichM);
    free(lmzx1gp);
    free(lmzx1gd);
    free(lmzx1gi);
    free(lmzx1tp);
    free(lmzx1td);
    free(lmzx1ti);
    free(lmsztc);
    free(lmszdt);
    free(lmipref);
    free(lmipeoo);
    free(lmipetl);
    free(lmdummyIP);
    free(lmdummyZX1);
    free(lmdummyIVS1);
    free(vs_timebase);
    free(zpos);
    free(derror);
    free(error);
    free(pvector);
    free(precmd);
    free(fincmd);
    free(fincmd_bit);

    //reset lpf
    lp_elast = 0;
    lp_tlast = 0;
    
    lp_vlast = 0;
    lbbtime = 0;
    limited_init_cycle = 1;
    //reset pid
    scratch->dlast = 0;
    scratch->elast = 0;
    scratch->ilast = 0;
    scratch->tlast = 0;
    scratch->vlast = 0;

    DestroyPwrAlgoInst();
}

void fastz_do32(unsigned *xo, unsigned short cmd, unsigned long status){
	unsigned *do32 = (unsigned*) xo;
	unsigned data;
	data = cmd;
	unsigned long a_status = status << 16;
	data |= a_status;
	do32[DO_IX] = data;
}



#endif
