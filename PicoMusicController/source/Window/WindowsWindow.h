#pragma once

#include <windows.h>
#include <atomic>

class WindowsWindow
{
public:
	WindowsWindow(std::atomic<bool>& IsRunning);
	int Update();

	std::atomic<bool>& m_IsAppRunning;

	UINT uniqueID{ 100 }; // Random
private:
	HINSTANCE m_HInstance = nullptr;
};

void MinmizeToTrayIcon(HWND hwnd, UINT id);

LRESULT CALLBACK
MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);