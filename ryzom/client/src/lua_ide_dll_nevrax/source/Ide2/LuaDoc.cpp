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

// LuaDoc.cpp : implementation of the CLuaDoc class
//

#include "stdafx.h"
#include "ide2.h"

#include "LuaDoc.h"

#include "LuaView.h"
#include "MainFrame.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CLuaDoc

IMPLEMENT_DYNCREATE(CLuaDoc, CDocument)

BEGIN_MESSAGE_MAP(CLuaDoc, CDocument)
	//{{AFX_MSG_MAP(CLuaDoc)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLuaDoc construction/destruction

CLuaDoc::CLuaDoc()
{
	// TODO: add one-time construction code here
	m_ModifTime = CTime(0);
}

CLuaDoc::~CLuaDoc()
{
	if (theApp.GetMainFrame()->m_LastCycledDoc == this)
		theApp.GetMainFrame()->m_LastCycledDoc = NULL;
}


const CString& CLuaDoc::GetTitle() const
{	
	m_TmpTitle = CDocument::GetTitle();
	if (const_cast<CLuaDoc *>(this)->IsModified()) m_TmpTitle += "*";
	return m_TmpTitle;
}


BOOL CLuaDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	return TRUE;
}



/////////////////////////////////////////////////////////////////////////////
// CLuaDoc serialization

void CLuaDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
		CLuaEditor* pEditor = GetView()->GetEditor();
		pEditor->Save(ar.GetFile());
	}
	else
	{
		// TODO: add loading code here
		CLuaEditor* pEditor = GetView()->GetEditor();
		pEditor->Load(ar.GetFile());
	}
	CFileStatus fs;
	VERIFY(ar.GetFile()->GetStatus(fs));	
	m_ModifTime = fs.m_mtime;
}


void CLuaDoc::CheckExternallyModified()
{
	CTime newModifTime = GetModifTime();
	if (newModifTime > m_ModifTime)
	{
		// ask for reload
		CString caption;
		caption.LoadString(AFX_IDS_APP_TITLE);
		CString msg;
		msg.LoadString(IDS_EXTERNALLY_MODIFIED_FILE);
		if (theApp.GetMainFrame()->MessageBox((LPCTSTR) (GetPathName() + msg),   (LPCTSTR) caption,   MB_OKCANCEL) == IDOK)
		{
			CFile f;
			if (f.Open(GetPathName(),   CFile::modeRead))
			{	
				CLuaEditor* pEditor = GetView()->GetEditor();
				pEditor->Load(&f);
			}
		}
		m_ModifTime = newModifTime;
	}	
}

CTime CLuaDoc::GetModifTime() const
{
	return GetModifTime(GetPathName());
}

CTime CLuaDoc::GetModifTime(const CString &filename) const
{
	CFile f;
	if (f.Open(filename, CFile::modeRead))
	{
		CFileStatus fs;
		if (f.GetStatus(fs))
		{
			return fs.m_mtime;
		}
	}
	return CTime(0);
}



/////////////////////////////////////////////////////////////////////////////
// CLuaDoc diagnostics

#ifdef _DEBUG
void CLuaDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CLuaDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CLuaDoc commands

CLuaView* CLuaDoc::GetView()
{
	POSITION pos = GetFirstViewPosition();
	if (pos != NULL)
		return (CLuaView*)GetNextView(pos); // get first one

	return NULL;
}

BOOL CLuaDoc::IsInProject()
{
	return GetProjectFile()!=NULL;
}

CProjectFile* CLuaDoc::GetProjectFile()
{
	CMainFrame* pFrame = theApp.GetMainFrame();
	return pFrame->GetProject()->GetProjectFile(GetPathName());
}

BOOL CLuaDoc::OnOpenDocument(LPCTSTR lpszPathName) 
{
	if (!CDocument::OnOpenDocument(lpszPathName))
		return FALSE;
	
	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	CProjectFile* pPF = pFrame->GetProject()->GetProjectFile(lpszPathName);
	if ( pPF )
		pPF->SetBreakPointsIn(GetView()->GetEditor());
	
	return TRUE;
}

void CLuaDoc::SetModifiedFlag(BOOL bModified /*=TRUE*/)
{
	CProjectFile *pf =	GetProjectFile();
	if (pf)
	{
		pf->SetModifiedFlag(bModified);
	}
}
