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

#if !defined(AFX_PAGECOMTEXT_H__C904D249_575F_460D_9E6B_F3D814B65873__INCLUDED_)
#define AFX_PAGECOMTEXT_H__C904D249_575F_460D_9E6B_F3D814B65873__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PageComtext.h : header file
//
#include "nel/sound/u_audio_mixer.h"
#include "PageBase.h"

/////////////////////////////////////////////////////////////////////////////
// CPageComtext dialog

class CPageComtext : public CPageBase
{
	DECLARE_DYNCREATE(CPageComtext)

// Construction
public:
	CPageComtext(){}
	CPageComtext(NLGEORGES::CSoundDialog *soundDialog);
	~CPageComtext();

	/// The current sound context.
	NLSOUND::CSoundContext	SoundContext;

// Dialog Data
	//{{AFX_DATA(CPageComtext)
	enum { IDD = IDD_PAGE_CONTEXT };
		// NOTE - ClassWizard will add data members here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CPageComtext)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CPageComtext)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PAGECOMTEXT_H__C904D249_575F_460D_9E6B_F3D814B65873__INCLUDED_)
