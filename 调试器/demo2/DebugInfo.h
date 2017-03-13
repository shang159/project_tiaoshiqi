#pragma once
#include <windows.h>
#include "FileInfo.h"
const BYTE code = 0xCC;
#define PRINTFREGVAULE(def)   if (0 != def)				\
								printf("  在 线\r\n");	\
																												else					\
								printf("  失 效\r\n");  \


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
//DR7与宏定义得出一个值 来判断DR0 DR1 DR2 DR3执行 读或写  写断点
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
//INT3结构
struct BreakPoint
{
	BOOL  DelFlag;
	BYTE  b_code;
	DWORD BreakAddress;
};

//硬件断点处理标记
struct HareWareFlag
{
	BOOL DR0;        //当前寄存器使用状态
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

//内存断点处理标记
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
	BOOL bDelFlag;	      // 删除标记	
	BOOL bRWFlag;		  // READ   or  WRITE
	DWORD beginAddress;   //断点开始地址
	DWORD endAddress;     //断点终止地址
};

//添加用户断点表 <当前页>
struct MemPageBreakPoint
{
	DWORD  BeginPageAddress;			//分页开始地址
	DWORD  EndPageAddress;              //设置内存断点当前分页的最后终止地址
	DWORD  OldPageProtect;				//原来页的保护属性	
	CList<MemBPointInfor, MemBPointInfor> *RWMemBPList;  //保存当前分页中所有读地址断点 <段+偏移 （保存偏移）>   
};


//保存当前程序运行中的一条代码  主要用于单步和步过
struct SaveCurCode
{
	char szSrcCode;                     //保存原来的单个字节  CALL设置当前一个字节CC
	char szCurLineCode[4];              //保存当前机器码主要用来分析CALL指令 步过
	UINT nCodeSize;						//当前代码长度
	DWORD pStartAddress;                //程序入口地址
	DWORD pCurCodeEntryAddress;			//当前地址，执行完后保存下一条的地址首地址
	char szCurAsmBuf[64];				//当前程序指令
};

struct ExceptionFlag
{
	BOOL bGRunFlag;						//G命令处理标志
	BOOL bSetInFlag;					//单步进入处理标志
	BOOL bSetJumpFlag;					//单步跳过处理标志
	BOOL bHardWareSetinFlag;            //硬件断点引起的单步重设标记
	BOOL bHardWareFlag;                 //硬件标记
	BOOL bSetInt3Flag;                  //int3断点重置标记
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
	unsigned int m_StartMemAddr;          //一个分页中保存内存断点触发的异常地址(读or写)
	CList<BreakPoint, BreakPoint> m_myBreakList;
	CList<MemPageBreakPoint, MemPageBreakPoint> m_MemBPList;
	//保存内存链表用于显示   <仅仅用于显示内存链表信息个数>
	CList<MemBPointInfor, MemBPointInfor> m_ShowMemList;
	//脚本保存命令
	CList<CString, CString> m_MyCmdList;
};

