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
	// �������ԣ������ڴ�ϵ㻹�ж�д��д��ִ�еȵ�
	union
	{
		BYTE bCCOld;// ����ϵ�ԭʼ�ֽ�
		DWORD dwMemOld;// �ڴ���ʶϵ�ԭʼ����
	}u;
}BPINFO, *PBPINFO;
typedef struct _EFLAGS
{
	unsigned CF : 1;  // ��λ���λ
	unsigned Reservel1 : 1;  // ��Dr0����ĵ�ַ���� ȫ�ֶϵ�
	unsigned PF : 1;  // ��������λ����ż������1ʱ �˱�־λ1
	unsigned Reservel2 : 1;  // ��Dr0����ĵ�ַ���� ȫ�ֶϵ�
	unsigned AF : 1;  // ������λ��־ ��λ3��   �н�λ���βʱ �ñ�־Ϊ1
	unsigned Reservel3 : 1;  // ����
	unsigned ZF : 1;  // ������Ϊ0ʱ �˱�־λ1
	unsigned SF : 1;  // ���ű�־ ������Ϊ��ʱ �ñ�־λ1
	unsigned TF : 1;  // �����־ �˱�־Ϊ1ʱ CPUÿ�ν���ִ��һ��ָ��
	unsigned IF : 1;  // �жϱ�־ Ϊ0ʱ��ֹ��Ӧ�������жϣ�  Ϊ1�ظ�
	unsigned DF : 1;  // �����־
	unsigned OF : 1;  // �����־ ������������ﷶΧΪ1 ����Ϊ0
	unsigned IOPL : 2;  //���ڱ�����ǰ�����I/0��Ȩ��
	unsigned NT : 1;  // ����Ƕ�ױ�־
	unsigned Reservel4 : 1;  // ��Dr0����ĵ�ַ���� ȫ�ֶϵ�
	unsigned RF : 1;  // �����쳣��Ӧ��־λ Ϊ1��ֹ��Ӧָ��ϵ��쳣
	unsigned VM : 1;  // Ϊ1ʱ��������8086ģʽ
	unsigned AC : 1;  // �ڴ�������־
	unsigned VIF : 1;  // �����жϱ�־
	unsigned VIP : 1;  // �����жϱ�־
	unsigned ID : 1;  // cpuID����־
	unsigned Reservel5 : 1;  // ���� 
}EFLAGS, *PEFLAGS;
class CDebug
{
private:
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

