// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2019  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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


#if !defined(AFX_PICK_SOUND_H__14638414_B951_439C_A087_5468A8CDFCE1__INCLUDED_)
#define AFX_PICK_SOUND_H__14638414_B951_439C_A087_5468A8CDFCE1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// pick_sound.h : header file
//
#include "nel/misc/string_mapper.h"
#include <vector>
#include <string>


namespace NLSOUND
{
	class USource;
}

/////////////////////////////////////////////////////////////////////////////
// CPickSound dialog

class CPickSound : public CDialog
{
// Construction
public:
	typedef std::vector<NLMISC::TStringId> TNameVect;
	CPickSound(const TNameVect &names, CWnd* pParent = NULL);   // standard constructor


	const NLMISC::TStringId &getName(void) const { return _CurrName; }

// Dialog Data
	//{{AFX_DATA(CPickSound)
	enum { IDD = IDD_PICK_SOUND };
	CListBox	m_NameList;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPickSound)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	TNameVect			_Names;
	NLMISC::TStringId	_CurrName;

	UINT_PTR _Timer;

	NLMISC::CVector _BackupVel;
	float _BackupGain;
	NLSOUND::USource *_CurrSource;

	// Generated message map functions
	//{{AFX_MSG(CPickSound)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchange();
	afx_msg void OnPlaySound();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnDestroy();
	afx_msg void OnDblclkList();
	afx_msg void OnClose();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	void stopCurrSource();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PICK_SOUND_H__14638414_B951_439C_A087_5468A8CDFCE1__INCLUDED_)
