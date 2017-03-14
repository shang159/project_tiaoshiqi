#include "Debug.h"
#include <stdio.h>
#define BEA_ENGINE_STATIC
#define BEA_USE_STDCALL
#include "Bea/headers/BeaEngine.h"
#include <strsafe.h>
#pragma comment(lib, "Bea/Win32/Lib/BeaEngine.lib")
#pragma comment(linker, "/NODEFAULTLIB:\"crt.lib\"")

CDebug::CDebug()
{
}


CDebug::~CDebug()
{
}
void CDebug::DebugMain()
{
	WCHAR        szFileName[MAX_PATH] = L"";
	OPENFILENAME ofn;
	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.lpstrFile = szFileName;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrFilter = L"Exe Files(*.exe)\0*.exe\0All Files(*.*)\0*.*\0\0";
	ofn.nFilterIndex = 1;
	if (!GetOpenFileName(&ofn))
	{
		printf_s("获取文件名失败");
	}
	STARTUPINFO si = { sizeof(STARTUPINFO) };
	BOOL bStatus = CreateProcess(szFileName, NULL, NULL, NULL, FALSE,
		DEBUG_ONLY_THIS_PROCESS | CREATE_NEW_CONSOLE,	//调试新建进程 | 拥有新控制台,不继承其父级控制台（默认）
		NULL, NULL, &si, &m_pi);
	if (!bStatus)
	{
		printf("创建调试进程失败!\n");
		return;
	}
	//1.2	初始化调试事件结构体
	DEBUG_EVENT DbgEvent = { 0 };
	DWORD dwState = DBG_EXCEPTION_NOT_HANDLED;
	//2.等待目标Exe产生调试事件  
	while (1)
	{
		WaitForDebugEvent(&DbgEvent, INFINITE);
		//2.1 根据调试事件类型,分别处理
		dwState = DispatchDbgEvent(DbgEvent);
		//2.2 处理完异常,继续运行被调试Exe
		ContinueDebugEvent(DbgEvent.dwProcessId, DbgEvent.dwThreadId, dwState);
	}

}
//分发调试事件
DWORD CDebug::DispatchDbgEvent(DEBUG_EVENT& de)
{
	OutputDebugString(L"分发调试事件\n");
	//判断调试类型
	DWORD dwRet = DBG_EXCEPTION_NOT_HANDLED;
	switch (de.dwDebugEventCode)
	{
	case CREATE_PROCESS_DEBUG_EVENT:	//进程调试
		dwRet = OnCreateProcess(de);
		break;
	case EXCEPTION_DEBUG_EVENT:			//异常调试
		dwRet = OnException(de);
		break;
	case CREATE_THREAD_DEBUG_EVENT:		//线程调试
	case EXIT_THREAD_DEBUG_EVENT:		//退出线程
	case EXIT_PROCESS_DEBUG_EVENT:		//退出进程
	case LOAD_DLL_DEBUG_EVENT:			//加载DLL
		
		break;
	case UNLOAD_DLL_DEBUG_EVENT:		//卸载DLL
		//printf("UnLoad:%x\n", pDebugEvent->u.UnloadDll.lpBaseOfDll); 
		break;
	case OUTPUT_DEBUG_STRING_EVENT:		//输出调试字符串
	case RIP_EVENT:						//RIP调试
		return dwRet;	//不处理
	}
	return dwRet;
}

DWORD CDebug::OnCreateProcess(DEBUG_EVENT& de)
{

	OutputDebugString(L"创建调试进程事件\n");
	DWORD dwRet = DBG_CONTINUE;
	// 设置主模块OEP断点
	DWORD dwOep = (DWORD)de.u.CreateProcessInfo.lpStartAddress;
	BYTE bOld = 0;
	BPINFO bi = {};
	bi.bt = BP_CC;
	bi.dwAddress = dwOep;
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, 0, de.dwProcessId);
	if (SetCcBreakPoint(hProcess, dwOep, bi.u.bCCOld))
	{
		// 保存断点信息
		m_vecBp.push_back(bi);
		OutputDebugString(L"断点信息已经保存\n");
	}
	else
	{
		// 设置断点失败
		dwRet = DBG_EXCEPTION_NOT_HANDLED;
	}
	CloseHandle(hProcess);
	return dwRet;
}

DWORD CDebug::OnException(DEBUG_EVENT& de)
{
	OutputDebugString(L"异常事件\n");
	// 打印寄存器信息，异常地址
	HANDLE hThread = OpenThread(THREAD_ALL_ACCESS, 0, de.dwThreadId);
	CONTEXT ct = {};
	ct.ContextFlags = CONTEXT_ALL;// all register
	GetThreadContext(hThread, &ct);
	CloseHandle(hThread);
	ShowRegisterInfo(ct);
	printf("Exception Addr:0x%08X\n", (DWORD)de.u.Exception.ExceptionRecord.ExceptionAddress);
	// 根据异常类型分别处理
	DWORD dwRet = DBG_CONTINUE;
	switch (de.u.Exception.ExceptionRecord.ExceptionCode)
	{
		//软件断点
	case EXCEPTION_BREAKPOINT:
		dwRet = OnExceptionCc(de);
		break;

		//单步异常
	case EXCEPTION_SINGLE_STEP:
		dwRet = OnExceptionSingleStep(de);
		break;

		//内存访问异常
	case EXCEPTION_ACCESS_VIOLATION:
		dwRet = OnExceptionAccess(de);
		break;
	default:
		break;
	}

	return dwRet;
}

DWORD CDebug::OnLoadDll(DEBUG_EVENT& de)
{
	DWORD dwRet = DBG_EXCEPTION_NOT_HANDLED;
	printf_s("dll\n");
	return dwRet;
}
//设置cc断点
BOOL CDebug::SetCcBreakPoint(HANDLE hProcess, DWORD dwAddress, BYTE& oldByte)
{
	BOOL bRet = TRUE;
	DWORD TmpLen = 0;		//返回实际读取,写入字节
	UCHAR tmp = 0;			//读取写入结果
	BYTE Int3 = 0xcc;		//cc断点
	//1. 更改原内存页的保护属性，确保能写进去
	DWORD dwProtect = 0;
	VirtualProtectEx(hProcess, (LPVOID)dwAddress, 1, PAGE_READWRITE, &dwProtect);
	//2. 读取保存当前1字节数据
	if (!ReadProcessMemory(hProcess, (LPVOID)(dwAddress), &oldByte, 1, &TmpLen))
		bRet = FALSE;
	//3. 写入cc
	if (!WriteProcessMemory(hProcess, (LPVOID)(dwAddress), &Int3, 1, &TmpLen))
		bRet = FALSE;
	//4. 把内存页的保护属性写回去
	VirtualProtectEx(hProcess, (LPVOID)dwAddress, 1, dwProtect, &dwProtect);
	return bRet;

}
VOID CDebug::ShowRegisterInfo(CONTEXT& ct)
{
	OutputDebugString(L"显示寄存器信息\n");
	printf("EAX = %08X ", ct.Eax);
	printf("ECX = %08X ", ct.Ecx);
	printf("EDX = %08X ", ct.Edx);
	printf("EBX = %08X ", ct.Ebx);
	printf("ESP = %08X \r\n", ct.Esp);
	printf("EBP = %08X ", ct.Ebp);
	printf("ESI = %08X ", ct.Esi);
	printf("EDI = %08X ", ct.Edi);
	printf("EIP = %08X \r\n", ct.Eip);
	printf("CS=%04X  DS=%04X  ES=%04X  SS=%04X  FS=%04X\t",
		ct.SegCs, ct.SegDs,
		ct.SegEs, ct.SegSs,
		ct.SegFs);
	printf("OF IF TF SF ZF AF PF CF\r\n");
	printf("\t\t\t\t\t\t%02d ", (ct.EFlags & _FLAG_OF) ? 1 : 0);
	printf("%02d ", (ct.EFlags & _FLAG_IF) ? 1 : 0);
	printf("%02d ", (ct.EFlags & _FLAG_TF) ? 1 : 0);
	printf("%02d ", (ct.EFlags & _FLAG_SF) ? 1 : 0);
	printf("%02d ", (ct.EFlags & _FLAG_ZF) ? 1 : 0);
	printf("%02d ", (ct.EFlags & _FLAG_AF) ? 1 : 0);
	printf("%02d ", (ct.EFlags & _FLAG_PF) ? 1 : 0);
	printf("%02d \r\n", (ct.EFlags & _FLAG_CF) ? 1 : 0);



// 	printf(
// 		"EAX = 0x%x\tEBX = 0x%x\tECX = 0x%x\tEDX = 0x%x\t\n"
// 		"ESP = 0x%x\tEBP = 0x%x\tESI = 0x%x\tEIP = 0x%x\t\n",
// 		ct.Eax, ct.Ebx, ct.Ecx, ct.Edx, ct.Esp, ct.Ebp, ct.Esi, ct.Eip
// 		);
}

DWORD CDebug::OnExceptionCc(DEBUG_EVENT& de)
{
	OutputDebugString(L"软件断点\n");
	DWORD dwRet = DBG_CONTINUE;
	WaitforUserCommand(de);
	return dwRet;
}
DWORD CDebug::OnExceptionSingleStep(DEBUG_EVENT& de)
{
	OutputDebugString(L"单步断点\n");
	DWORD dwRet = DBG_CONTINUE;
	IsSingle = TRUE;
	if (isCmdgo)
	{
		return dwRet;
	}
	WaitforUserCommand(de);
	return dwRet;
}
DWORD CDebug::OnExceptionAccess(DEBUG_EVENT& de)
{
	OutputDebugString(L"内存访问异常\n");
	DWORD dwRet = DBG_EXCEPTION_NOT_HANDLED;
	//WaitforUserCommand();
	return dwRet;
}
void CDebug::WaitforUserCommand(DEBUG_EVENT& de)
{
	OutputDebugString(L"等待用户命令\n");
	CHAR szCommand[MAX_INPUT] = {};
	while (true)
	{
		gets_s(szCommand, MAX_INPUT);
		switch (szCommand[0])
		{
		case 'u':// 反汇编 继续接受用户命令
			UserCommandDisasm(szCommand);
			break;
		case 't':// 单步F7 当前不继续接受用户命令
			UserCommandF7(szCommand,de);
			return ;
		case 'p':// 单步F8 当前不继续接受用户命令
			UserCommandF8(szCommand);
			return;
		case 'g':// go
			UserCommandGO(szCommand, de);
			return;
		default:
			break;
		}
	}
}
void CDebug::UserCommandDisasm(CHAR* pCommand)
{
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, 0, m_pi.dwProcessId);
	//1. 把内存断点的值写回去，防止影响反汇编
	ResetAllCC(hProcess);
	//2. 解析反汇编指令 
	char seps[] = " ";
	char *token = NULL;
	char *next_token = NULL;
	// token = 'u'
	token = strtok_s(pCommand, seps, &next_token);
	//2.1 反汇编地址
	// token = address(123456)
	token = strtok_s(NULL, seps, &next_token);
	DWORD dwAddress = strtol(token, NULL, 16);
	//2.2 反汇编行数
	// token = count(10)
	// token = strtok_s(NULL, seps, &next_token);
	// DWORD dwCount = strtol(token, NULL, 16);
	WCHAR szOpCode[50] = {};
	WCHAR szAsm[50] = {};
	WCHAR szComment[50] = {};
	//2.3 一次反汇编1条,默认反汇编5条，可以自定义反汇编指令数目，也可以由输入命令指定
	UINT uLen;
	for (int i = 0; i < 10; i++)
	{
		uLen = DBG_Disasm(hProcess, (PVOID)dwAddress, szOpCode, szAsm, szComment);
		wprintf_s(L"0x%08X %-16s%s\n", dwAddress, szOpCode, szAsm);
		dwAddress += uLen;
	}
	//3. 恢复所有的软件断点
	SetallCC(hProcess);
	CloseHandle(hProcess);
}
void CDebug::ResetAllCC(HANDLE hProcess)
{
	for (auto each : m_vecBp)
	{
		if (each.bt == BP_CC)
		{
			ResetCC(hProcess, each);
		}
	}
}
BOOL CDebug::ResetCC(HANDLE hProcess, BPINFO &bi)
{
	BOOL bRet = TRUE;
	DWORD TmpLen = 0;		//返回实际读取,写入字节
	//1. 更改原内存页的保护属性，确保能写进去
	DWORD dwProtect = 0;
	VirtualProtectEx(hProcess, (LPVOID)bi.dwAddress, 1, PAGE_READWRITE, &dwProtect);
	//2. 还原cc
	if (!WriteProcessMemory(hProcess, (LPVOID)(bi.dwAddress), &bi.u.bCCOld, 1, &TmpLen))
		bRet = FALSE;
	//3. 把内存页的保护属性写回去
	VirtualProtectEx(hProcess, (LPVOID)bi.dwAddress, 1, dwProtect, &dwProtect);
	return bRet;
}
UINT CDebug::DBG_Disasm(HANDLE hProcess, LPVOID lpAddress, PWCHAR pOPCode, PWCHAR pASM, PWCHAR pComment)
{
	// 1. 将调试程序的内存复制到本地
	DWORD  dwRetSize = 0;
	BYTE lpRemote_Buf[32] = {};
	ReadProcessMemory(hProcess, lpAddress, lpRemote_Buf, 32, &dwRetSize);
	// 2. 初始化反汇编引擎
	DISASM objDiasm;
	objDiasm.EIP = (UIntPtr)lpRemote_Buf; // 起始地址
	objDiasm.VirtualAddr = (UINT64)lpAddress;     // 虚拟内存地址（反汇编引擎用于计算地址）
	objDiasm.Archi = 0;                     // AI-X86
	objDiasm.Options = 0x000;                 // MASM
	// 3. 反汇编代码
	UINT unLen = Disasm(&objDiasm);
	if (-1 == unLen) return unLen;
	// 4. 将机器码转码为字符串
	LPWSTR lpOPCode = pOPCode;
	PBYTE  lpBuffer = lpRemote_Buf;
	for (UINT i = 0; i < unLen; i++)
	{
		StringCbPrintf(lpOPCode++, 50, L"%X", *lpBuffer & 0xF0);
		StringCbPrintf(lpOPCode++, 50, L"%X", *lpBuffer & 0x0F);
		lpBuffer++;
	}
	// 6. 保存反汇编出的指令
	WCHAR szASM[50] = { 0 };
	MultiByteToWideChar(CP_ACP, 0, objDiasm.CompleteInstr, -1, szASM, _countof(szASM));
	StringCchCopy(pASM, 50, szASM);
	return unLen;
}
void CDebug::SetallCC(HANDLE hProcess)
{
	for (auto each : m_vecBp)
	{
		if (each.bt == BP_CC)
		{
			SetCcBreakPoint(hProcess, each.dwAddress, each.u.bCCOld);
		}
	}
}
void CDebug::UserCommandF7(CHAR* pCommand, DEBUG_EVENT& de)
{
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, 0, m_pi.dwProcessId);
	HANDLE hThread = OpenThread(THREAD_ALL_ACCESS, 0, de.dwThreadId);
	//将eip向前移动1个
	CONTEXT ct = {};
	ct.ContextFlags = CONTEXT_ALL;// all register
	GetThreadContext(hThread, &ct);

	EFLAGS* eflags = (EFLAGS*)&ct.EFlags;
	for (int i = 0; i < m_vecBp.size(); i++)
	{
		//判断cc是自己修改的还是系统自带的
		if ((DWORD)de.u.Exception.ExceptionRecord.ExceptionAddress==m_vecBp[i].dwAddress)
		{
			//如果是自己设置的  并且是单步运行到此处
			if ((DWORD)de.u.Exception.ExceptionRecord.ExceptionAddress == ct.Eip)
			{
				ResetAllCC(hProcess);
				eflags->TF = 1;
				SetThreadContext(hThread, &ct);
				if (IsSingle)
				{
					SetallCC(hProcess);
					IsSingle = false;
				}
			}

			//如果是自己设置的  并且是GO 运行到此处
			else if ((DWORD)de.u.Exception.ExceptionRecord.ExceptionAddress + 1 == ct.Eip)
			{	//恢复断点之前的内容
				ct.Eip--;
				ResetAllCC(hProcess);
				eflags->TF = 1;
				SetThreadContext(hThread, &ct);
				if (IsSingle)
				{
					SetallCC(hProcess);
					IsSingle = false;
				}
			}
		} 		
		//如果是系统自身的，则设置单步。 
		else
		{
			eflags->TF = 1;
			SetThreadContext(hThread, &ct);
		}
	}
	CloseHandle(hProcess);
	CloseHandle(hThread);

}
void CDebug::UserCommandF8(CHAR* pCommand)
{

}
void CDebug::UserCommandGO(CHAR* pCommand, DEBUG_EVENT& de )
{
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, 0, m_pi.dwProcessId);
	HANDLE hThread = OpenThread(THREAD_ALL_ACCESS, 0, de.dwThreadId);
	//将eip向前移动1个
	CONTEXT ct = {};
	ct.ContextFlags = CONTEXT_ALL;// all register
	GetThreadContext(hThread, &ct);
	//判断cc是自己修改的还是系统自带的
	EFLAGS* eflags = (EFLAGS*)&ct.EFlags;
	for (int i = 0; i < m_vecBp.size(); i++)
	{
		
		if ((DWORD)de.u.Exception.ExceptionRecord.ExceptionAddress== m_vecBp[i].dwAddress)
		{	//恢复断点之前的内容
			ct.Eip--;
			ResetAllCC(hProcess);
			eflags->TF = 1;
			SetThreadContext(hProcess, &ct);
			if (IsSingle)
			{
				SetallCC(hProcess);
				isCmdgo = TRUE;
			}
			
		}
	}
	CloseHandle(hProcess);
	CloseHandle(hThread);
}