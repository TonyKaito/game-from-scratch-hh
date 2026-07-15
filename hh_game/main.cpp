#include <windows.h>


/*
* https://learn.microsoft.com/en-us/windows/win32/api/winuser/nc-winuser-wndproc
*/
LRESULT CALLBACK MainWindowCallback(
	HWND window,
	UINT message, // https://learn.microsoft.com/en-us/windows/win32/winmsg/about-messages-and-message-queues#system-defined-messages
	WPARAM wParam,
	LPARAM lParam)
{
	// https://learn.microsoft.com/en-us/windows/win32/winprog/windows-data-types
	LRESULT result = 0;

	switch (message)
	{
		case WM_SIZE:
		{
			OutputDebugString("WM_SIZE\n");
		} break;

		case WM_DESTROY:
		{
			OutputDebugString("WM_DESTROY\n");
		} break;

		case WM_CLOSE:
		{
			OutputDebugString("WM_CLOSE\n");
		} break;

		case WM_ACTIVATEAPP:
		{
			OutputDebugString("WM_ACTIVATEAPP\n");
		} break;

		case WM_PAINT:
		{
			OutputDebugString("WM_PAINT\n");
		} break;

		default:
		{
			// OutputDebugString("default\n");
			result = DefWindowProc(window, message, wParam, lParam);
		} break;
	}

	return(result);
}


/*
* Docs Involved
* https://learn.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-winmain
* But use CALLBACK instead of __clrcall
* Also, remember to set up Project to be Window App instead of Console App - Properties > Linker > System
*/
int CALLBACK WinMain(
	HINSTANCE instance,
	HINSTANCE prevInstance,
	LPSTR cmdLine,
	int showCode)
{
	// https://learn.microsoft.com/en-us/windows/win32/api/winuser/ns-winuser-wndclassa
	WNDCLASS winClass = {};

	// https://learn.microsoft.com/en-us/windows/win32/winmsg/window-class-styles
	winClass.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
	winClass.lpfnWndProc = MainWindowCallback;
	winClass.hInstance = instance;
	winClass.lpszClassName = "TokiGameWindowClass";

	// https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-registerclassa
	if (RegisterClass(&winClass))
	{
		// https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-createwindowexa
		HWND winHandle = CreateWindowEx(
			0,
			winClass.lpszClassName,
			"Toki's Game",
			WS_OVERLAPPEDWINDOW | WS_VISIBLE,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			0,
			0,
			instance,
			0);

		if (winHandle)
		{
			for (;;)
			{
				// https://learn.microsoft.com/en-us/windows/win32/api/winuser/ns-winuser-msg
				MSG message;
				// https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getmessage
				BOOL msgResult = GetMessage(&message, 0, 0, 0);
				if (msgResult > 0)
				{
					// https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-translatemessage
					TranslateMessage(&message);
					// https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-dispatchmessage
					DispatchMessage(&message);
				}
				else {
					break;
				}
			}
		}
		else 
		{
			// TODO(kt): maybe log/error system
		}
	}
	else 
	{
		// TODO(kt): maybe log/error system
	}

	return(0);
}