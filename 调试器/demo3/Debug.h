#pragma once
#include<windows.h>
#include <vector>
using std::vector;
#define MAX_INPUT 1024   // 控制台命令最大长度
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
class CDebug
{
private:
	typedef enum _EXCEPTIONTYPE
	{
		BP_CC,// 软件断点
		BP_HP,// 硬件断点
		BP_MM // 内存访问断点 
	}EXCEPTIONTYPE;
	typedef struct _BPINFO
	{
		EXCEPTIONTYPE bt;// 断点类型
		DWORD dwAddress; // 断点地址
		BOOL bOnce;		 // 一次性断点
		BOOL bSys;
		// 其他属性，比如内存断点还有读写，写，执行等等
		union
		{
			BYTE bCCOld;// 软件断点原始字节
			DWORD dwMemOld;// 内存访问断点原始属性
		}u;
	}BPINFO, *PBPINFO;

public:
	CDebug();
	~CDebug();
	void DebugMain();
	//分发调试事件
	DWORD DispatchDbgEvent(DEBUG_EVENT& de);
	// 创建进程事件
	DWORD OnCreateProcess(DEBUG_EVENT& de);
	// 异常调试事件
	DWORD OnException(DEBUG_EVENT& de);
	// 模块加载事件
	DWORD OnLoadDll(DEBUG_EVENT& de);


	// 设置CC软件断点
	BOOL SetCcBreakPoint(HANDLE hProcess, DWORD dwAddress, BYTE& oldByte);
	VOID ShowRegisterInfo(CONTEXT& ct);
	DWORD OnExceptionCc(DEBUG_EVENT& de);
	DWORD OnExceptionSingleStep(DEBUG_EVENT& de);
	DWORD OnExceptionAccess(DEBUG_EVENT& de);
	void WaitforUserCommand(DEBUG_EVENT& de);
	void UserCommandDisasm(CHAR* pCommand);
	void ResetAllCC(HANDLE hProcess);
	BOOL ResetCC(HANDLE hProcess, BPINFO &bi);
	UINT DBG_Disasm(HANDLE hProcess, LPVOID lpAddress, PWCHAR pOPCode, PWCHAR pASM, PWCHAR pComment);
	void SetallCC(HANDLE hProcess);
	void UserCommandF7(CHAR* pCommand);
	void UserCommandF8(CHAR* pCommand);
	void UserCommandGO(CHAR* pCommand, DEBUG_EVENT& de);
public:
	vector<BPINFO> m_vecBp;
	PROCESS_INFORMATION m_pi = {};
	
};

