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
	// 其他属性，比如内存断点还有读写，写，执行等等
	union
	{
		BYTE bCCOld;// 软件断点原始字节
		DWORD dwMemOld;// 内存访问断点原始属性
	}u;
}BPINFO, *PBPINFO;
typedef struct _EFLAGS
{
	unsigned CF : 1;  // 进位或错位
	unsigned Reservel1 : 1;  // 对Dr0保存的地址启用 全局断点
	unsigned PF : 1;  // 计算结果低位包含偶数个数1时 此标志位1
	unsigned Reservel2 : 1;  // 对Dr0保存的地址启用 全局断点
	unsigned AF : 1;  // 辅助进位标志 当位3处   有进位或结尾时 该标志为1
	unsigned Reservel3 : 1;  // 保留
	unsigned ZF : 1;  // 计算结果为0时 此标志位1
	unsigned SF : 1;  // 符号标志 计算结果为负时 该标志位1
	unsigned TF : 1;  // 陷阱标志 此标志为1时 CPU每次仅会执行一条指令
	unsigned IF : 1;  // 中断标志 为0时禁止响应（屏蔽中断）  为1回复
	unsigned DF : 1;  // 方向标志
	unsigned OF : 1;  // 溢出标志 计算结果超过表达范围为1 否则为0
	unsigned IOPL : 2;  //用于标明当前任务的I/0特权级
	unsigned NT : 1;  // 任务嵌套标志
	unsigned Reservel4 : 1;  // 对Dr0保存的地址启用 全局断点
	unsigned RF : 1;  // 调试异常相应标志位 为1禁止相应指令断点异常
	unsigned VM : 1;  // 为1时启用虚拟8086模式
	unsigned AC : 1;  // 内存对齐检查标志
	unsigned VIF : 1;  // 虚拟中断标志
	unsigned VIP : 1;  // 虚拟中断标志
	unsigned ID : 1;  // cpuID检查标志
	unsigned Reservel5 : 1;  // 保留 
}EFLAGS, *PEFLAGS;
class CDebug
{
private:
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
	void UserCommandF7(CHAR* pCommand, DEBUG_EVENT& de);
	void UserCommandF8(CHAR* pCommand, DEBUG_EVENT& de);
	void UserCommandGO(CHAR* pCommand, DEBUG_EVENT& de);
	void UserCommandBk(CHAR* pCommand, DEBUG_EVENT& de);
public:
	BOOL IsSingle=FALSE;
	vector<BPINFO> m_vecBp;
	PROCESS_INFORMATION m_pi = {};
	BOOL isCmdgo;
	BOOL IsCmdF8=FALSE;
};

