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
typedef struct _DBG_REG6
{
	//断点命中标志位，如果位于DR0-3的某个断点被命中，则进行异常处理前，对应的B0-3就会被置为1
	unsigned B0 : 1;//Dr0 断点触发置位
	unsigned B1 : 1;//Dr1 断点触发置位
	unsigned B2 : 1;//Dr2 断点触发置位
	unsigned B3 : 1;//Dr3 断点触发置位
	unsigned Reserve1: 9;//保留字段
	//其他状态字段
	unsigned BD : 1; //调试寄存器本身触发断点后，此位置被置为1
	unsigned BS : 1;//单步异常被触发，需要与寄存器EFLAGS的TF联合使用
	unsigned BT : 1;//此位与TSS的T标识联合使用，用于接收cpu任务切换异常
	unsigned Reserve2 : 16;//保留字段
}DBG_REG6, *PDBG_REG6;
typedef union  _DBG_REG7
{
	DWORD useDword;
	struct{
		//局部断点（L0-3）与全局断点（G0-3）的标志位
		unsigned L0 : 1;//对Dr0保存的地址启用局部断点
		unsigned G0 : 1;//对Dr0保存的地址启用全局断点
		unsigned L1 : 1;//对Dr1保存的地址启用局部断点
		unsigned G1 : 1;//对Dr1保存的地址启用全局断点
		unsigned L2 : 1;//对Dr2保存的地址启用局部断点
		unsigned G2 : 1;//对Dr2保存的地址启用全局断点
		unsigned L3 : 1;//对Dr3保存的地址启用局部断点
		unsigned G3 : 1;//对Dr3保存的地址启用全局断点
		//已经弃用，用于降低CPU频率，以方便准确检测断点异常
		unsigned LE : 1;
		unsigned GE : 1;
		unsigned Reserve1 : 3;//保留字段
		unsigned GD : 1;//保护调试寄存器标志位，如果此位为1，则有指令修改调试寄存器时会触发异常
		unsigned Reserve2 : 2;//保留字段
		//保存Dr0-Dr3地址所指向位置的断点类型（RW0-3）与断点长度（LEN0-3），状态描述如下
		unsigned RW0 : 2;  //设定Dr0指向地址的断点类型
		unsigned LEN0 : 2;//设定Dr0指向地址的长度
		unsigned RW1 : 2;//设定Dr1指向地址的断点类型
		unsigned LEN1 : 2;//设定Dr1指向地址的长度
		unsigned RW2 : 2;//设定Dr2指向地址的断点类型
		unsigned LEN2 : 2;//设定Dr2指向地址的长度
		unsigned RW3 : 2;//设定Dr3指向地址的断点类型
		unsigned LEN3 : 2;//设定Dr3指向地址的长度
	}useStruct;
}DBG_REG7, *PDBG_REG7;




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
	BOOL SetHkBreakPoint(HANDLE hProcess, DWORD dwAddress);
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
	void UserCommandHk(CHAR* szCommand, DEBUG_EVENT& de);
public:
	BOOL IsSingle=FALSE;
	vector<BPINFO> m_vecBp;
	PROCESS_INFORMATION m_pi = {};
	int ToDelOfHK = 0;
	BOOL isCmdgo;
	BOOL IsCmdF8=FALSE;
};

