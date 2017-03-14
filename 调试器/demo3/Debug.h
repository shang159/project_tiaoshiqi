#pragma once
#include<windows.h>
#include <vector>
using std::vector;
#define MAX_INPUT 1024   // ����̨������󳤶�
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
		BP_CC,// ����ϵ�
		BP_HP,// Ӳ���ϵ�
		BP_MM // �ڴ���ʶϵ� 
	}EXCEPTIONTYPE;
	typedef struct _BPINFO
	{
		EXCEPTIONTYPE bt;// �ϵ�����
		DWORD dwAddress; // �ϵ��ַ
		BOOL bOnce;		 // һ���Զϵ�
		BOOL bSys;
		// �������ԣ������ڴ�ϵ㻹�ж�д��д��ִ�еȵ�
		union
		{
			BYTE bCCOld;// ����ϵ�ԭʼ�ֽ�
			DWORD dwMemOld;// �ڴ���ʶϵ�ԭʼ����
		}u;
	}BPINFO, *PBPINFO;

public:
	CDebug();
	~CDebug();
	void DebugMain();
	//�ַ������¼�
	DWORD DispatchDbgEvent(DEBUG_EVENT& de);
	// ���������¼�
	DWORD OnCreateProcess(DEBUG_EVENT& de);
	// �쳣�����¼�
	DWORD OnException(DEBUG_EVENT& de);
	// ģ������¼�
	DWORD OnLoadDll(DEBUG_EVENT& de);


	// ����CC����ϵ�
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

