#pragma once
#include <windows.h>
class CDebugInfo
{
public:
	CDebugInfo();
	~CDebugInfo();
public:
	bool InitializeFile(WCHAR* wcPathName);
	bool CreateDebugProcess();
};

