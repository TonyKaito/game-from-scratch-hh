#include <windows.h>

/*
* Docs Involved
* https://learn.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-winmain
* But use CALLBACK instead of __clrcall
* Also, remember to set up Project to be Window App instead of Console App - Properties > Linker > System
*/

int CALLBACK WinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nCmdShow)
{
	// https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-messagebox
	MessageBoxA(0, "This is Toki's Window.", "Toki's Window", MB_OK | MB_ICONINFORMATION);

	return(0);
}