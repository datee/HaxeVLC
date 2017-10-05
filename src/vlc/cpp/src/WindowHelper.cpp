#include <iostream>
#include <windows.h>
#undef RegisterClass

void WindowHelper::showWindow(bool show)
{
	HWND hwnd=GetForegroundWindow(); 
	ShowWindow(hwnd, SW_HIDE);		
	std::cout << "yo...?";
}
