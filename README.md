# Distributed auxiliary program for PCS of HL-2M

```
*********************************************
******██████╗  █████╗ ██╗   ██╗██╗  ██╗******
******██╔══██╗██╔══██╗██║   ██║╚██╗██╔╝******
******██║  ██║███████║██║   ██║ ╚███╔╝ ******
******██║  ██║██╔══██║██║   ██║ ██╔██╗ ******
******██████╔╝██║  ██║╚██████╔╝██╔╝ ██╗******
******╚═════╝ ╚═╝  ╚═╝ ╚═════╝ ╚═╝  ╚═╝******
***Distributed auxiliary for PCS of HL-2M ***
*********************************************
*******D-TACQ2106 Support & VS control*******
*********************************************
```

Distributed auxiliary(DAUX) involves D-TACQ 2106 and VS coils control. The latter consists coilcurrent and limited algorithms for PCS. It MUST simultaneously start with PCS.

## Environment

Daux requires two D-TACQ2106 cases and a reflective memories card installed. D-TACQ2106 input case uses AI1234AO5DO6 (aka 04_04_04_04_41_61) configruation. D-TACQ2106 output case uses AI1DO6 (04_61) configuration. Meanwhile, a remote MDSPlus server for archive is optional.

Check list for executing DAUX is followed:

1. Update /etc/hosts
   Adding 2106 case IP and MDSPlus server IP.
2. Check RFM network
   Ensuring that there are no nodes conflict and data transfer is available using rfm2g_utilt.
3. Mounting NFS
   Mounting control-manager:/gateway_disk -> /gateway_disk
   creating softlink /link -> /gateway_disk/data/
4. Modfing scrpit daux.c
   Making the `LLCHOST` and `DOHOST` fit custom host. Values of `devnum` and `do_devnum` is depend on ray transmit module slot order. A,B,C,D -> 0,1,2,3.
5. Checing corresponding variables between PCS and DAUX

| at PCS                                          | at DAUX                          |
| ----------------------------------------------- | -------------------------------- |
| installdefs.h**OFFSET_DAQ1**              | rfm_helper.h**OFFSET_DAQ** |
| msg2daux@infra/waveserver.c***server*** | LAN broadcast address            |
| msg2daux@infra/waveserver.c***port***   | daux.c***PORT***         |

6. MDSPlus server
   6.1 Establishing fastz_hl2m tree dir
   6.2 Running *mds_build_fastz_tree* to building model tree

> NOTICE: *mds_build_fastz_tree* is a script with IDL, So a way to execute is coping it to control manager.

7. DO device is disable by default. It could be enable by adding `-dDO` to `CFLAG` in Makefile and recompile.

## Useage

DAUX provides some optional arguments. However, default is enough for most conditions.

./daux [-OPTION]

-a          Programme CPU affinity. 01 by default.
-c          Execution cycle count by microsecond. 50 by default.
-z          Zfile path. /link/ops/data_hl2m/ztargrt.dat by default.
-n          Only save fastz data, do NOT save daq data. Disable by default.
-t          Test mode. Using /link/ops/data_hl2m/test_ztarget.dat and dummy data to call fastz. Disable by default.
-q          Only executing data acquisition, disable by default.

-q argument will block -n automatically.

## PCS parameter files

"Data refresh" button will trigger a series action include with generate ztartet file sent a shot info UDP broadcast massage. DAUX will replay an abracadabra to PCS after ready.

### ztarget file

The ztarget file is wave vertices and parameters exported by PCS waveserver. It is unstructured and organized by nature order, hence parser code MUST be adjusted carefully when ztarget file change. There is one or none data segment for each phasealg in ztarget file and it starts with its alg name.

Present ztarget order is followed:

| Data              | Format       | Comment                                         |
| ----------------- | ------------ | ----------------------------------------------- |
| phasealg          | char*        | *coilcurrent*                                 |
| cc duration       | float, float | cc duration                                     |
| IVS1              | waveform     |                                                 |
| IVS2              | waveform     |                                                 |
| VVS1              | waveform     |                                                 |
| VVS2              | waveform     |                                                 |
| phasealg          | char*        | *limited*                                     |
| limited duration  | float, float | limited duration if mode == 1                   |
| ZX1REF            | waveform     |                                                 |
| WhichMmatrices    | waveform     | STEP WAVE                                       |
| mmatrix[y1]       | float        | col:zx1,row:VS1                                 |
| …                | ...          |                                                 |
| mmatrix[ynum]     | float        |                                                 |
| whichEmatrices    | waveform     | STEP WAVE                                       |
| ematrix[y1]       | float array  | zx1 line data                                   |
| …                | ...          |                                                 |
| ematrix[ynum]     | float array  |                                                 |
| ZX1_GP            | waveform     |                                                 |
| ZX1_GD            | waveform     |                                                 |
| ZX1_GI            | waveform     |                                                 |
| ZX1_TAUP          | waveform     |                                                 |
| ZX1_TAUD          | waveform     |                                                 |
| ZX1_TAUI          | waveform     |                                                 |
| Slow Z time const | waveform     |                                                 |
| Slow Z delay time | waveform     |                                                 |
| phasealg          | char*        | *ipcontrol*                                   |
| IPREF             | waveform     | IPREF if IP exists                              |
| phasealg          | char*        | *faultdetection*                              |
| IPEOO             | waveform     | **DISCARD** IP ERROR detection on/off     |
| IPETL             | waveform     | **DISCARD** IP ERROR detection trip level |
| phasealg          | char*        | *sysmain*                                     |
| VS1 power mode    | int          | disable:0 voltage:1 current:2                   |
| VS2 power mode    | int          | disable:0 voltage:1 current:2                   |
| IP threshold      | float        |                                                 |
| DummyZX1          | waveform     | dummy data used in test mode                    |
| DummyIP           | waveform     | dummy data used in test mode                    |
| DummyIVS1         | waveform     | dummy data used in test mode                    |

### calib file

/link/ops/data_hl2m/calib.dat records calibration data setted at PCS baselinedata algorithm, the order is same with input_data.h.

## VS control

For this version, ONLY current control on VS1 is available.
A typical VS control process is a coilcurrent->RZIP sequence.
