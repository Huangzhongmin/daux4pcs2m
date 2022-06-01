/*
 * @Author: zhongmin.huang
 * @Date: 2022-04-28 14:35:28
 * @LastEditors: zhongmin.huang
 * @LastEditTime: 2022-05-09 09:19:30
 * @FilePath: \新建文件夹 (2)\low_pass_filter.h
 * @Description: 
 */
int limited_init_cycle = 1;
float lbbtime = 0;

float lp_vlast = 0;
float lp_elast = 0;
float lp_tlast = 0;

/* low_pass_filter*/
float low_pass_filter(float value, float f_tau, float t)
{
        float vout, qqp, tau, taupinverse;
        float time;

        tau = f_tau * 0.001;

        time = t;

        if (limited_init_cycle == 1)
        {
                lp_tlast = 0;
        }
        if (lp_tlast == 0)
        {
                lp_vlast = value;
                lp_elast = value;
                lp_tlast = time;
                vout = value;
        }
        else if (time != lp_tlast)
        {
                if (tau >= 1.0e-4)
                {
                        taupinverse = 1.0 / (2.0 * tau);
                        qqp = (time - lp_tlast) * taupinverse;
                        vout = (qqp * (value + lp_elast) - (qqp - 1.0) * lp_vlast) / (qqp + 1.0);
                }
                else
                {
                        vout = value;
                }
                if (vout < 1.0e-35 && vout > -1.0e-35)
                {
                        vout = 0.0;
                }

                lp_vlast = vout;
                lp_elast = value;
                lp_tlast = time;
        }
        else
        {
                vout = lp_vlast;
        }

        return vout;
}
