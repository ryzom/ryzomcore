// rich_edit_ctrl_ex.cpp : implementation file
//

#include "stdpch.h"
#include "client_background_downloader.h"
#include "rich_edit_ctrl_ex.h"


/////////////////////////////////////////////////////////////////////////////
// CRichEditCtrlEx

CRichEditCtrlEx::CRichEditCtrlEx()
{
	setFont(12, "Verdana", 0);
}

CRichEditCtrlEx::~CRichEditCtrlEx()
{
}


BEGIN_MESSAGE_MAP(CRichEditCtrlEx, CRichEditCtrl)
	//{{AFX_MSG_MAP(CRichEditCtrlEx)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()



static HMODULE Shlwapi = 0;
static bool	   LoadingFailed = false;
typedef LRESULT (WINAPI * TSendMessageWrapW) (HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
static TSendMessageWrapW SendMessageWrapW = NULL;


void CRichEditCtrlEx::append(const ucstring &str)
{
	if (!Shlwapi && !LoadingFailed)
	{
		Shlwapi = LoadLibrary("shlwapi.dll");
		if (!Shlwapi)
		{
			LoadingFailed = true;
		}
		SendMessageWrapW = (TSendMessageWrapW) GetProcAddress(Shlwapi, (const char *) 136 /*"SendMessageWrapW"*/);
	}
	SendMessage(EM_SETSEL, - 1, 0);
	SetSelectionCharFormat(_CharFormat);
	if (SendMessageWrapW)
	{		
		::SendMessageWrapW(m_hWnd, EM_REPLACESEL, (WPARAM) FALSE, (LPARAM) str.c_str());
	}
	else
	{
		SendMessage(EM_REPLACESEL, (WPARAM) FALSE, (LPARAM) str.toString().c_str());
	}	
}


void CRichEditCtrlEx::setFont(LONG size, const char *fontName, DWORD effects)
{	
	ZeroMemory(&_CharFormat, sizeof(_CharFormat));
	_CharFormat.cbSize = sizeof(_CharFormat);

	_CharFormat.dwMask = CFM_BOLD|CFM_ITALIC|CFM_STRIKEOUT|CFM_UNDERLINE|CFM_COLOR|CFM_SIZE|CFE_AUTOCOLOR|CFM_FACE;
	_CharFormat.dwEffects = effects;
	_CharFormat.yHeight = size * (1440 / 72);
	_CharFormat.bCharSet = DEFAULT_CHARSET;
	_CharFormat.bPitchAndFamily = DEFAULT_PITCH;
	strncpy(_CharFormat.szFaceName, fontName, 32);	
}

void CRichEditCtrlEx::clear()
{
	SendMessage(EM_SETSEL, 0, -1);
	SendMessage(EM_REPLACESEL, (WPARAM) FALSE, (LPARAM) "");
}



/////////////////////////////////////////////////////////////////////////////
// CRichEditCtrlEx message handlers
