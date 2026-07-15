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
	return(0);
}