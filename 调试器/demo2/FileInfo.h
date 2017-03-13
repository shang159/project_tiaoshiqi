#pragma once
#include <afxtempl.h>

#if !defined(AFX_MYFILEINFOR_H__D3FD997A_4100_470A_A3D5_B6C5CDB83499__INCLUDED_)
#define AFX_MYFILEINFOR_H__D3FD997A_4100_470A_A3D5_B6C5CDB83499__INCLUDED_
#endif
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//�����ṹ
struct ImportFunInfor
{
	char szFunName[32];
	unsigned int nFunAddress;
};

struct ImportTableInfor
{
	HMODULE hModule;
	DWORD ModuleAddress;
	char DllModuleName[32];
	CList<ImportFunInfor, ImportFunInfor> *pFunInfoList;
};

//������ṹ
struct ExportFunInfor
{
	char  szFunName[64];                    //��������
	int   nExportNumber;                    //�������
	DWORD dwFunAddress;                     //�������ڵ�ַ
};

struct ExportTableInfor                                 //����������
{
	char szDllTableName[32];                             //����<DLL>
	CList<ExportFunInfor, ExportFunInfor> *pEptTableFun;  //����    
};

class CFileInfo
{
public:
	CFileInfo();
	~CFileInfo();
	BOOL AnalysingFile(WCHAR* szFilename);
	BOOL IsPeFile(LPVOID StartAddress);
public:
	WCHAR* m_CPathFileName;
	//���ļ��ľ��
	HANDLE m_hFile;
	//�ļ�ӳ����
	HANDLE m_hFileMapHandle;
	//�ļ�ӳ��
	LPVOID m_hFileMapImageBase;

	//Dosͷ
	IMAGE_DOS_HEADER *m_pFileDosHeader;
	//NTͷ
	IMAGE_NT_HEADERS *m_pFileNtHeader;

	//Fileͷ
	IMAGE_FILE_HEADER *m_pFileHeader;
	//��ѡͷ
	IMAGE_OPTIONAL_HEADER32 *m_pOptionalHeader;

	//�ڱ�λ��
	IMAGE_SECTION_HEADER *m_pFileSectionHeader;
	//IMAGE_DATA_DIRECTORYλ��
	IMAGE_DATA_DIRECTORY *m_pDataDirectory;

	//�����ͷ
	IMAGE_IMPORT_DESCRIPTOR m_DllImportTableHeader;

	//������
	IMAGE_EXPORT_DIRECTORY m_DllExportTable;

public:
	ExportTableInfor m_EptInfor;
	CList<ImportTableInfor, ImportTableInfor> m_ImportTableList;
	CList<ExportTableInfor, ExportTableInfor> m_ExportTableList;
};

