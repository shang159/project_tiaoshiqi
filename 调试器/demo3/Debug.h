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
typedef struct _DBG_REG6
{
	//�ϵ����б�־λ�����λ��DR0-3��ĳ���ϵ㱻���У�������쳣����ǰ����Ӧ��B0-3�ͻᱻ��Ϊ1
	unsigned B0 : 1;//Dr0 �ϵ㴥����λ
	unsigned B1 : 1;//Dr1 �ϵ㴥����λ
	unsigned B2 : 1;//Dr2 �ϵ㴥����λ
	unsigned B3 : 1;//Dr3 �ϵ㴥����λ
	unsigned Reserve1: 9;//�����ֶ�
	//����״̬�ֶ�
	unsigned BD : 1; //���ԼĴ����������ϵ�󣬴�λ�ñ���Ϊ1
	unsigned BS : 1;//�����쳣����������Ҫ��Ĵ���EFLAGS��TF����ʹ��
	unsigned BT : 1;//��λ��TSS��T��ʶ����ʹ�ã����ڽ���cpu�����л��쳣
	unsigned Reserve2 : 16;//�����ֶ�
}DBG_REG6, *PDBG_REG6;
typedef union  _DBG_REG7
{
	DWORD useDword;
	struct{
		//�ֲ��ϵ㣨L0-3����ȫ�ֶϵ㣨G0-3���ı�־λ
		unsigned L0 : 1;//��Dr0����ĵ�ַ���þֲ��ϵ�
		unsigned G0 : 1;//��Dr0����ĵ�ַ����ȫ�ֶϵ�
		unsigned L1 : 1;//��Dr1����ĵ�ַ���þֲ��ϵ�
		unsigned G1 : 1;//��Dr1����ĵ�ַ����ȫ�ֶϵ�
		unsigned L2 : 1;//��Dr2����ĵ�ַ���þֲ��ϵ�
		unsigned G2 : 1;//��Dr2����ĵ�ַ����ȫ�ֶϵ�
		unsigned L3 : 1;//��Dr3����ĵ�ַ���þֲ��ϵ�
		unsigned G3 : 1;//��Dr3����ĵ�ַ����ȫ�ֶϵ�
		//�Ѿ����ã����ڽ���CPUƵ�ʣ��Է���׼ȷ���ϵ��쳣
		unsigned LE : 1;
		unsigned GE : 1;
		unsigned Reserve1 : 3;//�����ֶ�
		unsigned GD : 1;//�������ԼĴ�����־λ�������λΪ1������ָ���޸ĵ��ԼĴ���ʱ�ᴥ���쳣
		unsigned Reserve2 : 2;//�����ֶ�
		//����Dr0-Dr3��ַ��ָ��λ�õĶϵ����ͣ�RW0-3����ϵ㳤�ȣ�LEN0-3����״̬��������
		unsigned RW0 : 2;  //�趨Dr0ָ���ַ�Ķϵ�����
		unsigned LEN0 : 2;//�趨Dr0ָ���ַ�ĳ���
		unsigned RW1 : 2;//�趨Dr1ָ���ַ�Ķϵ�����
		unsigned LEN1 : 2;//�趨Dr1ָ���ַ�ĳ���
		unsigned RW2 : 2;//�趨Dr2ָ���ַ�Ķϵ�����
		unsigned LEN2 : 2;//�趨Dr2ָ���ַ�ĳ���
		unsigned RW3 : 2;//�趨Dr3ָ���ַ�Ķϵ�����
		unsigned LEN3 : 2;//�趨Dr3ָ���ַ�ĳ���
	}useStruct;
}DBG_REG7, *PDBG_REG7;




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

