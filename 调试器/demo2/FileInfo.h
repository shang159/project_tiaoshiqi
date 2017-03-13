#pragma once
#include <afxtempl.h>

#if !defined(AFX_MYFILEINFOR_H__D3FD997A_4100_470A_A3D5_B6C5CDB83499__INCLUDED_)
#define AFX_MYFILEINFOR_H__D3FD997A_4100_470A_A3D5_B6C5CDB83499__INCLUDED_
#endif
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//导入表结构
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

//导出表结构
struct ExportFunInfor
{
	char  szFunName[64];                    //函数名称
	int   nExportNumber;                    //导出编号
	DWORD dwFunAddress;                     //函数所在地址
};

struct ExportTableInfor                                 //导出函数表
{
	char szDllTableName[32];                             //表名<DLL>
	CList<ExportFunInfor, ExportFunInfor> *pEptTableFun;  //数据    
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
	//打开文件的句柄
	HANDLE m_hFile;
	//文件映射句柄
	HANDLE m_hFileMapHandle;
	//文件映射
	LPVOID m_hFileMapImageBase;

	//Dos头
	IMAGE_DOS_HEADER *m_pFileDosHeader;
	//NT头
	IMAGE_NT_HEADERS *m_pFileNtHeader;

	//File头
	IMAGE_FILE_HEADER *m_pFileHeader;
	//可选头
	IMAGE_OPTIONAL_HEADER32 *m_pOptionalHeader;

	//节表位置
	IMAGE_SECTION_HEADER *m_pFileSectionHeader;
	//IMAGE_DATA_DIRECTORY位置
	IMAGE_DATA_DIRECTORY *m_pDataDirectory;

	//导入表头
	IMAGE_IMPORT_DESCRIPTOR m_DllImportTableHeader;

	//导出表
	IMAGE_EXPORT_DIRECTORY m_DllExportTable;

public:
	ExportTableInfor m_EptInfor;
	CList<ImportTableInfor, ImportTableInfor> m_ImportTableList;
	CList<ExportTableInfor, ExportTableInfor> m_ExportTableList;
};

