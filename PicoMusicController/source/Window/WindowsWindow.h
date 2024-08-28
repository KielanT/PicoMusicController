#pragma once

#include <windows.h>
#include <atomic>

class WindowsWindow
{
public:
	WindowsWindow(std::atomic<bool>& IsRunning);
	int Update();

	std::atomic<bool>& m_IsAppRunning;

private:
	HINSTANCE m_HInstance = nullptr;

};

LRESULT CALLBACK
MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);