#include "stdafx.h"
#include "FileInfo.h"
#include <minwindef.h>


CFileInfo::CFileInfo()
{
}


CFileInfo::~CFileInfo()
{
}

BOOL CFileInfo::AnalysingFile(WCHAR* szFilename)
{
	int nLength = wcsnlen(szFilename,MAX_PATH);
	int rellength = nLength + sizeof(WCHAR);
	m_CPathFileName = new WCHAR[nLength + sizeof(WCHAR)];
	wcsncpy(m_CPathFileName, szFilename, rellength);

	m_hFile = CreateFile(szFilename, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (m_hFile == INVALID_HANDLE_VALUE)
	{
		printf("CreateFile Faile!\r\n");
		return FALSE;
	}

	m_hFileMapHandle = ::CreateFileMapping(m_hFile, NULL, PAGE_READWRITE, NULL, NULL, NULL);
	if (m_hFileMapHandle == NULL)
	{
		printf("�����ļ�ӳ��ʧ��!\r\n");
		CloseHandle(m_hFile);
		return FALSE;
	}

	m_hFileMapImageBase = ::MapViewOfFile(m_hFileMapHandle, FILE_MAP_ALL_ACCESS, 0, 0, 0); //����ļ��Ļ���ַ 
	if (m_hFileMapImageBase == NULL)
	{
		printf("�ļ�ӳ��ʧ��!\r\n");
		CloseHandle(m_hFileMapHandle);
		CloseHandle(m_hFile);
		return FALSE;
	}

	//�ж��ǲ�����Ч��PE�ļ�
	if (!IsPeFile(m_hFileMapImageBase))
	{
		printf("������Ч��PE�ļ�!\r\n");
		return FALSE;
	}


	if (m_hFileMapImageBase)
	{
		//ж���ڴ�ӳ��
		UnmapViewOfFile(m_hFileMapImageBase);
	}
	if (m_hFile)
	{
		CloseHandle(m_hFile);
	}

	return TRUE;
}
BOOL CFileInfo::IsPeFile(LPVOID StartAddress)
{
	//IMAGE_DOS_SIGNATURE 0x5a4d = MZ
	//IMAGE_NT_SIGNATURE  0x5045 = PE
	m_pFileDosHeader = (IMAGE_DOS_HEADER *)StartAddress;
	if (m_pFileDosHeader->e_magic != IMAGE_DOS_SIGNATURE)
	{
		return FALSE;
	}

	m_pFileNtHeader = (IMAGE_NT_HEADERS *)((int)StartAddress + m_pFileDosHeader->e_lfanew);

	if (m_pFileNtHeader->Signature != IMAGE_NT_SIGNATURE)
	{
		return FALSE;
	}

	m_pFileHeader = &m_pFileNtHeader->FileHeader;

	m_pFileSectionHeader = (IMAGE_SECTION_HEADER *)((int)StartAddress + m_pFileDosHeader->e_lfanew
		+ sizeof(DWORD)
		+ m_pFileHeader->SizeOfOptionalHeader
		+ sizeof(IMAGE_FILE_HEADER));



	//��ѡͷ
	m_pOptionalHeader = &m_pFileNtHeader->OptionalHeader;

	//����Ŀ¼
	m_pDataDirectory = m_pOptionalHeader->DataDirectory;

	return TRUE;
}