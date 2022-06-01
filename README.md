# Distributed auxiliary program for PCS of HL-2M

Distributed auxiliary(DAUX) involves D-TACQ 2106 and VS coils control including coilcurrent and limited algorithms for PCS.

## Environment

Daux requires D-TACQ2106 case and Reflective memories card installed. D-TACQ2106 uses AI1234AO5DO6 (aka 04_04_04_04_41_61) configruation. Meanwhile, a remote MDSPlus server for archive is optional.

Check list for executing DAUX is followed:

1. Update /etc/hosts
    Adding 2106 case IP and MDSPlus server IP.
2. Check RFM network
    Ensuring that there are no nodes conflict and data transfer is available using rfm2g_utilt.
3. Mounting NFS 
    control-manager:/gateway_list -> /gateway_list
    creating softlink /link -> /gateway_disk/data/
4. Modfing scrpit daux.c 
    Making the LLCHOST fit custom host.
5. Checing corresponding variables between PCS and DAUX

| at PCS                         | at DAUX                     |
| ------------------------------ | --------------------------- |
| installdefs.h  **OFFSET_DAQ1** | rfm_helper.h **OFFSET_DAQ** |
| etc/msg2daux.c  ***port***     | daux.c ***PORT***           |
| etc/msg2daux.c ***serv***      | localhost                   |

6. MDSPlus server
    6.1 Establishing fastz_hl2m tree dir
    6.2 Running *mds_build_fastz_tree* to building model tree

> NOTICE: *mds_build_fastz_tree* is a script with IDL, So a way to execute is coping it to control manager.

## Useage
DAUX privodes some optional arguments. However, default is enough for most conditions.

./daux [-OPTION] 

-a          Programme CPU affinity. 01 by default.
-c          Execution cycle count by microsecond. 50 by default.
-z          Zfile path. /link/ops/data_hl2m/ztargrt.dat by default.
-n          Only save fastz data, do NOT save daq data. Disable by default.
-t          Test mode. Using /link/ops/data_hl2m/test_ztarget.dat and dummy data to call fastz. Disable by default.

## PCS parameter file
"Data refresh" button in charge with gerenate ztartet file.

### ztarget file

The ztarget file is wave vertics and parameters exported by PCS waveserver. It is unstructured and organized by nature order, hence parser code MUST be adjusted carefully when ztarget file change.

Present ztarget order is followed:

| Data                | Format       | Comment                          |
| ------------------- | ------------ | -------------------------------- |
| mode                | int          | 0:no limited 1:limited           |
| cc      duration    | float, float | cc      duration                 |
| IVS1                | waveform     |                                  |
| IVS2                | waveform     |                                  |
| VVS1                | waveform     |                                  |
| VVS2                | waveform     |                                  |
| limited duration    | float, float | limited duration if mode == 1    |
| ZX1REF              | waveform     |                                  |
| WhichMmatrices      | waveform     | STEP WAVE                        |
| mmatrix[y1]         | float        | col:zx1,row:VS1                  |
| …                   | ...          |                                  |
| mmatrix[ynum]       | float        |                                  |
| whichEmatrices      | waveform     | STEP WAVE                        |
| ematrix[y1]         | float array  | zx1 line data                    |
| …                   | ...          |                                  |
| ematrix[ynum]       | float array  |                                  |
| ZX1_GP              | waveform     |                                  |
| ZX1_GD              | waveform     |                                  |
| ZX1_GI              | waveform     |                                  |
| ZX1_TAUP            | waveform     |                                  |
| ZX1_TAUD            | waveform     |                                  |
| ZX1_TAUI            | waveform     |                                  |
| Slow Z time const   | waveform     |                                  |
| Slow Z delay time   | waveform     |                                  |
| IPREF               | waveform     | IPREF at ipcontrol algorithm     |
| IpError_OnOff       | waveform     | Alarms    STEP WAVE              |
| IPError_triplevel   | waveform     | Alarms                           |
| VS1 power mode      | int          | disable:0 voltage:1 current:2    |
| VS2 power mode      | int          | disable:0 voltage:1 current:2    |
| VS1 duration        | float, float | PF7U duration *DISCARD*          |
| VS2 duration        | float, float | PF7L duration *DISCARD*          |
| IP threshold        | float        |                                  |
| DummyZX1            | waveform     |                                  |
| DummyIP             | waveform     |                                  |
| DummyIVS1           | waveform     |                                  |

### calib file
/link/ops/data_hl2m/calib.dat records calibration data setted at PCS baselinedata algorithm, the order is same with input_data.h.

## Workflow

A topical seq setting is as follow.
A CoilCurrent in ShotStart phase is required. Limited algorithm phase is optional.