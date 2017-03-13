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

	//进入调试
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
	BOOL bRWMemFlag;        //输出读或写类型判断标记
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

	//得出下断地址的分页 系统默认的内存页为1000
	unsigned int EpnPageAddr = RecordTmp.ExceptionInformation[1] / 0x1000 * 0x1000;
	MemPageBreakPoint mBPListNode;
	DWORD dwProtect = 0;
	char szEpnCode[32] = { 0 };

	//分页
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

				//判断命中断点地址
				if ((BPStartAddr.beginAddress <= (RecordTmp.ExceptionInformation[1])) &&
					((RecordTmp.ExceptionInformation[1]) <= (BPStartAddr.endAddress)) &&
					(BPStartAddr.bDelFlag == TRUE) &&     //删除判断标记
					//当下断为访问 都可以断， 当下断为写，则写的指令才生效		访问为0 写入为1	
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
						printf("当前EIP: 0x%08X, 触发内存断点地址: 0x%08X, 类型: 访问内存\r\n", m_Context.Eip, RecordTmp.ExceptionInformation[1]);
					}
					else
					{
						printf("当前EIP: 0x%08X, 触发内存断点地址: 0x%08X, 类型: 写入内存\r\n", m_Context.Eip, RecordTmp.ExceptionInformation[1]);
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
	//对文件进行解析，并判断是否是pe文件
	BOOL ret = m_hFileInfor.AnalysingFile(wcPathName);
	if (!ret)
	{
		return FALSE;
	}
	return TRUE;
}
