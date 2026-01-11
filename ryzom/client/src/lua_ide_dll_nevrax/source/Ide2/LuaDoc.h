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

// LuaDoc.h : interface of the CLuaDoc class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_LUADOC_H__60B9BAB0_26C5_4E6D_AB07_404295AEF7E6__INCLUDED_)
#define AFX_LUADOC_H__60B9BAB0_26C5_4E6D_AB07_404295AEF7E6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CLuaView;
class CProjectFile;

class CLuaDoc : public CDocument
{
protected: // create from serialization only
	CLuaDoc();
	DECLARE_DYNCREATE(CLuaDoc)

// Attributes
public:

// Operations
public:
	CLuaView* GetView();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLuaDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	//}}AFX_VIRTUAL

// Implementation
public:
	BOOL DoFileSave() { return CDocument::DoFileSave(); };
	void	CheckExternallyModified();
	CProjectFile* GetProjectFile();
	BOOL IsInProject();
	const CString& GetTitle() const;
	virtual ~CLuaDoc();
	virtual void SetModifiedFlag(BOOL bModified = TRUE);
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	CTime		m_ModifTime;		
	CTime		GetModifTime() const;
	CTime		GetModifTime(const CString &filename) const;
	mutable CString		m_TmpTitle;
// Generated message map functions
protected:
	//{{AFX_MSG(CLuaDoc)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LUADOC_H__60B9BAB0_26C5_4E6D_AB07_404295AEF7E6__INCLUDED_)
