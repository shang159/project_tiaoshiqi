#include<windows.h>
#include<stdio.h>
#include <conio.h>
#include "Debug.h"

void ShowMenu()
{
	printf_s("*************�˵�*****************\n");
	printf_s("\n");
	printf_s("\t1.���ļ�\n");
	printf_s("\n");
	printf_s("\t2.���ӽ���\n");
	printf_s("\n");
	printf_s("\t3.�˳�\n");
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
		//�������̣��õ�pid��Ȼ��ѡ��pid�½�����
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