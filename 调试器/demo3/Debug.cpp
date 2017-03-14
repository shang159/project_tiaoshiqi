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
		printf_s("��ȡ�ļ���ʧ��");
	}
	STARTUPINFO si = { sizeof(STARTUPINFO) };
	BOOL bStatus = CreateProcess(szFileName, NULL, NULL, NULL, FALSE,
		DEBUG_ONLY_THIS_PROCESS | CREATE_NEW_CONSOLE,	//�����½����� | ӵ���¿���̨,���̳��丸������̨��Ĭ�ϣ�
		NULL, NULL, &si, &m_pi);
	if (!bStatus)
	{
		printf("�������Խ���ʧ��!\n");
		return;
	}
	//1.2	��ʼ�������¼��ṹ��
	DEBUG_EVENT DbgEvent = { 0 };
	DWORD dwState = DBG_EXCEPTION_NOT_HANDLED;
	//2.�ȴ�Ŀ��Exe���������¼�  
	while (1)
	{
		WaitForDebugEvent(&DbgEvent, INFINITE);
		//2.1 ���ݵ����¼�����,�ֱ���
		dwState = DispatchDbgEvent(DbgEvent);
		//2.2 �������쳣,�������б�����Exe
		ContinueDebugEvent(DbgEvent.dwProcessId, DbgEvent.dwThreadId, dwState);
	}

}
//�ַ������¼�
DWORD CDebug::DispatchDbgEvent(DEBUG_EVENT& de)
{
	OutputDebugString(L"�ַ������¼�\n");
	//�жϵ�������
	DWORD dwRet = DBG_EXCEPTION_NOT_HANDLED;
	switch (de.dwDebugEventCode)
	{
	case CREATE_PROCESS_DEBUG_EVENT:	//���̵���
		dwRet = OnCreateProcess(de);
		break;
	case EXCEPTION_DEBUG_EVENT:			//�쳣����
		dwRet = OnException(de);
		break;
	case CREATE_THREAD_DEBUG_EVENT:		//�̵߳���
	case EXIT_THREAD_DEBUG_EVENT:		//�˳��߳�
	case EXIT_PROCESS_DEBUG_EVENT:		//�˳�����
	case LOAD_DLL_DEBUG_EVENT:			//����DLL
		
		break;
	case UNLOAD_DLL_DEBUG_EVENT:		//ж��DLL
		//printf("UnLoad:%x\n", pDebugEvent->u.UnloadDll.lpBaseOfDll); 
		break;
	case OUTPUT_DEBUG_STRING_EVENT:		//��������ַ���
	case RIP_EVENT:						//RIP����
		return dwRet;	//������
	}
	return dwRet;
}

DWORD CDebug::OnCreateProcess(DEBUG_EVENT& de)
{

	OutputDebugString(L"�������Խ����¼�\n");
	DWORD dwRet = DBG_CONTINUE;
	// ������ģ��OEP�ϵ�
	DWORD dwOep = (DWORD)de.u.CreateProcessInfo.lpStartAddress;
	BYTE bOld = 0;
	BPINFO bi = {};
	bi.bt = BP_CC;
	bi.dwAddress = dwOep;
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, 0, de.dwProcessId);
	if (SetCcBreakPoint(hProcess, dwOep, bi.u.bCCOld))
	{
		// ����ϵ���Ϣ
		m_vecBp.push_back(bi);
		OutputDebugString(L"�ϵ���Ϣ�Ѿ�����\n");
	}
	else
	{
		// ���öϵ�ʧ��
		dwRet = DBG_EXCEPTION_NOT_HANDLED;
	}
	CloseHandle(hProcess);
	return dwRet;
}

DWORD CDebug::OnException(DEBUG_EVENT& de)
{
	OutputDebugString(L"�쳣�¼�\n");
	// ��ӡ�Ĵ�����Ϣ���쳣��ַ
	HANDLE hThread = OpenThread(THREAD_ALL_ACCESS, 0, de.dwThreadId);
	CONTEXT ct = {};
	ct.ContextFlags = CONTEXT_ALL;// all register
	GetThreadContext(hThread, &ct);
	CloseHandle(hThread);
	ShowRegisterInfo(ct);
	printf("Exception Addr:0x%08X\n", (DWORD)de.u.Exception.ExceptionRecord.ExceptionAddress);
	// �����쳣���ͷֱ���
	DWORD dwRet = DBG_CONTINUE;
	switch (de.u.Exception.ExceptionRecord.ExceptionCode)
	{
		//����ϵ�
	case EXCEPTION_BREAKPOINT:
		dwRet = OnExceptionCc(de);
		break;

		//�����쳣
	case EXCEPTION_SINGLE_STEP:
		dwRet = OnExceptionSingleStep(de);
		break;

		//�ڴ�����쳣
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
//����cc�ϵ�
BOOL CDebug::SetCcBreakPoint(HANDLE hProcess, DWORD dwAddress, BYTE& oldByte)
{
	BOOL bRet = TRUE;
	DWORD TmpLen = 0;		//����ʵ�ʶ�ȡ,д���ֽ�
	UCHAR tmp = 0;			//��ȡд����
	BYTE Int3 = 0xcc;		//cc�ϵ�
	//1. ����ԭ�ڴ�ҳ�ı������ԣ�ȷ����д��ȥ
	DWORD dwProtect = 0;
	VirtualProtectEx(hProcess, (LPVOID)dwAddress, 1, PAGE_READWRITE, &dwProtect);
	//2. ��ȡ���浱ǰ1�ֽ�����
	if (!ReadProcessMemory(hProcess, (LPVOID)(dwAddress), &oldByte, 1, &TmpLen))
		bRet = FALSE;
	//3. д��cc
	if (!WriteProcessMemory(hProcess, (LPVOID)(dwAddress), &Int3, 1, &TmpLen))
		bRet = FALSE;
	//4. ���ڴ�ҳ�ı�������д��ȥ
	VirtualProtectEx(hProcess, (LPVOID)dwAddress, 1, dwProtect, &dwProtect);
	return bRet;

}
VOID CDebug::ShowRegisterInfo(CONTEXT& ct)
{
	OutputDebugString(L"��ʾ�Ĵ�����Ϣ\n");
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
	OutputDebugString(L"����ϵ�\n");
	DWORD dwRet = DBG_CONTINUE;
	WaitforUserCommand(de);
	return dwRet;
}
DWORD CDebug::OnExceptionSingleStep(DEBUG_EVENT& de)
{
	OutputDebugString(L"�����ϵ�\n");
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
	OutputDebugString(L"�ڴ�����쳣\n");
	DWORD dwRet = DBG_EXCEPTION_NOT_HANDLED;
	//WaitforUserCommand();
	return dwRet;
}
void CDebug::WaitforUserCommand(DEBUG_EVENT& de)
{
	OutputDebugString(L"�ȴ��û�����\n");
	CHAR szCommand[MAX_INPUT] = {};
	while (true)
	{
		gets_s(szCommand, MAX_INPUT);
		switch (szCommand[0])
		{
		case 'u':// ����� ���������û�����
			UserCommandDisasm(szCommand);
			break;
		case 't':// ����F7 ��ǰ�����������û�����
			UserCommandF7(szCommand,de);
			return ;
		case 'p':// ����F8 ��ǰ�����������û�����
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
	//1. ���ڴ�ϵ��ֵд��ȥ����ֹӰ�췴���
	ResetAllCC(hProcess);
	//2. ���������ָ�� 
	char seps[] = " ";
	char *token = NULL;
	char *next_token = NULL;
	// token = 'u'
	token = strtok_s(pCommand, seps, &next_token);
	//2.1 ������ַ
	// token = address(123456)
	token = strtok_s(NULL, seps, &next_token);
	DWORD dwAddress = strtol(token, NULL, 16);
	//2.2 ���������
	// token = count(10)
	// token = strtok_s(NULL, seps, &next_token);
	// DWORD dwCount = strtol(token, NULL, 16);
	WCHAR szOpCode[50] = {};
	WCHAR szAsm[50] = {};
	WCHAR szComment[50] = {};
	//2.3 һ�η����1��,Ĭ�Ϸ����5���������Զ��巴���ָ����Ŀ��Ҳ��������������ָ��
	UINT uLen;
	for (int i = 0; i < 10; i++)
	{
		uLen = DBG_Disasm(hProcess, (PVOID)dwAddress, szOpCode, szAsm, szComment);
		wprintf_s(L"0x%08X %-16s%s\n", dwAddress, szOpCode, szAsm);
		dwAddress += uLen;
	}
	//3. �ָ����е�����ϵ�
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
	DWORD TmpLen = 0;		//����ʵ�ʶ�ȡ,д���ֽ�
	//1. ����ԭ�ڴ�ҳ�ı������ԣ�ȷ����д��ȥ
	DWORD dwProtect = 0;
	VirtualProtectEx(hProcess, (LPVOID)bi.dwAddress, 1, PAGE_READWRITE, &dwProtect);
	//2. ��ԭcc
	if (!WriteProcessMemory(hProcess, (LPVOID)(bi.dwAddress), &bi.u.bCCOld, 1, &TmpLen))
		bRet = FALSE;
	//3. ���ڴ�ҳ�ı�������д��ȥ
	VirtualProtectEx(hProcess, (LPVOID)bi.dwAddress, 1, dwProtect, &dwProtect);
	return bRet;
}
UINT CDebug::DBG_Disasm(HANDLE hProcess, LPVOID lpAddress, PWCHAR pOPCode, PWCHAR pASM, PWCHAR pComment)
{
	// 1. �����Գ�����ڴ渴�Ƶ�����
	DWORD  dwRetSize = 0;
	BYTE lpRemote_Buf[32] = {};
	ReadProcessMemory(hProcess, lpAddress, lpRemote_Buf, 32, &dwRetSize);
	// 2. ��ʼ�����������
	DISASM objDiasm;
	objDiasm.EIP = (UIntPtr)lpRemote_Buf; // ��ʼ��ַ
	objDiasm.VirtualAddr = (UINT64)lpAddress;     // �����ڴ��ַ��������������ڼ����ַ��
	objDiasm.Archi = 0;                     // AI-X86
	objDiasm.Options = 0x000;                 // MASM
	// 3. ��������
	UINT unLen = Disasm(&objDiasm);
	if (-1 == unLen) return unLen;
	// 4. ��������ת��Ϊ�ַ���
	LPWSTR lpOPCode = pOPCode;
	PBYTE  lpBuffer = lpRemote_Buf;
	for (UINT i = 0; i < unLen; i++)
	{
		StringCbPrintf(lpOPCode++, 50, L"%X", *lpBuffer & 0xF0);
		StringCbPrintf(lpOPCode++, 50, L"%X", *lpBuffer & 0x0F);
		lpBuffer++;
	}
	// 6. ���淴������ָ��
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
	//��eip��ǰ�ƶ�1��
	CONTEXT ct = {};
	ct.ContextFlags = CONTEXT_ALL;// all register
	GetThreadContext(hThread, &ct);

	EFLAGS* eflags = (EFLAGS*)&ct.EFlags;
	for (int i = 0; i < m_vecBp.size(); i++)
	{
		//�ж�cc���Լ��޸ĵĻ���ϵͳ�Դ���
		if ((DWORD)de.u.Exception.ExceptionRecord.ExceptionAddress==m_vecBp[i].dwAddress)
		{
			//������Լ����õ�  �����ǵ������е��˴�
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

			//������Լ����õ�  ������GO ���е��˴�
			else if ((DWORD)de.u.Exception.ExceptionRecord.ExceptionAddress + 1 == ct.Eip)
			{	//�ָ��ϵ�֮ǰ������
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
		//�����ϵͳ����ģ������õ����� 
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
	//��eip��ǰ�ƶ�1��
	CONTEXT ct = {};
	ct.ContextFlags = CONTEXT_ALL;// all register
	GetThreadContext(hThread, &ct);
	//�ж�cc���Լ��޸ĵĻ���ϵͳ�Դ���
	EFLAGS* eflags = (EFLAGS*)&ct.EFlags;
	for (int i = 0; i < m_vecBp.size(); i++)
	{
		
		if ((DWORD)de.u.Exception.ExceptionRecord.ExceptionAddress== m_vecBp[i].dwAddress)
		{	//�ָ��ϵ�֮ǰ������
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