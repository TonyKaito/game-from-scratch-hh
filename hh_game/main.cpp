#include <windows.h>

#define local_persist static
#define global_var static
#define internal_funct static

/*
* Globals
* automatically 0 by default
*/
global_var bool running; // temporarily

// https://learn.microsoft.com/en-us/windows/win32/api/wingdi/ns-wingdi-bitmapinfo
// https://learn.microsoft.com/en-us/windows/win32/api/wingdi/ns-wingdi-bitmapinfoheader
global_var BITMAPINFO bmInfo;
global_var void* bmMemory;

/*
* 
*/
internal_funct void Win32ResizeDIBSection(
	int width,
	int height)
{
	if (bmMemory)
	{
		// https://learn.microsoft.com/en-us/windows/win32/api/memoryapi/nf-memoryapi-virtualfree
		VirtualFree(bmMemory, 0, MEM_RELEASE);
	}

	bmInfo.bmiHeader.biSize = sizeof(bmInfo.bmiHeader);
	bmInfo.bmiHeader.biWidth = width;
	bmInfo.bmiHeader.biHeight = height;
	bmInfo.bmiHeader.biPlanes = 1;
	bmInfo.bmiHeader.biBitCount = 32;
	bmInfo.bmiHeader.biCompression = BI_RGB;
	
	// no need for DC, difference between StretchDIBits vs BitBlt
	int bytesPerPix = 4;
	int bmMemorySize = (width * height) * bytesPerPix;
	// https://learn.microsoft.com/en-us/windows/win32/api/memoryapi/nf-memoryapi-virtualalloc
	bmMemory = VirtualAlloc(0, bmMemorySize, MEM_COMMIT, PAGE_READWRITE);
}

internal_funct void Win32UpdateWindow(
	HDC devContext,
	int x,
	int y,
	int width,
	int height)
{
	// https://learn.microsoft.com/en-us/windows/win32/api/wingdi/nf-wingdi-stretchdibits
	StretchDIBits(
		devContext,
		x,
		y,
		width,
		height,
		x,
		y, 
		width,
		height,
		bmMemory,
		&bmInfo,
		DIB_RGB_COLORS,
		SRCCOPY);
}

/*
* https://learn.microsoft.com/en-us/windows/win32/api/winuser/nc-winuser-wndproc
*/
LRESULT CALLBACK Win32MainWindowCallback(
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
			RECT clientRect;
			// https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getclientrect
			GetClientRect(window, &clientRect);

			int width = clientRect.right - clientRect.left;
			int height = clientRect.bottom - clientRect.top;

			Win32ResizeDIBSection(width, height);
		} break;

		case WM_DESTROY:
		{
			running = false; // TODO(kt): handle as error
		} break;

		case WM_CLOSE:
		{
			running = false; // TODO(kt): handle as message
		} break;

		case WM_ACTIVATEAPP:
		{
			OutputDebugString("WM_ACTIVATEAPP\n");
		} break;

		case WM_PAINT:
		{
			// https://learn.microsoft.com/en-us/windows/win32/api/winuser/ns-winuser-paintstruct
			PAINTSTRUCT paint;
			// https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-beginpaint
			HDC devContext = BeginPaint(window, &paint);

			int x = paint.rcPaint.left;
			int y = paint.rcPaint.top;
			int width = paint.rcPaint.right - paint.rcPaint.left;
			int height = paint.rcPaint.bottom - paint.rcPaint.top;
			
			Win32UpdateWindow(devContext, x, y, width, height);

			// https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-endpaint
			EndPaint(window, &paint);
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
	winClass.lpfnWndProc = Win32MainWindowCallback;
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
			running = true;
			while (running)
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