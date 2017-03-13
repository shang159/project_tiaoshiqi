// demo2.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <windows.h>
#include "DebugInfo.h"

//////////////////////////////////////////////////////////////////////////
/*
开始调试函数
函数内生成debuginfo类的一个对象，调用该对象的两个函数进行初始化和创建调试进程
*/
//////////////////////////////////////////////////////////////////////////
int StartDebug(WCHAR* szszFileName)
{
	CDebugInfo DebugInfo;
	BOOL ret = DebugInfo.InitializeFile(szszFileName);
	if (!ret)
	{
		printf("文件初始化错误!\r\n");
		return 0;
	}
	ret = DebugInfo.CreateDebugProcess();
	if (!ret)
	{
		printf("进程创建错误!\r\n");
		return 0;
	}
	return 1;
}
//////////////////////////////////////////////////////////////////////////
/*
程序入口
调用api打开文件，然后进入调试

*/
//////////////////////////////////////////////////////////////////////////
int main()
{
	WCHAR        szFileName[MAX_PATH] = L"";
	OPENFILENAME ofn;

	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.lpstrFile = szFileName;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrFilter = L"Exe Files(*.exe)\0*.exe\0All Files(*.*)\0*.*\0\0";
	ofn.nFilterIndex = 1;
	if (GetOpenFileName(&ofn))
	{
		StartDebug(szFileName);
	}
	return 0;
}
