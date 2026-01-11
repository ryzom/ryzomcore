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

#if !defined(AFX_SOUND_DLG_H__2BBC25D5_CD0E_4705_8D66_3912F5B14D88__INCLUDED_)
#define AFX_SOUND_DLG_H__2BBC25D5_CD0E_4705_8D66_3912F5B14D88__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// sound_dlg.h : header file
//

// ***************************************************************************
// CSoundDlg dialog
// ***************************************************************************

class CSoundDlg : public CBaseDialog
{
// Construction
public:
	CSoundDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CSoundDlg)
	enum { IDD = IDD_SOUND };
	CStatic	TextSB;
	CStatic	TextEAX;
	CStatic	TextFMod;
	CStatic	TextForceSoundSoft;
	CButton	EAXCtrl;
	CButton	FModCtrl;
	CButton	ForceSoundSoftCtrl;
	BOOL	SoundOn;
	BOOL	EAX;
	BOOL	FMod;
	BOOL	ForceSoundSoft;
	CSliderCtrl	SoundQuality;
	CStatic		SoundQualityValue;
	int			SoundQualityInt;
	//}}AFX_DATA

	void updateState ();

	void		setAsNumSoundBuffer(uint sb);
	uint		getAsNumSoundBuffer() const;
	
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSoundDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSoundDlg)
	afx_msg void OnSoundOn();
	virtual BOOL OnInitDialog();
	afx_msg void OnEax();
	afx_msg void OnFMod();
	afx_msg void OnForceSoundSoft();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SOUND_DLG_H__2BBC25D5_CD0E_4705_8D66_3912F5B14D88__INCLUDED_)
