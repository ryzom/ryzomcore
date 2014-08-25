// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
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

#include "stdmisc.h"
#include "nel/misc/win_displayer.h"

#ifdef NL_OS_WINDOWS
#ifndef NL_COMP_MINGW
#	define NOMINMAX
#endif
#include <windows.h>
#include <windowsx.h>
#include <winuser.h>
#include <cstring>
#include <cstdlib>
#include <richedit.h>
#include <commctrl.h>
#include <ctime>

#include "nel/misc/path.h"
#include "nel/misc/command.h"
#include "nel/misc/thread.h"
#include "nel/misc/ucstring.h"

using namespace std;

#ifdef DEBUG_NEW
	#define new DEBUG_NEW
#endif

namespace NLMISC {

static CHARFORMAT2 CharFormat;


CWinDisplayer::~CWinDisplayer ()
{
}

LRESULT CALLBACK WndProc (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	MSGFILTER *pmf;

	switch (message)
	{
	case WM_ACTIVATE:
		{
			if (LOWORD(wParam) != WA_INACTIVE)
			{
				CWinDisplayer *cwd=(CWinDisplayer *)GetWindowLongPtrW (hWnd, GWLP_USERDATA);
				if (cwd != NULL)
					SetFocus(cwd->_HInputEdit);
				return 0;
			}
		}
		break;
	case WM_SIZE:
		{
			CWinDisplayer *cwd=(CWinDisplayer *)GetWindowLongPtrW (hWnd, GWLP_USERDATA);
			if (cwd != NULL)
			{
				int w = lParam & 0xFFFF;
				int h = (lParam >> 16) & 0xFFFF;

//				SetWindowPos (cwd->_HEdit, NULL, 0, cwd->_ToolBarHeight, w, h-cwd->_ToolBarHeight-cwd->_InputEditHeight, SWP_NOZORDER | SWP_NOACTIVATE );
				SetWindowPos (cwd->_HInputEdit, NULL, 0, h-cwd->_InputEditHeight, w, cwd->_InputEditHeight, SWP_NOZORDER | SWP_NOACTIVATE );
				cwd->resizeLabels ();
			}
		}
		break;
	case WM_DESTROY:	PostQuitMessage (0); break;
	case WM_CLOSE:
		{
			CWinDisplayer *cwd=(CWinDisplayer *)GetWindowLongPtrW (hWnd, GWLP_USERDATA);
			if (cwd != NULL)
				cwd->_Continue = false;
		}
		break;
	case WM_COMMAND:
		{
			if (HIWORD(wParam) == BN_CLICKED)
			{
				CWinDisplayer *cwd=(CWinDisplayer *)GetWindowLongPtrW (hWnd, GWLP_USERDATA);
				// find the button and execute the command
				CSynchronized<std::vector<CWindowDisplayer::CLabelEntry> >::CAccessor access (&cwd->_Labels);
				for (uint i = 0; i < access.value().size(); i++)
				{
					if (access.value()[i].Hwnd == (HWND)lParam)
					{
						if(access.value()[i].Value == "@Clear|CLEAR")
						{
							// special commands because the clear must be called by the display thread and not main thread
							cwd->clear ();
						}
						else
						{
							// the button was found, add the command in the command stack
							CSynchronized<std::vector<std::string> >::CAccessor accessCommands (&cwd->_CommandsToExecute);
							string str;
							nlassert (!access.value()[i].Value.empty());
							nlassert (access.value()[i].Value[0] == '@');

							string::size_type pos = access.value()[i].Value.find ("|");
							if (pos != string::npos)
							{
								str = access.value()[i].Value.substr(pos+1);
							}
							else
							{
								str = access.value()[i].Value.substr(1);
							}
							if (!str.empty())
								accessCommands.value().push_back(str);
						}
						break;
					}
				}
			}
		}
		break;
	case WM_NOTIFY:
		switch (((NMHDR*)lParam)->code)
		{
		case EN_MSGFILTER:
			pmf = (MSGFILTER *)lParam;
			if (pmf->msg == WM_CHAR)
			{
				if (pmf->wParam == VK_RETURN)
				{
					WCHAR	wText[20000];
					string	TextSend;
					CWinDisplayer *cwd=(CWinDisplayer *)GetWindowLongPtr (hWnd, GWLP_USERDATA);
					// get the text as unicode string
					GetWindowTextW(cwd->_HInputEdit, wText, 20000);
					ucstring ucs((ucchar*)wText);
					// and convert it to UTF-8 encoding.
					TextSend = ucs.toUtf8();
					SendMessage (cwd->_HInputEdit, WM_SETTEXT, (WPARAM)0, (LPARAM)"");
					const char *pos2 = TextSend.c_str();
					string str;
					while (*pos2 != '\0')
					{
						str = "";

						// get the string
						while (*pos2 != '\0' && *pos2 != '\n')
						{
							if (*pos2 != '\r')
							{
								str += *pos2;
							}
							*pos2++;
						}

						// eat the \n
						if (*pos2 == '\n')
							*pos2++;

						if (!str.empty())
						{
							{
								CSynchronized<std::vector<std::string> >::CAccessor access (&cwd->_CommandsToExecute);
								access.value().push_back(str);
							}
							cwd->_History.push_back(str);
							cwd->_PosInHistory = (uint)cwd->_History.size();
						}
					}
				}
				else if (pmf->wParam == VK_TAB)
				{
					WCHAR	wText[20000];


					CWinDisplayer *cwd=(CWinDisplayer *)GetWindowLongPtrW (hWnd, GWLP_USERDATA);

					// get the text as unicode string
					GetWindowTextW(cwd->_HInputEdit, wText, 20000);
					ucstring ucs((ucchar*)wText);
					// and convert it to UTF-8 encoding
					string str = ucs.toUtf8();
					nlassert (cwd->Log != NULL);
					ICommand::expand (str, *cwd->Log);
					SendMessage (cwd->_HInputEdit, WM_SETTEXT, (WPARAM)0, (LPARAM)str.c_str());

					SendMessage (cwd->_HInputEdit, EM_SETSEL, str.size(), str.size());

					return 1;
				}
			}
			else if (pmf->msg == WM_KEYUP)
			{
				if (pmf->wParam == VK_UP)
				{
					CWinDisplayer *cwd=(CWinDisplayer *)GetWindowLongPtrW (hWnd, GWLP_USERDATA);

					if (cwd->_PosInHistory > 0)
						cwd->_PosInHistory--;

					if (!cwd->_History.empty())
					{
						ucstring ucs;
						// convert the text from UTF-8 to unicode
						ucs.fromUtf8(cwd->_History[cwd->_PosInHistory]);
						// set the text as unicode string
						SetWindowTextW(cwd->_HInputEdit, (LPCWSTR)ucs.c_str());
						SendMessage (cwd->_HInputEdit, EM_SETSEL, (WPARAM)ucs.size(), (LPARAM)ucs.size());
					}
				}
				else if (pmf->wParam == VK_DOWN)
				{
					CWinDisplayer *cwd=(CWinDisplayer *)GetWindowLongPtrW (hWnd, GWLP_USERDATA);

					if (cwd->_PosInHistory < cwd->_History.size()-1)
						cwd->_PosInHistory++;

					if (!cwd->_History.empty() && cwd->_PosInHistory < cwd->_History.size())
					{
						ucstring ucs;
						// convert the text from UTF-8 to unicode
						ucs.fromUtf8(cwd->_History[cwd->_PosInHistory]);
						// set the text as unicode string
						SetWindowTextW(cwd->_HInputEdit, (LPCWSTR)ucs.c_str());
						SendMessage (cwd->_HInputEdit, EM_SETSEL, (WPARAM)ucs.size(), (LPARAM)ucs.size());
					}
				}
			}
		}
	}

	return DefWindowProcW (hWnd, message, wParam, lParam);
}

void CWinDisplayer::updateLabels ()
{
	bool needResize = false;
	{
		CSynchronized<std::vector<CLabelEntry> >::CAccessor access (&_Labels);
		for (uint i = 0; i < access.value().size(); i++)
		{
			if (access.value()[i].NeedUpdate && !access.value()[i].Value.empty())
			{
				if (access.value()[i].Hwnd == NULL)
				{
					// create a button for command and label for variables
					if (access.value()[i].Value[0] == '@')
					{
						access.value()[i].Hwnd = CreateWindowW (L"BUTTON", L"", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, 0, 0, 0, 0, _HWnd, (HMENU) NULL, (HINSTANCE) GetWindowLongPtr(_HWnd, GWLP_HINSTANCE), NULL);
					}
					else
					{
						access.value()[i].Hwnd = CreateWindowW (L"STATIC", L"", WS_CHILD | WS_VISIBLE | SS_SIMPLE, 0, 0, 0, 0, _HWnd, (HMENU) NULL, (HINSTANCE) GetWindowLongPtr(_HWnd, GWLP_HINSTANCE), NULL);
					}
					SendMessage ((HWND)access.value()[i].Hwnd, WM_SETFONT, (WPARAM)_HFont, TRUE);
					needResize = true;
				}

				string n;

				// do this tricks to be sure that windows will clear what is after the number
				if (access.value()[i].Value[0] != '@')
					n = access.value()[i].Value + "                                                 ";
				else
				{
					string::size_type pos = access.value()[i].Value.find ('|');
					if (pos != string::npos)
					{
						n = access.value()[i].Value.substr (1, pos - 1);
					}
					else
					{
						n = access.value()[i].Value.substr (1);
					}
				}

				SendMessage ((HWND)access.value()[i].Hwnd, WM_SETTEXT, 0, (LPARAM) n.c_str());
				access.value()[i].NeedUpdate = false;
			}
		}
	}
	if (needResize)
		resizeLabels();
}

void CWinDisplayer::resizeLabels ()
{
	{
		CSynchronized<std::vector<CLabelEntry> >::CAccessor access (&_Labels);

		RECT Rect;
		GetClientRect (_HWnd, &Rect);

		uint i = 0, nb;
		uint y = 0, nby = 1;

		for (i = 0; i < access.value().size (); i++)
			if (access.value()[i].Value.empty())
				nby++;

		i = 0;

		for(;;)
		{
			nb = 0;
			while (i+nb != access.value().size () && !access.value()[i+nb].Value.empty()) nb++;

			sint delta;
			if (nb == 0)
				delta = 0;
			else
				delta = Rect.right / nb;

			for (uint j = 0; j< nb; j++)
			{
				if ((HWND)access.value()[i+j].Hwnd != NULL)
					SetWindowPos ((HWND)access.value()[i+j].Hwnd, NULL, j*delta, y*_ToolBarHeight, delta, _ToolBarHeight, SWP_NOZORDER | SWP_NOACTIVATE );
			}
			i += nb + 1;
			y++;
			if (i >= access.value().size())
				break;
		}

		SetWindowPos (_HEdit, NULL, 0, nby*_ToolBarHeight, Rect.right, Rect.bottom-nby*_ToolBarHeight-_InputEditHeight, SWP_NOZORDER | SWP_NOACTIVATE );
	}
}

void CWinDisplayer::setTitleBar (const string &titleBar)
{
	string wn;
	if (!titleBar.empty())
	{
		wn += titleBar;
		wn += ": ";
	}
	wn += "Nel Service Console (compiled " __DATE__ " " __TIME__ " in " + nlMode + " mode)";

	nldebug("SERVICE: Set title bar to '%s'", wn.c_str());

	SetWindowTextW (_HWnd, (LPWSTR)ucstring::makeFromUtf8(wn).c_str());
}

void CWinDisplayer::open (string titleBar, bool iconified, sint x, sint y, sint w, sint h, sint hs, sint fs, const std::string &fn, bool ww, CLog *log)
{
	if (w == -1)
		w = 700;
	if (h == -1)
		h = 300;
	if (hs == -1)
		hs = 1000;

	Log = log;

	_HistorySize = hs;

	WNDCLASSW wc;
	memset (&wc,0,sizeof(wc));
	wc.style			= CS_HREDRAW | CS_VREDRAW ;//| CS_DBLCLKS;
	wc.lpfnWndProc		= (WNDPROC)WndProc;
	wc.cbClsExtra		= 0;
	wc.cbWndExtra		= 0;
	wc.hInstance		= GetModuleHandleW(NULL);
	wc.hIcon			= NULL;
	wc.hCursor			= LoadCursorW(NULL,(LPWSTR)IDC_ARROW);
	wc.hbrBackground	= (HBRUSH)COLOR_WINDOW;
	wc.lpszClassName	= L"NLDisplayerClass";
	wc.lpszMenuName		= NULL;
	if ( !RegisterClassW(&wc) ) return;

	ULONG	WndFlags;
	RECT	WndRect;
	WndFlags = WS_OVERLAPPEDWINDOW /*| WS_CLIPCHILDREN | WS_CLIPSIBLINGS*/;
	WndRect.left = 0;
	WndRect.top = 0;
	WndRect.right = w;
	WndRect.bottom = h;
	AdjustWindowRect(&WndRect,WndFlags,FALSE);

	// create the window
	_HWnd = CreateWindowW (L"NLDisplayerClass", L"", WndFlags, CW_USEDEFAULT,CW_USEDEFAULT, WndRect.right,WndRect.bottom, NULL, NULL, GetModuleHandle(NULL), NULL);
	SetWindowLongPtrW (_HWnd, GWLP_USERDATA, (LONG_PTR)this);

	_HLibModule = LoadLibraryW(L"RICHED20.DLL");
	if (_HLibModule == NULL)
	{
		nlerror ("RichEdit 2.0 library not found!");
	}

	string rfn;
	if (fn.empty())
		rfn = "courier";
	else
		rfn = fn;

	sint rfs;
	if (fs == 0)
		rfs = 10;
	else
		rfs = fs;

	_HFont = CreateFontW (-MulDiv(rfs, GetDeviceCaps(GetDC(0),LOGPIXELSY), 72), 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, (LPCWSTR)ucstring::makeFromUtf8(rfn).c_str());


	// create the edit control
	DWORD dwStyle = WS_HSCROLL | WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_READONLY | ES_LEFT | ES_MULTILINE /*| ES_AUTOVSCROLL*/;

	if(ww)
		dwStyle &= ~WS_HSCROLL;
	else
		dwStyle |= WS_HSCROLL;

	_HEdit = CreateWindowExW(WS_EX_OVERLAPPEDWINDOW, RICHEDIT_CLASSW, L"", dwStyle, 0, _ToolBarHeight, w, h-_ToolBarHeight-_InputEditHeight, _HWnd, (HMENU) NULL, (HINSTANCE) GetWindowLongPtr(_HWnd, GWLP_HINSTANCE), NULL);
	SendMessage (_HEdit, WM_SETFONT, (WPARAM)_HFont, TRUE);

	// set the edit text limit to lot of :)
	SendMessage (_HEdit, EM_LIMITTEXT, -1, 0);

	CharFormat.cbSize = sizeof(CharFormat);
	CharFormat.dwMask = CFM_COLOR;
	SendMessage(_HEdit,EM_GETCHARFORMAT,(WPARAM)0,(LPARAM)&CharFormat);
	CharFormat.dwEffects &= ~CFE_AUTOCOLOR;

	// create the input edit control
	_HInputEdit = CreateWindowExW(WS_EX_OVERLAPPEDWINDOW, RICHEDIT_CLASSW, L"", WS_CHILD | WS_VISIBLE
		/*| ES_MULTILINE*/ | ES_WANTRETURN | ES_NOHIDESEL | ES_AUTOHSCROLL, 0, h-_InputEditHeight, w, _InputEditHeight,
		_HWnd, NULL, (HINSTANCE) GetWindowLongPtr(_HWnd, GWLP_HINSTANCE), NULL);
	SendMessageW (_HInputEdit, WM_SETFONT, (WPARAM)_HFont, TRUE);

	LRESULT dwEvent = SendMessageW(_HInputEdit, EM_GETEVENTMASK, (WPARAM)0, (LPARAM)0);
	dwEvent |= ENM_MOUSEEVENTS | ENM_KEYEVENTS | ENM_CHANGE;
	SendMessage(_HInputEdit, EM_SETEVENTMASK, (WPARAM)0, (LPARAM)dwEvent);

	// resize the window
	RECT rc;
	SetRect (&rc, 0, 0, w, h);
	AdjustWindowRectEx (&rc, GetWindowStyle (_HWnd), GetMenu (_HWnd) != NULL, GetWindowExStyle (_HWnd));

	LONG flag = SWP_NOZORDER | SWP_NOACTIVATE;

	if (x == -1 && y == -1) flag |= SWP_NOMOVE;
	if (w == -1 && h == -1) flag |= SWP_NOSIZE;

	SetWindowPos (_HWnd, NULL, x, y, w, h, flag);
	SetWindowPos (_HWnd, NULL, x, y, rc.right - rc.left, rc.bottom - rc.top, flag);

	setTitleBar (titleBar);

	if (iconified)
		ShowWindow(_HWnd,SW_MINIMIZE);
	else
		ShowWindow(_HWnd,SW_SHOW);

	SetFocus (_HInputEdit);

	_Init = true;
}

void CWinDisplayer::clear ()
{
	bool focus = (GetFocus() == _HEdit);
	if (focus)
	{
		SendMessage(_HEdit,EM_SETOPTIONS,ECOOP_AND,(LPARAM)~ECO_AUTOVSCROLL);
		SendMessage(_HEdit,EM_SETOPTIONS,ECOOP_AND,(LPARAM)~ECO_AUTOHSCROLL);
	}

	// get number of line
	LRESULT nLine = SendMessageW (_HEdit, EM_GETLINECOUNT, 0, 0) - 1;

	// get size of the last line
	LRESULT nIndex = SendMessageW (_HEdit, EM_LINEINDEX, nLine, 0);

	// select all the text
	SendMessageW (_HEdit, EM_SETSEL, 0, nIndex);

	// clear all the text
	SendMessageW (_HEdit, EM_REPLACESEL, FALSE, (LPARAM) "");

	SendMessageW(_HEdit,EM_SETMODIFY,(WPARAM)TRUE,(LPARAM)0);

	if ( focus )
	{
		SendMessageW(_HEdit,EM_SETOPTIONS,ECOOP_OR,(LPARAM)ECO_AUTOVSCROLL);
		SendMessageW(_HEdit,EM_SETOPTIONS,ECOOP_OR,(LPARAM)ECO_AUTOHSCROLL);
	}
}

void CWinDisplayer::display_main ()
{
	nlassert (_Init);

	while (_Continue)
	{
		//
		// Display the bufferized string
		//

		{
			CSynchronized<std::list<std::pair<uint32, std::string> > >::CAccessor access (&_Buffer);
			std::list<std::pair<uint32, std::string> >::iterator it;

			sint vecSize = (sint)access.value().size();
			//nlassert (vecSize <= _HistorySize);

			if (vecSize > 0)
			{
				// look if we are at the bottom of the edit
				SCROLLINFO	info;
				info.cbSize = sizeof(info);
				info.fMask = SIF_ALL;

				bool bottom = true;
				if (GetScrollInfo(_HEdit,SB_VERT,&info) != 0)
					bottom = (info.nPage == 0) || (info.nMax<=(info.nPos+(int)info.nPage));

				// look if we have the focus
				bool focus = (GetFocus() == _HEdit);
				if (focus)
				{
					SendMessage(_HEdit,EM_SETOPTIONS,ECOOP_AND,(LPARAM)~ECO_AUTOVSCROLL);
					SendMessage(_HEdit,EM_SETOPTIONS,ECOOP_AND,(LPARAM)~ECO_AUTOHSCROLL);
				}

				// store old selection
				DWORD startSel, endSel;
				SendMessage (_HEdit, EM_GETSEL, (WPARAM)&startSel, (LPARAM)&endSel);

				// find how many lines we have to remove in the current output to add new lines

				// get number of line
				LRESULT nLine = SendMessageW (_HEdit, EM_GETLINECOUNT, 0, 0) - 1;

				if (_HistorySize > 0 && nLine+vecSize > _HistorySize)
				{
					int nblineremove = vecSize;
					//nlassert (nblineremove>0 && nblineremove <= _HistorySize);

					if (nblineremove == _HistorySize)
					{
						SendMessage (_HEdit, WM_SETTEXT, 0, (LPARAM) "");
						startSel = endSel = -1;
					}
					else
					{
						LRESULT oldIndex1 = SendMessageW (_HEdit, EM_LINEINDEX, 0, 0);
						//nlassert (oldIndex1 != -1);
						LRESULT oldIndex2 = SendMessageW (_HEdit, EM_LINEINDEX, nblineremove, 0);
						//nlassert (oldIndex2 != -1);
						SendMessageW (_HEdit, EM_SETSEL, oldIndex1, oldIndex2);
						SendMessageW (_HEdit, EM_REPLACESEL, FALSE, (LPARAM) "");

						// update the selection due to the erasing
						sint dt = (sint)(oldIndex2 - oldIndex1);
						if (startSel < 65000)
						{
							if ((sint)startSel-dt < 0) startSel = -1;
							else startSel -= dt;
						}
						else startSel = -1;
						if(endSel < 65000)
						{
							if ((sint)endSel-dt < 0) startSel = endSel = -1;
							else endSel -= dt;
						}
						else startSel = endSel = -1;
					}
				}

				for (it = access.value().begin(); it != access.value().end(); )
				{
					ucstring str = ucstring::makeFromUtf8((*it).second);
					uint32 col = (*it).first;

					// get all string that have the same color
					for (it++; it != access.value().end() && (*it).first == col; it++)
					{
						str += ucstring::makeFromUtf8((*it).second);
					}

					SendMessage (_HEdit, EM_SETSEL, -1, -1);

					if ((col>>24) == 0)
					{
						// there s a specific color
						CharFormat.crTextColor = RGB ((col>>16)&0xFF, (col>>8)&0xFF, col&0xFF);
						SendMessage((HWND) _HEdit, EM_SETCHARFORMAT, (WPARAM) SCF_SELECTION, (LPARAM) &CharFormat);
					}

					// add the string to the edit control
					SendMessageW (_HEdit, EM_REPLACESEL, FALSE, (LPARAM) str.c_str());
				}

				// restore old selection
				SendMessage (_HEdit, EM_SETSEL, startSel, endSel);

				SendMessage(_HEdit,EM_SETMODIFY,(WPARAM)TRUE,(LPARAM)0);

				if (bottom)
					SendMessage(_HEdit,WM_VSCROLL,(WPARAM)SB_BOTTOM,(LPARAM)0L);

				if (focus)
				{
					SendMessage(_HEdit,EM_SETOPTIONS,ECOOP_OR,(LPARAM)ECO_AUTOVSCROLL);
					SendMessage(_HEdit,EM_SETOPTIONS,ECOOP_OR,(LPARAM)ECO_AUTOHSCROLL);
				}
			}

			// clear the log
			access.value().clear ();
		}

		//
		// Update labels
		//

		updateLabels ();

		//
		// Manage windows message
		//

		MSG	msg;
		while (PeekMessageW(&msg,NULL,0,0,PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}

		//
		// Wait
		//

		//////////////////////////////////////////////////////////////////
		// WARNING: READ THIS !!!!!!!!!!!!!!!! ///////////////////////////
		// If at the release time, it freezes here, it's a microsoft bug:
		// http://support.microsoft.com/support/kb/articles/q173/2/60.asp
		nlSleep (1);
	}

	DeleteObject (_HFont);
	_HFont = NULL;
	DestroyWindow (_HWnd);
	_HWnd = NULL;
	DestroyWindow (_HEdit);
	_HEdit = NULL;
	FreeLibrary (_HLibModule);
	_HLibModule = NULL;
}

void CWinDisplayer::getWindowPos (uint32 &x, uint32 &y, uint32 &w, uint32 &h)
{
	RECT rect;
	// get the w and h of the client array
	GetClientRect (_HWnd, &rect);
	w = rect.right - rect.left;
	h = rect.bottom - rect.top;

	// get the x and y of the window (not the client array)
	GetWindowRect (_HWnd, &rect);
	x = rect.left;
	y = rect.top;
}


} // NLMISC

#endif // NL_OS_WINDOWS
