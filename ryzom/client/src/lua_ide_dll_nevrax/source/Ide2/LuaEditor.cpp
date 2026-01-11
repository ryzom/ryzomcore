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

// LuaEditor.cpp : implementation file
//

#include "stdafx.h"
#include "ide2.h"
#include "LuaEditor.h"

#include "ProjectFile.h"
#include "SString.h"
#include "SciLexer.h"
#include "MainFrame.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CLuaEditor

CLuaEditor::CLuaEditor()
{
	m_bShowCalltips = TRUE;
}

CLuaEditor::~CLuaEditor()
{
}


BEGIN_MESSAGE_MAP(CLuaEditor, CWnd)
	//{{AFX_MSG_MAP(CLuaEditor)
	ON_WM_MOUSEMOVE()
	ON_WM_KEYDOWN()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CLuaEditor message handlers

BOOL CLuaEditor::Create(CWnd *pParentWnd, UINT nCtrlId)
{
	BOOL bCreated = CreateEx(0, "Scintilla","", WS_CHILD|WS_VISIBLE|WS_TABSTOP,
		CRect(0,0,0,0),pParentWnd,nCtrlId);

	if ( !bCreated )
		return FALSE;

	m_fnScintilla = (int (__cdecl *)(void *,int,int,int))
		SendMessage(SCI_GETDIRECTFUNCTION,0,0);
	m_ptrScintilla = (void *)SendMessage(SCI_GETDIRECTPOINTER,0,0);

	Sci(SCI_SETMARGINWIDTHN, 1, 0);

	return TRUE;
}

int CLuaEditor::Sci(int nCmd, int wParam, int lParam)
{
	ASSERT(m_fnScintilla);
	ASSERT(m_ptrScintilla);

	return m_fnScintilla(m_ptrScintilla, nCmd, wParam, lParam);
}

void CLuaEditor::SetFocus(BOOL hasFocus)
{
	if (hasFocus) CWnd::SetFocus();
	Sci(SCI_SETFOCUS, hasFocus);
}



int CLuaEditor::LineFromPoint(CPoint &pt)
{
	return 1+Sci(SCI_LINEFROMPOSITION, Sci(SCI_POSITIONFROMPOINT, pt.x, pt.y), 0);
}

BOOL CLuaEditor::ToggleBreakpoint(int nLine)
{
	if ( Sci(SCI_MARKERGET, nLine-1) & 1 )
	{
		Sci(SCI_MARKERDELETE, nLine-1, 0);
		return FALSE;
	}
	else

	Sci(SCI_MARKERADD, nLine-1, 0);
	return TRUE;
}

BOOL CLuaEditor::Load(CFile* pFile)
{
	const int blockSize = 131072;

	Sci(SCI_CLEARALL);

	char data[blockSize];
	size_t lenFile = pFile->Read(data, sizeof(data));
	while (lenFile > 0) 
	{
		Sci(SCI_ADDTEXT, lenFile, (long)data);
		lenFile = pFile->Read(data, sizeof(data));
	}

	Sci(SCI_SETEOLMODE, SC_EOL_CRLF);
	Sci(SCI_SETUNDOCOLLECTION, 1);
	Sci(SCI_SETSAVEPOINT);
	Sci(SCI_GOTOPOS, 0);

	return TRUE;
}

BOOL CLuaEditor::Save(CFile* pFile)
{
	const int blockSize = 131072;

	TextRange tr;
	char data[blockSize + 1];
	int lengthDoc = Sci(SCI_GETLENGTH);
	for (int i = 0; i < lengthDoc; i += blockSize) 
	{
		int grabSize = lengthDoc - i;
		if (grabSize > blockSize)
			grabSize = blockSize;

		tr.chrg.cpMin = i;
		tr.chrg.cpMax = i + grabSize;
		tr.lpstrText = data;
		Sci(SCI_GETTEXTRANGE, 0, long(&tr));
	
		pFile->Write(data, grabSize);
	}

	Sci(SCI_SETSAVEPOINT);

	return TRUE;	  
}

CString CLuaEditor::GetSubString(int start, int length)
{
	if (start <  0) start = 0;
	int textLength = GetTextLength();
	if (start + length > textLength)
	{
		length = textLength - start;
	}
	char *txt = (char *) malloc(length + 1);
	TextRange tr;
	tr.chrg.cpMin = start;
	tr.chrg.cpMax = start + length;
	tr.lpstrText = txt;
	Sci(SCI_GETTEXTRANGE, 0, long(&tr)
		);
	txt[length] = '\0';
	CString result = txt;
	free(txt);
	return result;
}
void CLuaEditor::ClearAllBreakpoints()
{
	Sci(SCI_MARKERDELETEALL, 0);
}

void CLuaEditor::SetBreakpoint(int nLine)
{
	Sci(SCI_MARKERADD, nLine-1, 0);
}

void CLuaEditor::SetCursorLine(int nLine)
{
	Sci(SCI_GOTOLINE, nLine-1);
}

void CLuaEditor::SetCursorPos(int pos)
{
	Sci(SCI_SETCURRENTPOS, pos);
}

void CLuaEditor::GotoLine(int nLine)
{
	SetCursorLine(nLine);

	Sci(SCI_MARKERDELETEALL, 1);
	Sci(SCI_MARKERADD, nLine-1, 1);
}

void CLuaEditor::SetBreakPointsIn(CProjectFile *pPF)
{
	pPF->RemoveAllBreakPoints();

	int nLine = Sci(SCI_MARKERNEXT, 0, 1);
	while ( nLine>=0 )
	{
		pPF->AddBreakPoint(nLine+1); // from 0-based to 1-based 
		nLine = Sci(SCI_MARKERNEXT, nLine+1, 1);
	}	
}

BOOL CLuaEditor::CanUndo()
{
	return Sci(SCI_CANUNDO);
}

void CLuaEditor::Undo()
{
	Sci(SCI_UNDO);
}

BOOL CLuaEditor::CanRedo()
{
	return Sci(SCI_CANREDO);
}

void CLuaEditor::Redo()
{
	Sci(SCI_REDO);
}

void CLuaEditor::SelectAll()
{
	Sci(SCI_SELECTALL);
}

void CLuaEditor::SetSelection(int start, int end)
{
	Sci(SCI_SETSEL, start, end);	
}

void CLuaEditor::GetSelection(int &start, int &end)
{
	start = Sci(SCI_GETSELECTIONSTART);
	end = Sci(SCI_GETSELECTIONEND, end);
}

BOOL CLuaEditor::CanCutOrClear()
{
	int currentPos = Sci(SCI_GETCURRENTPOS);
	int anchor = Sci(SCI_GETANCHOR);

	return currentPos != anchor;
}

void CLuaEditor::Cut()
{
	Sci(SCI_CUT);
}

void CLuaEditor::Clear()
{
	Sci(SCI_CLEAR);
}

BOOL CLuaEditor::CanPaste()
{
	return Sci(SCI_CANPASTE);
}

void CLuaEditor::Paste()
{
	Sci(SCI_PASTE);
}

void CLuaEditor::Copy()
{
	Sci(SCI_COPY);
}

void CLuaEditor::GrabFocus()
{
	Sci(SCI_GRABFOCUS);
}

void CLuaEditor::SetEditorMargins()
{
	Sci(SCI_SETMARGINTYPEN, 0, SC_MARGIN_NUMBER);
	int pixelWidth = 6 * Sci(SCI_TEXTWIDTH, STYLE_LINENUMBER, (int)"9");
	Sci(SCI_SETMARGINWIDTHN, 0, pixelWidth);

	Sci(SCI_SETMARGINTYPEN, 1, SC_MARGIN_SYMBOL);
	Sci(SCI_SETMARGINWIDTHN, 1, 10);
	Sci(SCI_SETMARGINSENSITIVEN, 1, TRUE);

	Sci(SCI_MARKERDEFINE, 0, SC_MARK_CIRCLE);
	Sci(SCI_MARKERSETFORE, 0, RGB(0xff, 0x00, 0x00));
	Sci(SCI_MARKERSETBACK, 0, RGB(0xff, 0x00, 0x00));

	Sci(SCI_MARKERDEFINE, 1, SC_MARK_ARROW);
	Sci(SCI_MARKERDEFINE, 2, SC_MARK_BACKGROUND);
	Sci(SCI_MARKERSETFORE, 2, RGB(0xff, 0xff, 0xff));
	Sci(SCI_MARKERSETBACK, 2, RGB(0xff, 0x00, 0x00));
}

void CLuaEditor::SetCallStackMargins()
{
	Sci(SCI_SETMARGINTYPEN, 1, SC_MARGIN_SYMBOL);
	Sci(SCI_SETMARGINWIDTHN, 1, 10);
	Sci(SCI_SETMARGINSENSITIVEN, 1, FALSE);

	Sci(SCI_MARKERDEFINE, 0, SC_MARK_ARROW);
}

void CLuaEditor::SetReadOnly(BOOL bReadOnly)
{
	Sci(SCI_SETREADONLY, bReadOnly);
}

void CLuaEditor::AddText(const char* szText)
{
	Sci(SCI_ADDTEXT, strlen(szText), (int)szText);
}

void CLuaEditor::ClearAll()
{
	Sci(SCI_CLEARALL);
}

CString CLuaEditor::GetLine(int nLine)
{
	CString strLine;
	int nLineLength = Sci(SCI_LINELENGTH, nLine-1);
	if ( nLineLength>0 )
	{
		Sci(SCI_GETLINE, nLine-1, (int)strLine.GetBuffer(nLineLength));
		strLine.ReleaseBuffer();
	}

	return strLine;
}

void CLuaEditor::FindText(const char *txt, BOOL matchCase, BOOL wholeWord, BOOL regExp, BOOL forward, int startPos, int &first, int &last)
{
	TextToFind ttf;	
	ttf.chrg.cpMin = startPos;
	ttf.chrg.cpMax = forward ? GetTextLength() : 0;
	ttf.lpstrText = const_cast<char *>(txt);
	int flags = (wholeWord ? SCFIND_MATCHCASE : 0) |
				(matchCase ? SCFIND_WHOLEWORD : 0) |
				(regExp    ? SCFIND_REGEXP    : 0);
	int pos = Sci(SCI_FINDTEXT, flags, (int) &ttf);
	if (pos == -1)
	{
		// continue at beginning
		ttf.chrg.cpMax = startPos;
		ttf.chrg.cpMin = forward ? 0 : GetTextLength();
		pos = Sci(SCI_FINDTEXT, flags, (int) &ttf);
	}

	if (pos == -1)
	{
		first = -1;
		last = -1;		
	}
	else
	{
		first = ttf.chrgText.cpMin;
		last = ttf.chrgText.cpMax;		
		// TODO : show result in status bar
	}
}

int CLuaEditor::GetCursorPos()
{
	return Sci(SCI_GETCURRENTPOS, 0, 0);
}
int CLuaEditor::GetTextLength()
{
	return Sci(SCI_GETTEXTLENGTH, 0, 0);
}
void CLuaEditor::GotoLastLine()
{
	int nLine = Sci(SCI_GETLINECOUNT);
	Sci(SCI_GOTOLINE, nLine);
}

void CLuaEditor::WordLeft()
{
	Sci(SCI_WORDLEFT);
}

void CLuaEditor::WordRight()
{
	Sci(SCI_WORDRIGHT);
}

void CLuaEditor::WordLeftExtend()
{
	Sci(SCI_WORDLEFTEXTEND);
}

void CLuaEditor::WordRightExtend()
{
	Sci(SCI_WORDRIGHTEXTEND);
}

void CLuaEditor::WordPartLeft()
{
	Sci(SCI_WORDPARTLEFT);
}

void CLuaEditor::WordPartRight()
{
	Sci(SCI_WORDPARTRIGHT);
}

void CLuaEditor::WordPartLeftExtend()
{
	Sci(SCI_WORDPARTLEFTEXTEND);
}

void CLuaEditor::WordPartRightExtend()
{
	Sci(SCI_WORDPARTRIGHTEXTEND);
}



int CLuaEditor::GetCurrentLine()
{
	return Sci(SCI_LINEFROMPOSITION, Sci(SCI_GETCURRENTPOS)) + 1;

}

int CLuaEditor::GetLineCount()
{
	return Sci(SCI_GETLINECOUNT);
}


void CLuaEditor::SetStackTraceLevel(int nLevel)
{
	Sci(SCI_MARKERDELETEALL, 0);
	Sci(SCI_MARKERADD, nLevel, 0);
}

CharacterRange CLuaEditor::GetSelection() 
{
	CharacterRange crange;
	crange.cpMin = Sci(SCI_GETSELECTIONSTART);
	crange.cpMax = Sci(SCI_GETSELECTIONEND);
	return crange;
}

BOOL CLuaEditor::PreparePrint(CDC *pDC, CPrintInfo *pInfo)
{
	CharacterRange crange = GetSelection();
	int startPos = crange.cpMin;
	int endPos = crange.cpMax;

	LONG lengthDoc = Sci(SCI_GETLENGTH);
	LONG lengthPrinted = 0;
	LONG lengthDocMax = lengthDoc;

	// Requested to print selection
	if (pInfo->m_pPD->m_pd.Flags & PD_SELECTION) {
		if (startPos > endPos) {
			lengthPrinted = endPos;
			lengthDoc = startPos;
		} else {
			lengthPrinted = startPos;
			lengthDoc = endPos;
		}

		if (lengthPrinted < 0)
			lengthPrinted = 0;
		if (lengthDoc > lengthDocMax)
			lengthDoc = lengthDocMax;
	}

	Sci(SCI_SETWRAPMODE, SC_WRAP_WORD);

	m_pages.RemoveAll();

	RangeToFormat frPrint;
	frPrint.hdc = pDC->GetSafeHdc();
	frPrint.hdcTarget = pDC->m_hAttribDC;
	frPrint.rcPage.left		= frPrint.rc.left	= 0;
	frPrint.rcPage.right	= frPrint.rc.right	= pDC->GetDeviceCaps(HORZRES);
	frPrint.rcPage.top		= frPrint.rc.top	= 0;
	frPrint.rcPage.bottom	= frPrint.rc.bottom = pDC->GetDeviceCaps(VERTRES);

	while (lengthPrinted < lengthDoc) {
		frPrint.chrg.cpMin = lengthPrinted;
		frPrint.chrg.cpMax = lengthDoc;

		m_pages.Add(lengthPrinted);

		lengthPrinted = Sci(SCI_FORMATRANGE, FALSE,
		                           reinterpret_cast<LPARAM>(&frPrint));
	}

	Sci(SCI_FORMATRANGE, FALSE, 0);

	pInfo->SetMaxPage(m_pages.GetSize());

	return TRUE;
}

void CLuaEditor::PrintPage(CDC* pDC, CPrintInfo* pInfo)
{
	RangeToFormat frPrint;
	frPrint.hdc = pDC->GetSafeHdc();
	frPrint.hdcTarget = pDC->m_hAttribDC;
	frPrint.rc.left = pInfo->m_rectDraw.left;
	frPrint.rc.right = pInfo->m_rectDraw.right;
	frPrint.rc.top = pInfo->m_rectDraw.top;
	frPrint.rc.bottom = pInfo->m_rectDraw.bottom;
	frPrint.rcPage.left = pInfo->m_rectDraw.left;
	frPrint.rcPage.right = pInfo->m_rectDraw.right;
	frPrint.rcPage.top = pInfo->m_rectDraw.top;
	frPrint.rcPage.bottom = pInfo->m_rectDraw.bottom;

	frPrint.chrg.cpMin = m_pages[pInfo->m_nCurPage - 1];
	frPrint.chrg.cpMax = Sci(SCI_GETLENGTH);

	Sci(SCI_FORMATRANGE, TRUE, reinterpret_cast<LPARAM>(&frPrint));
}

void CLuaEditor::EndPrint(CDC *pDC, CPrintInfo *pInfo)
{
	Sci(SCI_SETWRAPMODE, SC_WRAP_NONE);
}


void CLuaEditor::SetLuaLexer()
{
   const char font[] = "Verdana";
   const char monospace[] = "Courier";
   const short fontsize = 9;
   const char keywords[] = "and break do else elseif end false for function global if in local nil not or repeat return then true until while";

   // set style bits, choose the right lexer (Lua) and set the keywords list
   Sci(SCI_SETSTYLEBITS,5,0);
   Sci(SCI_SETLEXER,SCLEX_LUA,0);
   Sci(SCI_SETKEYWORDS,0,(LPARAM)keywords);
   
   // set up basic features (iguides on, tab=3, tabs-to-spaces, EOL=CRLF)
   Sci(SCI_SETINDENTATIONGUIDES,1,0);
   Sci(SCI_SETTABWIDTH,3,0);
   Sci(SCI_SETUSETABS,0,0);
   Sci(SCI_SETEOLMODE,SC_EOL_CRLF,0);

   // now set up the styles (remember you have to set up font name for each style;
   // if you fail to do so, bold/italics will not work (only color will work)
   // !!colors are in format BGR!!

   // style 32: default
   Sci(SCI_STYLESETFONT,32, (LPARAM) font);
   Sci(SCI_STYLESETSIZE,32, fontsize);
   // style 0: whitespace
   Sci(SCI_STYLESETFORE,0, 0x808080);
   // style 1: comment (not used in Lua)
   // style 2: line comment (green)
   Sci(SCI_STYLESETFONT,2, (int)monospace);
   Sci(SCI_STYLESETSIZE,2, fontsize);
   Sci(SCI_STYLESETFORE,2, 0x00AA00);
   // style 3: doc comment (grey???)
   Sci(SCI_STYLESETFORE,3, 0x7F7F7F);      
   // style 4: numbers (blue)
   Sci(SCI_STYLESETFORE,4, 0xFF0000);
   // style 5: keywords (black bold)
   Sci(SCI_STYLESETFONT,5, (int)font);
   Sci(SCI_STYLESETSIZE,5, (int)fontsize);
   Sci(SCI_STYLESETFORE,5, 0x000000);
   Sci(SCI_STYLESETBOLD,5, 1);
   // style 6: double qouted strings (???)
   Sci(SCI_STYLESETFORE,6, 0x7F007F);
   // style 7: single quoted strings (???)
   Sci(SCI_STYLESETFORE,7, 0x7F007F);
   // style 8: UUIDs (IDL only, not used in Lua)
   // style 9: preprocessor directives (not used in Lua 4)
   // style 10: operators (black bold)
   Sci(SCI_STYLESETFONT,10, (int)font);
   Sci(SCI_STYLESETSIZE,10, fontsize);
   Sci(SCI_STYLESETFORE,10, 0x000000);
   Sci(SCI_STYLESETBOLD,10, 1);
   // style 11: identifiers (leave to default)
   // style 12: end of line where string is not closed (black on violet, eol-filled)
   Sci(SCI_STYLESETFORE,12, 0x000000);
   Sci(SCI_STYLESETBACK,12, 0xE0C0E0);
   Sci(SCI_STYLESETEOLFILLED,12, 1);
}


void CLuaEditor::OnMouseMove(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	if ( m_bShowCalltips && pFrame->GetMode()==CMainFrame::modeDebugBreak )
	{
		char  linebuf[1000];
		int  pos  =  Sci(SCI_POSITIONFROMPOINT, point.x, point.y);
		int start = Sci(SCI_WORDSTARTPOSITION, pos, TRUE);
		int end = Sci(SCI_WORDENDPOSITION, pos, TRUE);
		TextRange tr;
		tr.chrg.cpMin = start;
		tr.chrg.cpMax = end;
		tr.lpstrText = linebuf;
		Sci(SCI_GETTEXTRANGE, 0, long(&tr));
		
		CString strCalltip;
		if ( pFrame->GetCalltip(linebuf, strCalltip) )
		{
			if  (Sci(SCI_CALLTIPACTIVE) && m_strCallTip!=strCalltip)
					Sci(SCI_CALLTIPCANCEL);

			if (!Sci(SCI_CALLTIPACTIVE))
			{
				Sci(SCI_CALLTIPSHOW,  start,  (int)strCalltip.GetBuffer(0));
				strCalltip.ReleaseBuffer();
				m_strCallTip = strCalltip;
			};
		}
		else if (Sci(SCI_CALLTIPACTIVE))
					Sci(SCI_CALLTIPCANCEL);
	}
	else if (Sci(SCI_CALLTIPACTIVE))
				Sci(SCI_CALLTIPCANCEL);
	
	CWnd::OnMouseMove(nFlags, point);
}

void CLuaEditor::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	if (nChar == VK_ESCAPE)
	{
		SetSelection(GetCursorPos(),  GetCursorPos());
		return;
	}
	CWnd::OnKeyDown(nChar, nRepCnt, nFlags);
}
