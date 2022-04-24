#ifndef __READCALIB_H__
#define __READCALIB_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

int readcalib(char* file,float* calibs){
    FILE* fp = fopen(file, "r");
    char buf[128];
    char* lasts;
    int c = 0;
    while(fgets(buf,sizeof(buf),fp)!=NULL){
        if(buf[0]=='!' || isspace(buf[0]))
            continue;
    #ifdef ONLY_VS
        if(buf[2]!='m' && buf[2]!='f')
            continue;
    #endif
        else{
            lasts = strrchr(buf,' ');
            if(++lasts)
                sscanf(lasts,"%f",&calibs[i++]);
        }
    }
    fclose(fp);
    return c;
}

#endif