#include <stdio.h>  
#include <stdlib.h>  
#include <Windows.h>  
#include <conio.h>    
unsigned long num_written = 100;



int main()
{




	WCHAR *str = L"Hello World!";     //���������Ϣ  
	int len = wcsnlen(str,100), i;
	WORD shadow = BACKGROUND_INTENSITY;     //��Ӱ����  
	WORD text = BACKGROUND_GREEN | BACKGROUND_INTENSITY;    //�ı�����  
	HANDLE handle_out2 = GetStdHandle(STD_OUTPUT_HANDLE);    //��ñ�׼����豸���  
	LPDWORD lpNumberOfAttrsWritte=(LPDWORD)100;
	CONSOLE_SCREEN_BUFFER_INFO csbi;    //���崰�ڻ�������Ϣ�ṹ��  
	GetConsoleScreenBufferInfo(handle_out2, &csbi);  //��ô��ڻ�������Ϣ  
	SMALL_RECT rc;      //����һ���ı����������  
	COORD posText;      //�����ı������ʼ����  
	//ȷ������ı߽�  
	rc.Top = 0;     //�ϱ߽�  
	rc.Bottom = rc.Top + 30;     //�±߽�  
	rc.Left = (csbi.dwSize.X - len) / 2 - 2;    //��߽磬Ϊ����������ַ�������  
	rc.Right = rc.Left + len + 4;   //�ұ߽�  
	//ȷ���ı�����ʼ����  
	posText.X = rc.Left;
	posText.Y = rc.Top;
	for (i = 0; i < 20; ++i)     //������ı�����������Ӱ���غϵĲ��ֻᱻ���ǵ�  
	{
		FillConsoleOutputAttribute(handle_out2, text, len + 4, posText, &num_written);
		posText.Y++;
	}
	//�����ı������������  
	posText.X = rc.Left + 2;
	posText.Y = rc.Top + 2;
	WriteConsoleOutputCharacter(handle_out2, str, len, posText, &num_written);   //����ַ���  
	SetConsoleTextAttribute(handle_out2, csbi.wAttributes);   // �ָ�ԭ��������  
	CloseHandle(handle_out2);
	return 0;
}
