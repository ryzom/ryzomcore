// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "stdafx.h"

#include "../ide2/DebuggerMessages.h"
#include "libapi.h"

static HWND g_hWnd;

using namespace std;

// maximum mumber of lines the output console should have
static const WORD MAX_CONSOLE_LINES = 500;

BOOL isWindowsNT(void)
{
    static BOOL once = FALSE;
    static BOOL isNT = FALSE;
    
    if (!once)
    {
        OSVERSIONINFO osver;
        osver.dwOSVersionInfoSize = sizeof(osver);
        if (GetVersionEx(&osver))
            if (osver.dwPlatformId == VER_PLATFORM_WIN32_NT)
                isNT = TRUE;
        once = TRUE;
    }
    return isNT;
}

static BOOL CALLBACK EnumttyWindow(HWND wnd, LPARAM retwnd)
{
    char tmp[20], *tty;
    if (isWindowsNT())
        tty = "ConsoleWindowClass";
    else
        tty = "tty";
    if (GetClassName(wnd, tmp, sizeof(tmp)) && !strcmp(tmp, tty)) 
    {
        DWORD wndproc, thisproc = GetCurrentProcessId();
        GetWindowThreadProcessId(wnd, &wndproc);
        if (wndproc == thisproc) {
            *((HWND*)retwnd) = wnd;
            return FALSE;
        }
    }
    return TRUE;
}


static BOOL CtrlHandler(DWORD fdwCtrlType) 
{ 
    switch (fdwCtrlType) 
    { 
        // Handle the CTRL+C signal. 
 
        case CTRL_C_EVENT: 
        case CTRL_BREAK_EVENT: 
            return TRUE; 
 
        // CTRL+CLOSE: confirm that the user wants to exit. 
 
        case CTRL_CLOSE_EVENT: 

        case CTRL_LOGOFF_EVENT: 
 
        case CTRL_SHUTDOWN_EVENT: 
 
        default: 
 
            return FALSE; 
    } 
} 
 
void AttachConsole()
{
	int hConHandle;
	long lStdHandle;
	CONSOLE_SCREEN_BUFFER_INFO coninfo;
	FILE *fp;
	HWND hWndConsole;
	HMENU hSysMenu;

// allocate a console for this app
	FreeConsole();
	AllocConsole();

//disable system menu -> close box 
	EnumWindows(EnumttyWindow, (long)(&hWndConsole));
	hSysMenu = GetSystemMenu(hWndConsole, FALSE);
	ModifyMenu(hSysMenu, SC_CLOSE, MF_BYCOMMAND | MF_GRAYED, 0, 0);
	ShowWindow(hWndConsole, SW_SHOW);

// set the screen buffer to be big enough to let us scroll text
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &coninfo);
	coninfo.dwSize.Y = MAX_CONSOLE_LINES;
	SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE), coninfo.dwSize);

// redirect unbuffered STDOUT to the console
	lStdHandle = (long)GetStdHandle(STD_OUTPUT_HANDLE);
	hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
	fp = _fdopen( hConHandle, "w" );
	*stdout = *fp;
	setvbuf( stdout, NULL, _IONBF, 0 );

// redirect unbuffered STDIN to the console
	lStdHandle = (long)GetStdHandle(STD_INPUT_HANDLE);
	hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
	::SetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), ENABLE_LINE_INPUT|ENABLE_ECHO_INPUT|ENABLE_MOUSE_INPUT);
	fp = _fdopen( hConHandle, "r" );
	*stdin = *fp;
	setvbuf( stdin, NULL, _IONBF, 0 );

// redirect unbuffered STDERR to the console
	lStdHandle = (long)GetStdHandle(STD_ERROR_HANDLE);
	hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
	fp = _fdopen( hConHandle, "w" );
	*stderr = *fp;
	setvbuf( stderr, NULL, _IONBF, 0 );

// make cout, wcout, cin, wcin, wcerr, cerr, wclog and clog 
// point to console as well
	ios::sync_with_stdio();

	SetConsoleCtrlHandler( 
		(PHANDLER_ROUTINE) CtrlHandler,  // handler function 
		TRUE);                           // add to list 
}

void Trace(LPCSTR szMsg)
{
	if ( g_hWnd )
		::SendMessage(g_hWnd, DMSG_WRITE_DEBUG, (WPARAM)szMsg, 0);
	else
		fprintf(stderr, "%s", szMsg);
}

BOOL InitLibAPI(HWND hWnd)
{
	g_hWnd = hWnd;

	return TRUE;
}
