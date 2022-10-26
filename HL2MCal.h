// The following ifdef block is the standard way of creating macros which make exporting
// from a DLL simpler. All files within this DLL are compiled with the HL2MCAL_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see
// HL2MCAL_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.

#ifndef _HL2MCAL_H_
#define _HL2MCAL_H_

#ifdef HL2MCAL_EXPORTS
#ifdef _WINDOWS
	#define HL2MCAL_API __declspec(dllexport)
	#else
	#define HL2MCAL_API 
	#endif // _WINDOWS

#else
#define HL2MCAL_API 
#endif


/////////////////返回异常码///////////////////////////////////
#define HL_2M_SUCCESS 0x0
#define HL_2M_INI 0x0001
#define HL_2M_INVALID_INPUT 0x003
#define HL_2M_NULL_OUT_POINTER 0x004
#define HL_2M_INVALID_HANDLE  0x005 
//#define HL_2M_SYSTEM_ERROR  0x006
//////////////////////////////////////////////////////////////

///////////////////算法实例最大数量///////////////////////////////////
#define PF_COUNT		16
////////////////////////////////////////////////////////////



#pragma pack(1)

/////// 电源输入参数////////
//
//
typedef struct _tagCS_Stc_In_Param
{
	int			Start;					//CS电源打开时间
	int			End;					//CS电源结束时间
	int			Op_Mode;				//0: 保护 1:单正组，2:单负组，3:正负组对接
	int			Zero_Check;				//CS电流检查区间的起始时间
	float		IcsPosHigh;					//CS正组全运行->部分运行的电流阈值
	float		IcsPosLow;					//CS正组部分运行->环流运行的电流阈值
	float		IcsNegLow;					//CS环流运行->负组部分运行的电流阈值
	float		IcsNegHigh;					//CS负组部分运行->全运行的电流阈值
	int         VCS;                        //CS电源线电压
	int         MGV4;                       //4号电机电压（用于计算控制命令）
}CS_Stc_In_Param, *PCS_Stc_In_Param;

typedef struct _tagCS_Var_In_Param
{
	float		Ics;						//CS线圈电流
	int			Time;						//PCS当前时间
	int			Ipbuild;					//Ip存在标志位
	float		V_Demand;					//PCS计算出需要的CS电压值
	int         Error;                      //Error存在标
}CS_Var_In_Param, *PCS_Var_In_Param;

typedef struct _tagPF_Stc_In_Param
{
	int			Start;					//PF电源打开时间
	int			End;					//PF电源结束时间
	int			Op_Mode;				//0: 保护 1:单正组，2:单负组，3:正负组对接
	float		IpfPosCir;					//PF正组部分运行->环流运行的电流最大阈值
	float		IpfNegCir;					//PF正组部分运行->环流运行的电流最小阈值
	float		VPF;					//CS电源线电压
	int         MGV4;                   //4号电机电压（用于计算控制命令）
}PF_Stc_In_Param, *PPF_Stc_In_Param;

typedef struct _tagPF_Var_In_Param
{
	float		Ipf;						//PF电源电流
	int			Time;						//PCS当前时间
	int			Ipbuild;					//Ip存在标志位
	float		V_Demand;					//PCS给的电压值
	int         Error;                      //Error存在标识
}PF_Var_In_Param, *PPF_Var_In_Param;

typedef struct _tagPwr_Stc_In_Param
{
	CS_Stc_In_Param		csPara;
	PF_Stc_In_Param     pfPara[PF_COUNT];
}Pwr_Stc_In_Param, *PPwr_Stc_In_Param; //

typedef struct _tagPwr_Var_In_Param
{
	CS_Var_In_Param		csPara;
	PF_Var_In_Param     pfPara[PF_COUNT];
}Pwr_Var_In_Param, *PPwr_Var_In_Param;

//PCS给VS接口静态参数定义
typedef struct _tagVS_Stc_In_Param
{
    int			Start;					//VS电源打开时间
    int			End;					//VS电源结束时间
    int			Op_Mode;				//0: 禁止 1:电压，2:电流
}VS_Stc_In_Param, *SVS_Stc_In_Param;
//PCS给VS接口动态参数定义
typedef struct _tagVS_Var_In_Param
{
	float		IVS1;					//VS电源电流
	float		Time;					//当前时间
	float		VS1_Ctrl_Cmd;				//控制值
	int         Error;                  //Error存在标识
}VS_Var_In_Param, *SVS_Var_In_Param;

// CS电源运行状态2字节16标志位转换
//目前仅考虑CS单次过零
typedef union
{
	unsigned long		usVal;
	struct
	{
		unsigned short		Pblock : 1;					//0位  正组封锁、接封锁信号。0 封锁；1 解封锁
		unsigned short		Nblock : 1;					//1位  负组封锁、接封锁信号。0 封锁；1 解封锁
        unsigned short		FullP : 1;                  //2位  正组全运行标志
		unsigned short		PartP : 1;					//3位  退出正组全运行->进入正组部分运行
		unsigned short		Circle : 1;					//4位  退出正组部分运行->进入环流运行
		unsigned short		PartN : 1;					//5位  退出环流运行->进入负组部分运行
		unsigned short		FullN : 1;					//6位  退出负组部分运行-->进入负组全运行
        unsigned long		Reserve : 25;				//保留
	};
}*pCSStatus, CSStatus;

// PF1U运行状态
//PF考虑多次过零
typedef union
{
	unsigned long   usVal;
	struct
	{
		unsigned short		Pblock : 1;					//0位	正组封锁/解锁信号
		unsigned short		Nblock : 1;					//1位	负组封锁/解锁信号
        unsigned short		FullP : 1;					//2位	正组全运行
        unsigned short		CircleP : 1;			    //3位	从正组进入环流运行（正组6脉波打开，负组6脉波打开）
        unsigned short		CircleN : 1;			    //4位	从负组进入环流运行（正组6脉波打开，负组6脉波打开）
        unsigned short		FullN : 1;					//5位	负组全运行
        unsigned long		Reserve : 26;				//保留
	};
}*pPFStatus, PFStatus;

// 扩展类型结构
typedef union
{
	unsigned long		vsVal;
	struct
	{
		unsigned short		sbit0 : 1;					//0位  
		unsigned short		sbit1 : 1;					//1位  
		unsigned short		sbit2 : 1;					//2位  
		unsigned long		Reserve : 29;				//保留
	};
}*pVSSbit, VSSBit;

typedef struct _tagPF_OUT_PARAM
{
	float				Pout;        //4 PF正组给定
	float				Nout;        //4 PF负组给定
	PFStatus			Status;		//4 PF标志位输出
}PF_OUT_PARAM, *pPF_OUT_PARAM;

// CS输出参数
typedef struct _tagCS_OUT_PARAM
{
	float				Pout;          //4 CS正组给定
	float				Nout;          //4 CS负组给定
	CSStatus			Status;		  //2 CS标志位输出
}CS_OUT_PARAM, *pCS_OUT_PARAM;

typedef struct _tagPwr_OUT_PARAM
{
	CS_OUT_PARAM        csOParam;					//CS输出参数
	PF_OUT_PARAM        pfOParam[PF_COUNT];			//PF输出参数
}Pwr_OUT_PARAM, *PPwr_OUT_PARAM;

//快控电源VS接口代码返回给PCS的参数
typedef struct _tagVS_OUT_PARAM
{   
	VSSBit          Status;				//1 0位:使能信号VS1_Block，1位：电压模式，2位：电流模式；
	unsigned short int 	CmdVal;         //2 输出给快控电源VS1的控制值
}VS_OUT_PARAM, *SVS_OUT_PARAM;

#pragma pack()

#ifdef __cplusplus
extern "C" {
#endif
	HL2MCAL_API int CreatePwrAlgoInst();

	HL2MCAL_API int CalcPwrParams(const PPwr_Stc_In_Param  pInStcVals, const PPwr_Var_In_Param pInVarVals, PPwr_OUT_PARAM pOutVals);

	HL2MCAL_API int CalcVSParams(const SVS_Stc_In_Param  pInStcVals, const SVS_Var_In_Param pInVarVals, SVS_OUT_PARAM pOutVals);

	HL2MCAL_API int DestroyPwrAlgoInst();

#ifdef __cplusplus
}
#endif

#endif // !_HL2MCAL_H_
