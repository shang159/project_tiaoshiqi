#include "stdafx.h"
#include "DebugInfo.h"


CDebugInfo::CDebugInfo()
{
}


CDebugInfo::~CDebugInfo()
{
}

bool CDebugInfo::CreateDebugProcess()
{
	STARTUPINFO StartupInfo = { 0 };
	PROCESS_INFORMATION ProcessInfo = { 0 };

	::GetStartupInfo(&StartupInfo);
	StartupInfo.cb = sizeof(STARTUPINFO);

	if (!::CreateProcess(m_hFileInfor.m_CPathFileName,
		NULL, NULL, NULL, TRUE, DEBUG_PROCESS | DEBUG_ONLY_THIS_PROCESS,
		NULL, NULL, &StartupInfo, &ProcessInfo))
	{
		DWORD dwErrorId = ::GetLastError();
		printf("CreateProcess Error \r\nGetLastError: %d", dwErrorId);
		return FALSE;
	}

	CloseHandle(ProcessInfo.hProcess);
	CloseHandle(ProcessInfo.hThread);

	//�������
	DebugProcessProc();

	return TRUE;
}

void CDebugInfo::DebugProcessProc()
{
	DEBUG_EVENT DebugEv;                   // debugging event information 
	DWORD dwContinueStatus = DBG_CONTINUE; // exception continuation 
	while (true)
	{

		WaitForDebugEvent(&DebugEv, INFINITE);
		GetThreadInfo(DebugEv);
		switch (DebugEv.dwDebugEventCode)
		{
		case EXCEPTION_DEBUG_EVENT:
			::GetThreadContext(m_hThread, &m_Context);
			EXCEPTION_RECORD DebugRecordInfor = DebugEv.u.Exception.ExceptionRecord;
			switch (DebugRecordInfor.ExceptionCode)
			{
			case EXCEPTION_ACCESS_VIOLATION:
				// First chance: Pass this on to the system. 
				// Last chance: Display an appropriate error.
				DisposeAccessException(DebugRecordInfor);
				if (m_bExNOProc == TRUE)
				{
					dwContinueStatus = DBG_EXCEPTION_NOT_HANDLED;
					m_bExNOProc = FALSE;
				}
				break;
			case EXCEPTION_BREAKPOINT:
				// First chance: Display the current 
				// instruction and register values.							
				m_bFirstLoadDll = FALSE;
				DisposeBreakException(DebugRecordInfor);
				break;
			case EXCEPTION_SINGLE_STEP:
				// First chance: Update the display of the 
				// current instruction and register values.
				DisposeStepException(DebugRecordInfor);
				break;
			}
		break;
		}












	}















}

void CDebugInfo::GetThreadInfo(DEBUG_EVENT DebugEv)
{
	HMODULE hModule = ::LoadLibrary("Kernel32.dll");

	pMyOpenThread pOpenThread = (pMyOpenThread)GetProcAddress(hModule, "OpenThread");

	m_hThread = pOpenThread(THREAD_ALL_ACCESS, NULL, DebugEv.dwThreadId);
	if (m_hThread == NULL)
	{
		//MessageBox(NULL,"Open Thread fails !","Error",MB_OK);
		return;
	}
}

void CDebugInfo::DisposeAccessException(EXCEPTION_RECORD RecordTmp)
{
	BOOL bRWMemFlag;        //�������д�����жϱ��
	if (RecordTmp.ExceptionInformation[0] == NULL)
	{
		m_RWMemFlag = READ;
		bRWMemFlag = READ;
		//bDelRWMemFlag = READ;
	}
	else
	{
		m_RWMemFlag = WRITE;
		bRWMemFlag = WRITE;
		//bDelRWMemFlag = WRITE;
	}

	POSITION Pos = m_MemBPList.GetHeadPosition();

	//�ó��¶ϵ�ַ�ķ�ҳ ϵͳĬ�ϵ��ڴ�ҳΪ1000
	unsigned int EpnPageAddr = RecordTmp.ExceptionInformation[1] / 0x1000 * 0x1000;
	MemPageBreakPoint mBPListNode;
	DWORD dwProtect = 0;
	char szEpnCode[32] = { 0 };

	//��ҳ
	for (int i = 0; i < m_MemBPList.GetCount(); i++)
	{
		mBPListNode = m_MemBPList.GetNext(Pos);

		if (mBPListNode.BeginPageAddress == EpnPageAddr)
		{
			MemBPointInfor BPStartAddr;
			POSITION PosPage = NULL;
			CList<MemBPointInfor, MemBPointInfor> *RWListData = NULL;


			PosPage = mBPListNode.RWMemBPList->GetHeadPosition();
			RWListData = mBPListNode.RWMemBPList;

			for (int j = 0; j < RWListData->GetCount(); j++)
			{
				BPStartAddr = RWListData->GetNext(PosPage);

				//�ж����жϵ��ַ
				if ((BPStartAddr.beginAddress <= (RecordTmp.ExceptionInformation[1])) &&
					((RecordTmp.ExceptionInformation[1]) <= (BPStartAddr.endAddress)) &&
					(BPStartAddr.bDelFlag == TRUE) &&     //ɾ���жϱ��
					//���¶�Ϊ���� �����Զϣ� ���¶�Ϊд����д��ָ�����Ч		����Ϊ0 д��Ϊ1	
					(!BPStartAddr.bRWFlag || (bRWMemFlag == WRITE)))//&& 						  
					//(bDelRWMemFlag == BPStartAddr.bRWFlag) )
				{
					m_RWMemFlag = BPStartAddr.bRWFlag;
					m_StartMemAddr = BPStartAddr.beginAddress;
					::VirtualProtectEx(m_hProcess, (void *)BPStartAddr.beginAddress, 1, mBPListNode.OldPageProtect, &dwProtect);
					m_CurCode.pCurCodeEntryAddress = (DWORD)RecordTmp.ExceptionAddress;
					::ReadProcessMemory(m_hProcess, RecordTmp.ExceptionAddress, szEpnCode, sizeof(szEpnCode), NULL);
					m_CurCode.szCurLineCode[0] = szEpnCode[0];
					m_CurCode.szCurLineCode[1] = szEpnCode[1];

					m_Context.EFlags |= 0x100;
					m_MemStepFlag = TRUE;

					if (READ == bRWMemFlag)
					{
						printf("��ǰEIP: 0x%08X, �����ڴ�ϵ��ַ: 0x%08X, ����: �����ڴ�\r\n", m_Context.Eip, RecordTmp.ExceptionInformation[1]);
					}
					else
					{
						printf("��ǰEIP: 0x%08X, �����ڴ�ϵ��ַ: 0x%08X, ����: д���ڴ�\r\n", m_Context.Eip, RecordTmp.ExceptionInformation[1]);
					}

					PrintfRegisterInfor();
					ShowAsmCode(szEpnCode, m_CurCode.pCurCodeEntryAddress);
					if (FALSE == bSelIptFlag)
					{
						InputDictation();
					}
					return;
				}
				else
				{
					m_RWMemFlag = BPStartAddr.bRWFlag;
					m_StartMemAddr = BPStartAddr.beginAddress;
					::VirtualProtectEx(m_hProcess, (void *)BPStartAddr.beginAddress, 1, mBPListNode.OldPageProtect, &dwProtect);
					m_CurCode.pCurCodeEntryAddress = (DWORD)RecordTmp.ExceptionAddress;
					::ReadProcessMemory(m_hProcess, RecordTmp.ExceptionAddress, szEpnCode, sizeof(szEpnCode), NULL);

					m_Context.EFlags |= 0x100;
					m_MemStepFlag = TRUE;
				}
			}
		}
	}

	if (FALSE == m_MemStepFlag)
	{
		m_bExNOProc = TRUE;
	}
}

}

bool CDebugInfo::InitializeFile(WCHAR* wcPathName)
{
	//���ļ����н��������ж��Ƿ���pe�ļ�
	BOOL ret = m_hFileInfor.AnalysingFile(wcPathName);
	if (!ret)
	{
		return FALSE;
	}
	return TRUE;
}
