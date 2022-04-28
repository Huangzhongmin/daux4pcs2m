#include<stdio.h>
#include<math.h>


struct scratch_area
        {
        float tlast;
        float ilast;
        float dlast;
        float vlast;
        float elast;
        };

struct scratch_area *scratch;
int in_calculate = 1;
int in_use_gi = 1;
float in_shape[3];

void pidv4( int numpvelem,float time,int *calculate,int *use_gi,float *pvector,float *shape,float *error,float GP,float GD,float GI,float TAUP,float TAUD,float TAUI,struct scratch_area *scratch)
{

	float dt,vout,dout,iout,qqp,qqd,qqi,gp,gd,gi,taup,taud,taui,taupinverse,taudinverse,tauiinverse;
	int i;	
//	struct scratch_area scratch[100];
	
	for(i=0;i<numpvelem;i++)
	{
	if(calculate[i]==0)
		{scratch[i].tlast=0;}

	gp=GP;
	gd=GD;
	gi=GI;
	
	if(scratch[i].tlast==0)
	{
	scratch[i].ilast =0.0;
	scratch[i].dlast =0.0;
	scratch[i].vlast = error[i];
	scratch[i].elast = error[i];
	scratch[i].tlast = time;
	vout = error[i];
	dout = 0.0;
	iout = 0.0;
	}	

     if(time != scratch[i].tlast)
      {
	taup = TAUP;
	taud = TAUD;
	taui = TAUI;
	
	if(taup !=0.0)
	 	{taupinverse = 1.0/(2.0*1.0e6*taup);}
	
	else
		taupinverse =0.0;

	if(taud == 0.0)
		taud =1.0e-6;

	if(taui == 0.0)
		taui = 1.0e-6;
	taudinverse = 1.0 / (2.0 * 1.0e6 * taud);
        tauiinverse = 1.0 / (2.0 * 1.0e6 * taui);
	
	dt = (time - scratch[i].tlast);
        qqp = dt * taupinverse;
        qqd = dt * taudinverse;
        qqi = dt * tauiinverse;

//V(t) = P1 * E(t) + P1 * E(tlast) - P2 * V(tlast)
//D(t) = D1 * V(t) - (D1 * V(tlast) + D2 * D(tlast))
	if(taup != 0.0)
        	vout = (qqp * (error[i] + scratch[i].elast) -(qqp - 1.0f) * scratch[i].vlast) / (qqp + 1.0f);
        else
              	vout = error[i];
	 if(vout < 1.0e-35 && vout > -1.0e-35) 
		vout = 0.0;
	 
	 dout = (vout - scratch[i].vlast -(qqd - 1.0f) * scratch[i].dlast) / (qqd + 1.0f);
	 if(dout < 1.0e-35 && dout > -1.0e-35) 
		dout = 0.0;
//I(t) = I1 * V(t) + I1 * V(tlast) - I2 * I(tlast)
	 
	if(gi == 0.0 || use_gi[i] == 0)
        	iout = 0.0;
        else if(use_gi[i] == 2)
           	iout = scratch[i].ilast;
	
	else if( (use_gi[i] == 3) &&( ((vout > 0.0) && (scratch[i].ilast >= 0.0)) ||((vout < 0.0) && (scratch[i].ilast <= 0.0)) ) )
		iout = scratch[i].ilast;
   
	else if( (use_gi[i] ==  4) && (vout <= 0.0) )
              	iout = 0.0;
        else if( (use_gi[i] ==  5) && (vout >= 0.0) )
              	iout = 0.0;
           
        else
           {
              iout = (qqi * (vout + scratch[i].vlast) -(qqi - 1.0f) * scratch[i].ilast) / (qqi + 1.0f);
              if(iout < 1.0e-35 && iout > -1.0e-35) 
			iout = 0.0;
           }

	scratch[i].ilast = iout;
        scratch[i].dlast = dout;
        scratch[i].vlast = vout;
        scratch[i].elast = error[i];
        scratch[i].tlast = time;
       }
	else
	{
          vout = scratch[i].vlast;
          dout = scratch[i].dlast;
          iout = scratch[i].ilast;
	}

	pvector[i] = gp * vout + gd * dout + gi * iout;

        if(shape != NULL)
        {
           shape[i*3 + 0] = vout;
           shape[i*3 + 1] = dout;
           shape[i*3 + 2] = iout;
        }

	}
//printf("ok!\n");
}

