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

// CallStack.h: interface for the CCallStack class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CALLSTACK_H__1BC34BF3_8062_4E58_AAEC_8DA50AB351A2__INCLUDED_)
#define AFX_CALLSTACK_H__1BC34BF3_8062_4E58_AAEC_8DA50AB351A2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CScintillaView;
struct SCNotification;

#include "ScintillaBar.h"

class CCallStack : public CScintillaBar
{
public:
	void GotoStackTraceLevel(int nLevel);
	void Add(const char* szDesc, const char* szFile, int nLine);
	void Clear();
	CCallStack();
	virtual ~CCallStack();

	virtual BOOL Create(CWnd* pParentWnd, UINT nID, LPCTSTR lpszWindowName = NULL, CSize sizeDefault = CSize(200,100), DWORD dwStyle = CBRS_LEFT);
	virtual int OnSci(CScintillaView* pView, SCNotification* pNotify);
	int GetLevel() { return m_nCurrentLevel; };
protected:
	int m_nCurrentLevel;
	CUIntArray	m_levels, m_lines;
	CStringArray m_files;
};

#endif // !defined(AFX_CALLSTACK_H__1BC34BF3_8062_4E58_AAEC_8DA50AB351A2__INCLUDED_)
