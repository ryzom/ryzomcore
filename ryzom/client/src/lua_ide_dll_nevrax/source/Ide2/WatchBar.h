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

// WatchBar.h: interface for the CWatchBar class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_WATCHBAR_H__B3588692_9C51_457B_B360_FCF2CF7336A6__INCLUDED_)
#define AFX_WATCHBAR_H__B3588692_9C51_457B_B360_FCF2CF7336A6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "WatchList.h"

class CWatchBar : public CCJControlBar  
{
public:
	CWatchBar();
	virtual ~CWatchBar();

	void Redraw() { m_watches.Redraw(); }

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWatchBar)
	//}}AFX_VIRTUAL

// Generated message map functions
protected:
	CWatchList m_watches;

	//{{AFX_MSG(CWatchBar)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//afx_msg void OnEndlabeledit(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif // !defined(AFX_WATCHBAR_H__B3588692_9C51_457B_B360_FCF2CF7336A6__INCLUDED_)
