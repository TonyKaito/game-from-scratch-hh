#include <windows.h>
#include <stdint.h>

#define local_persist static
#define global_var static
#define internal_funct static

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;


struct win32_offscreen_buffer {
	// https://learn.microsoft.com/en-us/windows/win32/api/wingdi/ns-wingdi-bitmapinfo
	// https://learn.microsoft.com/en-us/windows/win32/api/wingdi/ns-wingdi-bitmapinfoheader
	BITMAPINFO info;
	void* memory;
	int width;
	int height;
	int bytesPerPix;
};

struct win32_window_dimension {
	int width;
	int height;
};

/*
* Globals
* automatically 0 by default
*/
global_var bool running; // temporarily
global_var win32_offscreen_buffer globalBackBuffer;

/*
* 
*/
win32_window_dimension Win32GetWindowDimension(HWND window)
{
	win32_window_dimension result;

	RECT clientRect;
	// https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getclientrect
	GetClientRect(window, &clientRect);
	result.width = clientRect.right - clientRect.left;
	result.height = clientRect.bottom - clientRect.top;
	
	return(result);
}

/*
* 
*/
internal_funct void RenderBGGradient(win32_offscreen_buffer buffer, int xOff, int yOff)
{
	int width = buffer.width;
	int height = buffer.height;

	int pitch = width * buffer.bytesPerPix;
	uint8* row = (uint8*)buffer.memory;
	for (int y = 0; y < buffer.height; y++)
	{
		uint32* pixel = (uint32*)row;
		for (int x = 0; x < buffer.width; x++)
		{
			uint8 blue = (x + xOff);
			uint8 green = (y + yOff);

			*pixel++ = (green << 8) | blue;
		}
		row += pitch;
	}
}

/*
* 
*/
internal_funct void Win32ResizeDIBSection(
	win32_offscreen_buffer* buffer,
	int width,
	int height)
{
	if (buffer->memory)
	{
		// https://learn.microsoft.com/en-us/windows/win32/api/memoryapi/nf-memoryapi-virtualfree
		VirtualFree(buffer->memory, 0, MEM_RELEASE);
	}

	buffer->width = width;
	buffer->height = height;
	buffer->bytesPerPix = 4;

	buffer->info.bmiHeader.biSize = sizeof(buffer->info.bmiHeader);
	buffer->info.bmiHeader.biWidth = buffer->width;
	buffer->info.bmiHeader.biHeight = -buffer->height;
	buffer->info.bmiHeader.biPlanes = 1;
	buffer->info.bmiHeader.biBitCount = 32;
	buffer->info.bmiHeader.biCompression = BI_RGB;
	
	// no need for DC, difference between StretchDIBits vs BitBlt
	int bmMemorySize = (buffer->width * buffer->height) * buffer->bytesPerPix;
	// https://learn.microsoft.com/en-us/windows/win32/api/memoryapi/nf-memoryapi-virtualalloc
	buffer->memory = VirtualAlloc(0, bmMemorySize, MEM_COMMIT, PAGE_READWRITE);

	// TODO(kt): turn it to black
}

internal_funct void Win32DisplayBufferInWindow(
	win32_offscreen_buffer buffer,
	HDC devContext,
	int winWidth,
	int winHeight,
	int x,
	int y,
	int width,
	int height)
{
	// https://learn.microsoft.com/en-us/windows/win32/api/wingdi/nf-wingdi-stretchdibits
	/*
	* // "dirty rectangle", will return to later
	* StretchDIBits(
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
	*/
	StretchDIBits(
		devContext,
		0,
		0,
		winWidth,
		winHeight,
		0,
		0,
		buffer.width,
		buffer.height,
		buffer.memory,
		&buffer.info,
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
			win32_window_dimension dimension = Win32GetWindowDimension(window);
			Win32ResizeDIBSection(&globalBackBuffer, dimension.width, dimension.height);
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

			win32_window_dimension dimension = Win32GetWindowDimension(window);
			
			Win32DisplayBufferInWindow(globalBackBuffer, devContext, dimension.width, dimension.height, x, y, width, height);

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
	winClass.style = CS_HREDRAW | CS_VREDRAW;
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

			int xOff = 0;
			int yOff = 0;
			while (running)
			{
				// https://learn.microsoft.com/en-us/windows/win32/api/winuser/ns-winuser-msg
				MSG message;
				// https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-peekmessagea
				while(PeekMessage(&message, 0, 0, 0, PM_REMOVE))
				{
					if (message.message == WM_QUIT)
					{
						running = false;
					}

					// https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-translatemessage
					TranslateMessage(&message);
					// https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-dispatchmessage
					DispatchMessage(&message);
				}

				// TODO(kt): refactor into a function
				RenderBGGradient(globalBackBuffer, xOff, yOff);
				// https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getdc
				HDC devContext = GetDC(winHandle);

				win32_window_dimension dimension = Win32GetWindowDimension(winHandle);
				Win32DisplayBufferInWindow(globalBackBuffer, devContext, dimension.width, dimension.height, 0, 0, dimension.width, dimension.height);

				// https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-releasedc
				ReleaseDC(winHandle, devContext);

				xOff++;
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