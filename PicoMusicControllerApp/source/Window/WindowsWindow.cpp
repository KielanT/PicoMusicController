#include "WindowsWindow.h"
#include <iostream>

#define WM_TRAY_ICON (WM_USER + 1)

WindowsWindow::WindowsWindow(std::atomic<bool>& IsRunning)
	: m_IsAppRunning(IsRunning)
{
	WNDCLASS wc;
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = MainWndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = m_HInstance;
	wc.hIcon = LoadIcon(0, IDI_APPLICATION);
	wc.hCursor = LoadCursor(0, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
	wc.lpszMenuName = 0;
	wc.lpszClassName = L"MainWnd";

	if (!RegisterClass(&wc))
	{
		std::cout << "RegisterClass Failed" << std::endl; 
	}

	RECT R = { 0, 0, 800, 600 };
	AdjustWindowRect(&R, WS_OVERLAPPEDWINDOW, false);

	int Width = R.right - R.left;
	int Height = R.bottom - R.top;

	Handle = CreateWindow(L"MainWnd", L"Pico Music Controller",
		WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, Width, Height, 0, 0, m_HInstance, this);


	if (!Handle)
	{
		std::cout << "Create Window Failed" << std::endl; 
	}

	ShowWindow(Handle, SW_SHOW);
	UpdateWindow(Handle);
}

int WindowsWindow::Update()
{
	MSG msg = { 0 };


	while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE) > 0)
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return (int)msg.wParam;
}

void MinmizeToTrayIcon(HWND hwnd, UINT id)
{
	ShowWindow(hwnd, SW_HIDE);

	NOTIFYICONDATA nid = {};
	nid.cbSize = sizeof(NOTIFYICONDATA);
	nid.hWnd = hwnd;
	nid.uID = id;
	nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	nid.uCallbackMessage = WM_TRAY_ICON;
	nid.hIcon = LoadIcon(NULL, IDI_APPLICATION); 
	wcscpy_s(nid.szTip, L"Pico Music Controller");

	Shell_NotifyIcon(NIM_ADD, &nid);
}

LRESULT MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	
	if (msg == WM_CREATE)
	{
		LPCREATESTRUCT pCreate = (LPCREATESTRUCT)lParam;
		WindowsWindow* window = (WindowsWindow*)pCreate->lpCreateParams;
		SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)window);

	}
	else
	{
		WindowsWindow* window = (WindowsWindow*)GetWindowLongPtr(hwnd, GWLP_USERDATA);

		switch (msg)
		{
		case WM_TRAY_ICON:
		{
			if (LOWORD(lParam) == WM_LBUTTONDBLCLK)  // Double-click on the tray icon to restore
			{
				ShowWindow(hwnd, SW_SHOW);
				ShowWindow(hwnd, SW_RESTORE);
			}
			break;
		}
		case WM_SIZE:
			if (wParam == SIZE_MINIMIZED)
			{
				MinmizeToTrayIcon(hwnd, window->uniqueID);
			}
			break;
		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hwnd, &ps);

			FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));

			EndPaint(hwnd, &ps);
			break;
		}
		case WM_DESTROY:
			PostQuitMessage(0);

			if (window)
				window->m_IsAppRunning.store(false);

			return 0;
		}
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}