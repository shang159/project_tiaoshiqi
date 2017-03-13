#pragma once
#include <windows.h>
#include "FileInfo.h"
const BYTE code = 0xCC;
#define PRINTFREGVAULE(def)   if (0 != def)				\
								printf("  �� ��\r\n");	\
																												else					\
								printf("  ʧ Ч\r\n");  \


// OF IF TF SF ZF AF PF CF
// NV UP EI PL NZ NA PO NC
#define  _FLAG_OF 0x800
#define  _FLAG_IF 0x200
#define  _FLAG_TF 0x100
#define  _FLAG_SF 0x80
#define  _FLAG_ZF 0x40
#define  _FLAG_AF 0x10
#define  _FLAG_PF 0x4
#define  _FLAG_CF 0x1

///////////////////////////////////////////////////////////////////////
//DR7��궨��ó�һ��ֵ ���ж�DR0 DR1 DR2 DR3ִ�� ����д  д�ϵ�
//DR0
#define ONEDR0EVALUE 	0X301
#define ONEDR0WVALUE	0x10701
#define ONEDR0RWVALUE	0x30701

#define TWODR0EVALUE 	0X301
#define TWODR0WVALUE	0x50701
#define TWODR0RWVALUE	0x70701

#define FOURDR0EVALUE 	0X301
#define FOURDR0WVALUE	0x0D0701
#define FOURDR0RWVALUE	0x0F0701

//DR1
#define ONEDR1EVALUE 	0x304
#define ONEDR1WVALUE	0x100704
#define ONEDR1RWVALUE	0x300704

#define TWODR1EVALUE 	0x304
#define TWODR1WVALUE	0x500704
#define TWODR1RWVALUE	0x700704

#define FOURDR1EVALUE 	0x304
#define FOURDR1WVALUE	0x0D00704
#define FOURDR1RWVALUE	0x0F00704


//DR2
#define ONEDR2EVALUE 	0x310
#define ONEDR2WVALUE	0x1000710
#define ONEDR2RWVALUE	0x3000710

#define TWODR2EVALUE 	0x301
#define TWODR2WVALUE	0x5000710
#define TWODR2RWVALUE	0x7000710

#define FOURDR2EVALUE 	0x301
#define FOURDR2WVALUE	0x0D000710
#define FOURDR2RWVALUE	0x0F000710

//DR3
#define ONEDR3EVALUE 	0x340
#define ONEDR3WVALUE	0x10000740
#define ONEDR3RWVALUE	0x30000740

#define TWODR3EVALUE 	0x340
#define TWODR3WVALUE	0x50000740
#define TWODR3RWVALUE	0x70000740

#define FOURDR3EVALUE 	0x340
#define FOURDR3WVALUE	0x0D0000740
#define FOURDR3RWVALUE	0x0F0000740
///////////////////////////////////////////////////////////////////////



typedef HANDLE(__stdcall *pMyOpenThread)(DWORD dwDesiredAccess,  // access right
	BOOL bInheritHandle,    // handle inheritance option
	DWORD dwThreadId        // thread identifier
	);

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
//INT3�ṹ
struct BreakPoint
{
	BOOL  DelFlag;
	BYTE  b_code;
	DWORD BreakAddress;
};

//Ӳ���ϵ㴦����
struct HareWareFlag
{
	BOOL DR0;        //��ǰ�Ĵ���ʹ��״̬
	BOOL DR1;
	BOOL DR2;
	BOOL DR3;
	HareWareFlag()
	{
		DR0 = FALSE;
		DR1 = FALSE;
		DR2 = FALSE;
		DR3 = FALSE;
	}
};

//�ڴ�ϵ㴦����
struct MemBPointFlag
{
	BOOL bMemRead;
	BOOL bMemWrite;
	MemBPointFlag()
	{
		bMemRead = FALSE;
		bMemWrite = FALSE;
	}

};

#define READ  0
#define WRITE 1

struct MemBPointInfor
{
	BOOL bDelFlag;	      // ɾ�����	
	BOOL bRWFlag;		  // READ   or  WRITE
	DWORD beginAddress;   //�ϵ㿪ʼ��ַ
	DWORD endAddress;     //�ϵ���ֹ��ַ
};

//����û��ϵ�� <��ǰҳ>
struct MemPageBreakPoint
{
	DWORD  BeginPageAddress;			//��ҳ��ʼ��ַ
	DWORD  EndPageAddress;              //�����ڴ�ϵ㵱ǰ��ҳ�������ֹ��ַ
	DWORD  OldPageProtect;				//ԭ��ҳ�ı�������	
	CList<MemBPointInfor, MemBPointInfor> *RWMemBPList;  //���浱ǰ��ҳ�����ж���ַ�ϵ� <��+ƫ�� ������ƫ�ƣ�>   
};


//���浱ǰ���������е�һ������  ��Ҫ���ڵ����Ͳ���
struct SaveCurCode
{
	char szSrcCode;                     //����ԭ���ĵ����ֽ�  CALL���õ�ǰһ���ֽ�CC
	char szCurLineCode[4];              //���浱ǰ��������Ҫ��������CALLָ�� ����
	UINT nCodeSize;						//��ǰ���볤��
	DWORD pStartAddress;                //������ڵ�ַ
	DWORD pCurCodeEntryAddress;			//��ǰ��ַ��ִ����󱣴���һ���ĵ�ַ�׵�ַ
	char szCurAsmBuf[64];				//��ǰ����ָ��
};

struct ExceptionFlag
{
	BOOL bGRunFlag;						//G������־
	BOOL bSetInFlag;					//�������봦���־
	BOOL bSetJumpFlag;					//�������������־
	BOOL bHardWareSetinFlag;            //Ӳ���ϵ�����ĵ���������
	BOOL bHardWareFlag;                 //Ӳ�����
	BOOL bSetInt3Flag;                  //int3�ϵ����ñ��
	ExceptionFlag()
	{
		bGRunFlag = FALSE;
		bSetInFlag = FALSE;
		bSetJumpFlag = FALSE;
	}
};


class CDebugInfo
{
public:
	CDebugInfo();
	~CDebugInfo();
public:
	bool InitializeFile(WCHAR* wcPathName);
	bool CreateDebugProcess();
	void DebugProcessProc();
	void GetThreadInfo(DEBUG_EVENT DebugEv);
	void DisposeAccessException(EXCEPTION_RECORD RecordTmp);
private:
	CFileInfo m_hFileInfor;
	HANDLE m_hThread;
	CONTEXT m_Context;
	BOOL m_RWMemFlag;
	unsigned int m_StartMemAddr;          //һ����ҳ�б����ڴ�ϵ㴥�����쳣��ַ(��orд)
	CList<BreakPoint, BreakPoint> m_myBreakList;
	CList<MemPageBreakPoint, MemPageBreakPoint> m_MemBPList;
	//�����ڴ�����������ʾ   <����������ʾ�ڴ�������Ϣ����>
	CList<MemBPointInfor, MemBPointInfor> m_ShowMemList;
	//�ű���������
	CList<CString, CString> m_MyCmdList;
};

