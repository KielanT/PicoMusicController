#include "WindowsWindow.h"
#include <iostream>

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

	HWND handle = CreateWindow(L"MainWnd", L"Pico Music Controller",
		WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, Width, Height, 0, 0, m_HInstance, this);


	if (!handle)
	{
		std::cout << "Create Window Failed" << std::endl; 
	}

	ShowWindow(handle, SW_SHOW);
	UpdateWindow(handle);
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
		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hwnd, &ps);

			FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));

			EndPaint(hwnd, &ps);
			return 0;
		}
		case WM_DESTROY:
			PostQuitMessage(0);

			if (window)
				window->m_IsAppRunning = false;

			return 0;
		}
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}