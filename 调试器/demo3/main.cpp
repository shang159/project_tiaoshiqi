#include<windows.h>
#include<stdio.h>
#include <conio.h>
#include "Debug.h"

void ShowMenu()
{
	printf_s("*************菜单*****************\n");
	printf_s("\n");
	printf_s("\t1.打开文件\n");
	printf_s("\n");
	printf_s("\t2.附加进程\n");
	printf_s("\n");
	printf_s("\t3.退出\n");
	printf_s("\n");
	printf_s("**********************************\n");
	HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_CURSOR_INFO CursorInfo;
	GetConsoleCursorInfo(handle, &CursorInfo);
	CursorInfo.bVisible = false;
	SetConsoleCursorInfo(handle, &CursorInfo);
}

int main()
{
	int ch;
	ShowMenu();
	switch (ch = _getch())
	{
	case 49:
	{
		system("cls");
		CDebug test;
		test.DebugMain();
	}
		break;
	case 50:
	{
		system("cls");
		//遍历进程，得到pid，然后选择pid新建进程
	}
		break;
	case 51:
	{
		exit(0);
	}
		break;
	default:
		break;
	}
	return 0;
}