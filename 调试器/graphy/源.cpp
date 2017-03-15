#include <stdio.h>  
#include <stdlib.h>  
#include <Windows.h>  
#include <conio.h>    
unsigned long num_written = 100;



int main()
{




	WCHAR *str = L"Hello World!";     //定义输出信息  
	int len = wcsnlen(str,100), i;
	WORD shadow = BACKGROUND_INTENSITY;     //阴影属性  
	WORD text = BACKGROUND_GREEN | BACKGROUND_INTENSITY;    //文本属性  
	HANDLE handle_out2 = GetStdHandle(STD_OUTPUT_HANDLE);    //获得标准输出设备句柄  
	LPDWORD lpNumberOfAttrsWritte=(LPDWORD)100;
	CONSOLE_SCREEN_BUFFER_INFO csbi;    //定义窗口缓冲区信息结构体  
	GetConsoleScreenBufferInfo(handle_out2, &csbi);  //获得窗口缓冲区信息  
	SMALL_RECT rc;      //定义一个文本框输出区域  
	COORD posText;      //定义文本框的起始坐标  
	//确定区域的边界  
	rc.Top = 0;     //上边界  
	rc.Bottom = rc.Top + 30;     //下边界  
	rc.Left = (csbi.dwSize.X - len) / 2 - 2;    //左边界，为了让输出的字符串居中  
	rc.Right = rc.Left + len + 4;   //右边界  
	//确定文本框起始坐标  
	posText.X = rc.Left;
	posText.Y = rc.Top;
	for (i = 0; i < 20; ++i)     //在输出文本框，其中与阴影框重合的部分会被覆盖掉  
	{
		FillConsoleOutputAttribute(handle_out2, text, len + 4, posText, &num_written);
		posText.Y++;
	}
	//设置文本输出处的坐标  
	posText.X = rc.Left + 2;
	posText.Y = rc.Top + 2;
	WriteConsoleOutputCharacter(handle_out2, str, len, posText, &num_written);   //输出字符串  
	SetConsoleTextAttribute(handle_out2, csbi.wAttributes);   // 恢复原来的属性  
	CloseHandle(handle_out2);
	return 0;
}
