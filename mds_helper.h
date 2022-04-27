#ifndef __MDS_HELPER_H__
#define __MDS_HELPER_H__

#include <string.h>
#include <mdslib.h>
#include <mdsshr.h>
#include <stdio.h>
#include <math.h>

#define statusOk(status) ((status) & 1)

int get_size(char* sigName){
    int dtype_long=DTYPE_LONG;
    int retSize;
    int lenDescr;
    int null=0;
    int status;
    char expression[1024];
    sprintf(expression,"SIZE(%s)",sigName);
    lenDescr=descr(&dtype_long,&retSize,&null);
    status=MdsValue(expression,&lenDescr,&null,0);
    if(!statusOk(status)){
        fprintf(stderr,"Error getting size of %s.\n",sigName);
        fprintf(stderr,"%s.\n",MdsGetMsg(status));
	return -1;
    } 
    return retSize;
}

void create_tree(char* host, char* db, int shot)
{
    int lens;
    int null = 0;
    int status;
    char cmd[64], cmd_tcl[96];
    int tstat;
    int dtype_long = DTYPE_LONG;
    int idesc = descr(&dtype_long, &tstat, &null);
    int socket;

    socket = MdsConnect(host);
    if (socket == -1)
        printf("Error connecting to mdsip server.\n");
    else
        printf("Success Connecting , socket:%d\n", socket);

    memset(cmd, 0, sizeof(cmd));
    memset(cmd_tcl, 0, sizeof(cmd_tcl));

    sprintf(cmd, "set tree %s",db);
    sprintf(cmd_tcl, "TCL(\"%s\",_output)", cmd);
    status = MdsValue(cmd_tcl, &idesc, &null, &lens);
    if (!statusOk(status))
        printf(" failure.\n");
    
    sprintf(cmd, "create pulse %d", shot);
    sprintf(cmd_tcl, "TCL(\"%s\",_output)", cmd);
    status = MdsValue(cmd_tcl, &idesc, &null, &lens);
    if (!statusOk(status))
        printf(" failure.\n");
    
    sprintf(cmd, "close");
    sprintf(cmd_tcl, "TCL(\"%s\",_output)", cmd);
    status = MdsValue(cmd_tcl, &idesc, &null, &lens);
    if (!statusOk(status))
        printf(" failure.\n");

    printf("create tree %d successfully!\n", shot);
    status = MdsClose(db, &shot);
    if (!statusOk(status))
        fprintf(stderr, "Error closing tree for shot %d: %s.\n", shot, MdsGetMsg(status));
}

int open_tree(char* serv, char* tree, int shot){
    int socket;
    int status;
    socket = MdsConnect(serv);
    if ( socket == -1 ){
        fprintf(stderr,"Error connecting to mdsip server.\n");
        return -1;
    }
    status = MdsOpen(tree, &shot);
    if ( !statusOk(status) ){
        fprintf(stderr,"Error opening tree for shot %d: %s.\n",shot, MdsGetMsg(status));
        return -1;
    }
    return socket;
}

void write_signal(char* node, float* x, float* y, int len){
    int dtypeFloat = DTYPE_FLOAT; 
    int null = 0; 
    int status; 
    int dataDesc; 
    int timeDesc; 

    dataDesc = descr(&dtypeFloat, y, &len, &null);
    timeDesc = descr(&dtypeFloat, x, &len, &null);

    status = MdsPut(node, "BUILD_SIGNAL($1,,$2)", &dataDesc, &timeDesc,&null);
    if(!statusOk(status))
        fprintf(stderr,"Error writing signal: %s\n", MdsGetMsg(status));
    
}

#endif
