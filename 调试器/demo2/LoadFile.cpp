// demo2.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include <windows.h>
#include "DebugInfo.h"

//////////////////////////////////////////////////////////////////////////
/*
��ʼ���Ժ���
����������debuginfo���һ�����󣬵��øö���������������г�ʼ���ʹ������Խ���
*/
//////////////////////////////////////////////////////////////////////////
int StartDebug(WCHAR* szszFileName)
{
	CDebugInfo DebugInfo;
	BOOL ret = DebugInfo.InitializeFile(szszFileName);
	if (!ret)
	{
		printf("�ļ���ʼ������!\r\n");
		return 0;
	}
	ret = DebugInfo.CreateDebugProcess();
	if (!ret)
	{
		printf("���̴�������!\r\n");
		return 0;
	}
	return 1;
}
//////////////////////////////////////////////////////////////////////////
/*
�������
����api���ļ���Ȼ��������

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
