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

// LuaView.cpp : implementation of the CLuaView class
//

#include "stdafx.h"
#include "ide2.h"

#include "LuaDoc.h"
#include "LuaView.h"
#include "ProjectFile.h"
#include "MainFrame.h"
#include "GotoLine.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CLuaView

IMPLEMENT_DYNCREATE(CLuaView, CView)

BEGIN_MESSAGE_MAP(CLuaView, CView)
	//{{AFX_MSG_MAP(CLuaView)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, OnUpdateEditCopy)
	ON_COMMAND(ID_EDIT_CUT, OnEditCut)
	ON_UPDATE_COMMAND_UI(ID_EDIT_CUT, OnUpdateEditCut)
	ON_COMMAND(ID_EDIT_PASTE, OnEditPaste)
	ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE, OnUpdateEditPaste)
	ON_COMMAND(ID_EDIT_UNDO, OnEditUndo)
	ON_UPDATE_COMMAND_UI(ID_EDIT_UNDO, OnUpdateEditUndo)
	ON_COMMAND(ID_EDIT_REDO, OnEditRedo)
	ON_UPDATE_COMMAND_UI(ID_EDIT_REDO, OnUpdateEditRedo)
	ON_COMMAND(ID_EDIT_SELECT_ALL, OnEditSelectAll)
	ON_COMMAND(ID_EDIT_CLEAR, OnEditClear)
	ON_UPDATE_COMMAND_UI(ID_EDIT_CLEAR, OnUpdateEditClear)
	ON_COMMAND(ID_TOGGLE_BREAKPOINT, OnToggleBreakpoint)
	ON_UPDATE_COMMAND_UI(ID_TOGGLE_BREAKPOINT, OnUpdateToggleBreakpoint)
	ON_COMMAND(IDM_GOTO_LINE,    OnGotoLine)
	ON_COMMAND(IDM_FIND_TEXT,    OnFindText)
	ON_COMMAND(IDM_FIND_NEXT,    OnFindNext)
	ON_COMMAND(IDM_FIND_PREVIOUS,    OnFindPrevious)
	ON_COMMAND(ID_SELECT_FIND_NEXT,  OnSelectFindNext)
    ON_COMMAND(ID_SELECT_FIND_PREVIOUS,  OnSelectFindPrevious)
	ON_UPDATE_COMMAND_UI(IDM_CHECK_SYNTAX,  OnUpdateCheckSyntax)
	ON_COMMAND(IDM_CHECK_SYNTAX,    OnCheckSyntax)
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()
	//}}AFX_MSG_MAP
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CView::OnFilePrintPreview)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLuaView construction/destruction

CLuaView::CLuaView()
{
	// TODO: add construction code here

}

CLuaView::~CLuaView()
{
}

void CLuaView::OnGotoLine()
{
	CGotoLine gt;
	gt.m_Line = m_editor.GetCurrentLine();	
	if (gt.DoModal() == IDOK)
	{
		if (gt.m_Line == 0) gt.m_Line = 1;
		if ((int) gt.m_Line > m_editor.GetLineCount()) gt.m_Line = m_editor.GetLineCount();
		m_editor.SetCursorLine(gt.m_Line);
	}
}

void CLuaView::OnSelectFindNext()
{
	CFindText &ft = theApp.GetMainFrame()->m_FindText;	
	m_editor.WordPartRight();
	m_editor.WordPartLeftExtend();
	int selStart,  selEnd;	
	m_editor.GetSelection(selStart,  selEnd);	
	ft.m_TextToFind = m_editor.GetSubString(selStart,  selEnd - selStart);
	ft.m_RegExp = FALSE;
	OnFindNext();
}

void CLuaView::OnSelectFindPrevious()
{
	CFindText &ft = theApp.GetMainFrame()->m_FindText;	
	m_editor.WordRight();
	m_editor.WordPartLeftExtend();
	int selStart,  selEnd;
	m_editor.GetSelection(selStart,  selEnd);	
	ft.m_TextToFind = m_editor.GetSubString(selStart,  selEnd - selStart);
	ft.m_RegExp = FALSE;
	OnFindPrevious();
}

void CLuaView::OnFindText()
{	
	CFindText &ft = theApp.GetMainFrame()->m_FindText;	
	// if there's a selection,  use it as a text to search
	int selStart,  selEnd;
	m_editor.GetSelection(selStart,  selEnd);
	bool findNext = false;
	if (selStart != selEnd)
	{
		ft.m_TextToFind = m_editor.GetSubString(selStart,  selEnd - selStart);
		findNext = true;
	}
	if (ft.DoModal() == IDOK)
	{		
		find(true,  findNext);
	}
}

void CLuaView::find(bool forward,  bool next)
{
	CFindText &ft = theApp.GetMainFrame()->m_FindText;	
	if (ft.m_TextToFind.GetLength() == 0) return;	
	int startPos,  endPos;
	int searchStartPos;
	int selStart,  selEnd;
	m_editor.GetSelection(selStart,  selEnd);
	if (selStart != selEnd)
	{
		searchStartPos = selStart;
	}
	else
	{
		searchStartPos = m_editor.GetCursorPos();
	}
	if (next)
	{
		searchStartPos += forward ? 1 : -1;
	}
	m_editor.FindText((LPCTSTR) ft.m_TextToFind,  ft.m_MatchCase,  ft.m_WholeWord,  ft.m_RegExp,  forward,  searchStartPos,  startPos,  endPos);
	if (startPos != -1)
	{		
		m_editor.SetSelection(startPos,  endPos);		
	}
}


void CLuaView::OnFindNext()
{
	find(true,  true);
}

void CLuaView::OnFindPrevious()
{
	find(false,  true);
}
BOOL CLuaView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CLuaView drawing

void CLuaView::OnDraw(CDC* pDC)
{
	CLuaDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	// TODO: add draw code for native data here
}

/////////////////////////////////////////////////////////////////////////////
// CLuaView printing

BOOL CLuaView::OnPreparePrinting(CPrintInfo* pInfo)
{
	if ( m_editor.CanCutOrClear() )
		pInfo->m_pPD->m_pd.Flags &= (~PD_NOSELECTION);

	// default preparation
	return DoPreparePrinting(pInfo);
}

void CLuaView::OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo)
{
	m_editor.PreparePrint(pDC, pInfo);
}


void CLuaView::OnPrint(CDC* pDC, CPrintInfo* pInfo) 
{
	m_editor.PrintPage(pDC, pInfo);	
}

void CLuaView::OnEndPrinting(CDC* pDC, CPrintInfo* pInfo)
{
	m_editor.EndPrint(pDC, pInfo);
}

/////////////////////////////////////////////////////////////////////////////
// CLuaView diagnostics

#ifdef _DEBUG
void CLuaView::AssertValid() const
{
	CView::AssertValid();
}

void CLuaView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CLuaDoc* CLuaView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CLuaDoc)));
	return (CLuaDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CLuaView message handlers

int CLuaView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;

	if ( !m_editor.Create(this, ID_EDIT_CTRL) )
		return -1;
	
	m_editor.SetEditorMargins();
	m_editor.SetLuaLexer();

	return 0;
}

void CLuaView::OnSize(UINT nType, int cx, int cy) 
{
	CView::OnSize(nType, cx, cy);
	
	if ( ::IsWindow(m_editor.m_hWnd) )
		m_editor.SetWindowPos(NULL, 0, 0, cx, cy, 0);
}

BOOL CLuaView::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult) 
{
	NMHDR *lpnmhdr = (LPNMHDR) lParam; 

	if (lpnmhdr->hwndFrom == m_editor.m_hWnd)
	{
		*pResult = OnSci((SCNotification*)lParam);
		return TRUE;
	}
	
	return CView::OnNotify(wParam, lParam, pResult);
}

int CLuaView::OnSci(SCNotification* pNotify)
{
	CPoint pt;
	int nLine;
	switch (pNotify->nmhdr.code)
	{
		case SCN_MARGINCLICK:
			if ( GetDocument()->IsInProject() )
			{
				GetCursorPos(&pt);
				ScreenToClient(&pt);
				nLine = m_editor.LineFromPoint(pt);
				
				ToggleBreakPoint(nLine);
			}
		break;

		case SCI_SETSAVEPOINT:
		case SCN_SAVEPOINTREACHED:
		{
			GetDocument()->SetModifiedFlag(FALSE);
			GetParentFrame()->OnUpdateFrameTitle(TRUE);						
		}
		break;

		case SCN_SAVEPOINTLEFT:
		{
			GetDocument()->SetModifiedFlag(TRUE);
			GetParentFrame()->OnUpdateFrameTitle(TRUE);			
		}
		break;
	}

	return TRUE;
}

void CLuaView::OnEditCopy() 
{
	// TODO: Add your command handler code here
	m_editor.Copy();
}

void CLuaView::OnUpdateEditCopy(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(m_editor.CanCutOrClear());	
}

void CLuaView::OnEditCut() 
{
	m_editor.Cut();
}

void CLuaView::OnUpdateEditCut(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(m_editor.CanCutOrClear());	
}

void CLuaView::OnEditClear() 
{
	m_editor.Clear();
}

void CLuaView::OnUpdateEditClear(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(m_editor.CanCutOrClear());	
}

void CLuaView::OnEditPaste() 
{
	m_editor.Paste();
}

void CLuaView::OnUpdateEditPaste(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(m_editor.CanPaste());
}

void CLuaView::OnEditUndo() 
{
	m_editor.Undo();
}

void CLuaView::OnUpdateEditUndo(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(m_editor.CanUndo());
}

void CLuaView::OnEditRedo() 
{
	m_editor.Redo();
}

void CLuaView::OnUpdateEditRedo(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(m_editor.CanRedo());	
}

void CLuaView::OnEditSelectAll() 
{
	m_editor.SelectAll();	
}


void CLuaView::Activate()
{
	CFrameWnd* pFrame = GetParentFrame();

	if (pFrame != NULL)
		pFrame->ActivateFrame();
	else
		TRACE0("Error: Can not find a frame for document to activate.\n");

	CFrameWnd* pAppFrame;
	if (pFrame != (pAppFrame = (CFrameWnd*)AfxGetApp()->m_pMainWnd))
	{
		ASSERT_KINDOF(CFrameWnd, pAppFrame);
		pAppFrame->ActivateFrame();
	}

	GetEditor()->GrabFocus();
}


void CLuaView::CloseFrame()
{
	CFrameWnd* pFrame = GetParentFrame();

	if (pFrame != NULL)
		pFrame->SendMessage(WM_CLOSE);
}

void CLuaView::OnInitialUpdate() 
{
	CView::OnInitialUpdate();
	
	// use main frame dynamic accel table
	GetParentFrame()->m_hAccelTable = NULL;
}

void CLuaView::ToggleBreakPoint(int nLine)
{
	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();

	if (!pFrame->IsExternalDebug())
	{
		if ( pFrame->GetMode() == CMainFrame::modeDebug ||
			pFrame->GetMode() == CMainFrame::modeDebugBreak )
		{
			nLine = GetDocument()->GetProjectFile()->GetNearestDebugLine(nLine);
			if ( nLine==0 )
				return;
		}
	}

	// if in external debug mode, can add breakpoints all the time

	if ( m_editor.ToggleBreakpoint(nLine) )
		GetDocument()->GetProjectFile()->AddBreakPoint(nLine);
	else
		GetDocument()->GetProjectFile()->RemoveBreakPoint(nLine);
}

void CLuaView::OnToggleBreakpoint() 
{
	if ( GetDocument()->IsInProject() )
		ToggleBreakPoint(m_editor.GetCurrentLine());
}

void CLuaView::OnUpdateToggleBreakpoint(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable(	GetDocument()->IsInProject() );
}


void CLuaView::OnSetFocus(CWnd* pOldWnd) 
{
	CView::OnSetFocus(pOldWnd);	
	m_editor.SetFocus(TRUE);
}

void CLuaView::OnKillFocus(CWnd* pNewWnd) 
{
	CView::OnKillFocus(pNewWnd);
	m_editor.SetFocus(FALSE);	
}

void CLuaView::OnUpdateCheckSyntax(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(theApp.GetActiveDoc() != NULL);	
}

void CLuaView::OnCheckSyntax()
{
	CString code = m_editor.GetSubString(0, m_editor.GetTextLength());
	CString errors;
	if (!theApp.GetMainFrame()->GetDebugger()->GetLuaHelper().CheckSyntax(code, errors))
	{		
		int pos = errors.Find(":");		
		int errLine;
		if (pos != -1)
		{
			const char *lineStr = ((LPCTSTR) errors) + pos + 1;
			errLine = atoi(lineStr);
			m_editor.Sci(SCI_MARKERADD, errLine - 1, 2);
			m_editor.SetCursorLine(errLine);
		}
		CString caption;
		caption.LoadString(AFX_IDS_APP_TITLE);
		theApp.GetMainFrame()->MessageBox((LPCTSTR) errors, (LPCTSTR) caption, MB_ICONEXCLAMATION|MB_OK);
		if (pos != -1)
		{
			m_editor.Sci(SCI_MARKERDELETE, errLine - 1, 2);
		}
	}
	else
	{
		CString msg;
		msg.LoadString(IDS_CHECK_OK);
		CString caption;
		caption.LoadString(AFX_IDS_APP_TITLE);
		theApp.GetMainFrame()->MessageBox((LPCTSTR) msg, (LPCTSTR) caption, MB_OK);
	}	
}

