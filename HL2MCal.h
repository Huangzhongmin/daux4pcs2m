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


/////////////////�����쳣��///////////////////////////////////
#define HL_2M_SUCCESS 0x0
#define HL_2M_INI 0x0001
#define HL_2M_INVALID_INPUT 0x003
#define HL_2M_NULL_OUT_POINTER 0x004
#define HL_2M_INVALID_HANDLE  0x005 
//#define HL_2M_SYSTEM_ERROR  0x006
//////////////////////////////////////////////////////////////

///////////////////�㷨ʵ���������///////////////////////////////////
#define PF_COUNT		16
////////////////////////////////////////////////////////////



#pragma pack(1)

/////// ��Դ�������////////
//
//
typedef struct _tagCS_Stc_In_Param
{
	int			Start;					//CS��Դ��ʱ��
	int			End;					//CS��Դ����ʱ��
	int			Op_Mode;				//0: ���� 1:�����飬2:�����飬3:������Խ�
	int			Zero_Check;				//CS��������������ʼʱ��
	float		IcsPosHigh;					//CS����ȫ����->�������еĵ�����ֵ
	float		IcsPosLow;					//CS���鲿������->�������еĵ�����ֵ
	float		IcsNegLow;					//CS��������->���鲿�����еĵ�����ֵ
	float		IcsNegHigh;					//CS���鲿������->ȫ���еĵ�����ֵ
	int         VCS;                        //CS��Դ�ߵ�ѹ
	int         MGV4;                       //4�ŵ����ѹ�����ڼ���������
}CS_Stc_In_Param, *PCS_Stc_In_Param;

typedef struct _tagCS_Var_In_Param
{
	float		Ics;						//CS��Ȧ����
	int			Time;						//PCS��ǰʱ��
	int			Ipbuild;					//Ip���ڱ�־λ
	float		V_Demand;					//PCS�������Ҫ��CS��ѹֵ
	int         Error;                      //Error���ڱ�
}CS_Var_In_Param, *PCS_Var_In_Param;

typedef struct _tagPF_Stc_In_Param
{
	int			Start;					//PF��Դ��ʱ��
	int			End;					//PF��Դ����ʱ��
	int			Op_Mode;				//0: ���� 1:�����飬2:�����飬3:������Խ�
	float		IpfPosCir;					//PF���鲿������->�������еĵ��������ֵ
	float		IpfNegCir;					//PF���鲿������->�������еĵ�����С��ֵ
	float		VPF;					//CS��Դ�ߵ�ѹ
	int         MGV4;                   //4�ŵ����ѹ�����ڼ���������
}PF_Stc_In_Param, *PPF_Stc_In_Param;

typedef struct _tagPF_Var_In_Param
{
	float		Ipf;						//PF��Դ����
	int			Time;						//PCS��ǰʱ��
	int			Ipbuild;					//Ip���ڱ�־λ
	float		V_Demand;					//PCS���ĵ�ѹֵ
	int         Error;                      //Error���ڱ�ʶ
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

//PCS��VS�ӿھ�̬��������
typedef struct _tagVS_Stc_In_Param
{
    int			Start;					//VS��Դ��ʱ��
    int			End;					//VS��Դ����ʱ��
    int			Op_Mode;				//0: ��ֹ 1:��ѹ��2:����
}VS_Stc_In_Param, *SVS_Stc_In_Param;
//PCS��VS�ӿڶ�̬��������
typedef struct _tagVS_Var_In_Param
{
	float		IVS1;					//VS��Դ����
	float		Time;					//��ǰʱ��
	float		VS1_Ctrl_Cmd;				//����ֵ
	int         Error;                  //Error���ڱ�ʶ
}VS_Var_In_Param, *SVS_Var_In_Param;

// CS��Դ����״̬2�ֽ�16��־λת��
//Ŀǰ������CS���ι���
typedef union
{
	unsigned long		usVal;
	struct
	{
		unsigned short		Pblock : 1;					//0λ  ����������ӷ����źš�0 ������1 �����
		unsigned short		Nblock : 1;					//1λ  ����������ӷ����źš�0 ������1 �����
        unsigned short		FullP : 1;                  //2λ  ����ȫ���б�־
		unsigned short		PartP : 1;					//3λ  �˳�����ȫ����->�������鲿������
		unsigned short		Circle : 1;					//4λ  �˳����鲿������->���뻷������
		unsigned short		PartN : 1;					//5λ  �˳���������->���븺�鲿������
		unsigned short		FullN : 1;					//6λ  �˳����鲿������-->���븺��ȫ����
        unsigned long		Reserve : 25;				//����
	};
}*pCSStatus, CSStatus;

// PF1U����״̬
//PF���Ƕ�ι���
typedef union
{
	unsigned long   usVal;
	struct
	{
		unsigned short		Pblock : 1;					//0λ	�������/�����ź�
		unsigned short		Nblock : 1;					//1λ	�������/�����ź�
        unsigned short		FullP : 1;					//2λ	����ȫ����
        unsigned short		CircleP : 1;			    //3λ	��������뻷�����У�����6�����򿪣�����6�����򿪣�
        unsigned short		CircleN : 1;			    //4λ	�Ӹ�����뻷�����У�����6�����򿪣�����6�����򿪣�
        unsigned short		FullN : 1;					//5λ	����ȫ����
        unsigned long		Reserve : 26;				//����
	};
}*pPFStatus, PFStatus;

// ��չ���ͽṹ
typedef union
{
	unsigned long		vsVal;
	struct
	{
		unsigned short		sbit0 : 1;					//0λ  
		unsigned short		sbit1 : 1;					//1λ  
		unsigned short		sbit2 : 1;					//2λ  
		unsigned long		Reserve : 29;				//����
	};
}*pVSSbit, VSSBit;

typedef struct _tagPF_OUT_PARAM
{
	float				Pout;        //4 PF�������
	float				Nout;        //4 PF�������
	PFStatus			Status;		//4 PF��־λ���
}PF_OUT_PARAM, *pPF_OUT_PARAM;

// CS�������
typedef struct _tagCS_OUT_PARAM
{
	float				Pout;          //4 CS�������
	float				Nout;          //4 CS�������
	CSStatus			Status;		  //2 CS��־λ���
}CS_OUT_PARAM, *pCS_OUT_PARAM;

typedef struct _tagPwr_OUT_PARAM
{
	CS_OUT_PARAM        csOParam;					//CS�������
	PF_OUT_PARAM        pfOParam[PF_COUNT];			//PF�������
}Pwr_OUT_PARAM, *PPwr_OUT_PARAM;

//��ص�ԴVS�ӿڴ��뷵�ظ�PCS�Ĳ���
typedef struct _tagVS_OUT_PARAM
{   
	VSSBit          Status;				//1 0λ:ʹ���ź�VS1_Block��1λ����ѹģʽ��2λ������ģʽ��
	unsigned short int 	CmdVal;         //2 �������ص�ԴVS1�Ŀ���ֵ
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
