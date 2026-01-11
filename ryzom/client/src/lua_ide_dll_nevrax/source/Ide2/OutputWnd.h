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

// OutputWnd.h: interface for the COutputWnd class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_OUTPUTWND_H__804E57BF_EABF_48F9_B600_90503B44A217__INCLUDED_)
#define AFX_OUTPUTWND_H__804E57BF_EABF_48F9_B600_90503B44A217__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CScintillaView;
struct SCNotification;

#include "ScintillaBar.h"

class COutputWnd : public CScintillaBar
{
public:
	void GotoLine(CString strLine);
	virtual BOOL Create(CWnd* pParentWnd, UINT nID, LPCTSTR lpszWindowName = NULL, CSize sizeDefault = CSize(200,100), DWORD dwStyle = CBRS_LEFT);
	CScintillaView* GetOutput(int nOutput) { return (CScintillaView*)GetView(nOutput); };
	void SetActiveOutput(int nOutput) { if ( GetActiveView()!=GetView(nOutput) ) SetActiveView(nOutput); };

	COutputWnd();
	virtual ~COutputWnd();

	enum
	{
		outputBuild = 0,
		outputDebug,
		outputFiF
	} outputTypes;

	virtual int OnSci(CScintillaView* pView, SCNotification* pNotify);

protected:
	int OnBuildSci(SCNotification* pNotify);
	int OnDebugSci(SCNotification* pNotify);
};

#endif // !defined(AFX_OUTPUTWND_H__804E57BF_EABF_48F9_B600_90503B44A217__INCLUDED_)
