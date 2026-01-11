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

#if !defined(AFX_EXPORTDLG_H__F0E3FA13_0D4B_4A26_8CFA_1C315EAA1CBE__INCLUDED_)
#define AFX_EXPORTDLG_H__F0E3FA13_0D4B_4A26_8CFA_1C315EAA1CBE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ExportDlg.h : header file
//
#include "resource.h"
/////////////////////////////////////////////////////////////////////////////
// CExportDlg dialog

class CExportDlg : public CDialog
{
// Construction
public:
	CExportDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CExportDlg)
	enum { IDD = IDD_EXPORT };
	CString	OutZoneDir;
	CString	RefZoneDir;
	CString	OutIGDir;
	CString	RefIGDir;
	CString	OutCMBDir;
	CString	RefCMBDir;
	CString	OutAdditionnalIGDir;
	CString	RefAdditionnalIGDir;	
	CString	DFNDir;
	CString	ContinentsDir;
	CString	ContinentFile;
	CString	TileBankFile;
	CString	ColorMapFile;
	CString	HeightMapFile;
	CString	ZFactor;
	CString	HeightMapFile2;
	CString	ZFactor2;
	uint8	Lighting;
	CString	ZoneMin;
	CString	ZoneMax;
	BOOL    ExportCollision;
	BOOL    ExportAdditionnalIGs;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CExportDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CExportDlg)
	virtual void OnOK ();
	afx_msg void OnButtonRefzonedir ();
	afx_msg void OnButtonOutzonedir ();
	afx_msg void OnButtonRefIGdir ();
	afx_msg void OnButtonOutIGdir ();
	afx_msg void OnButtonRefAdditionnalIGdir ();
	afx_msg void OnButtonOutAdditionnalIGdir ();
	afx_msg void OnButtonRefCMBdir ();
	afx_msg void OnButtonOutCMBdir ();	
	afx_msg void OnButtonDFNDir ();
	afx_msg void OnButtonContinentsDir ();
	afx_msg void OnButtonContinentFile ();
	afx_msg void OnButtonTilebankfile ();
	afx_msg void OnButtonColormapfile ();
	afx_msg void OnButtonHeightmapfile ();
	afx_msg void OnButtonHeightmapfile2 ();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	// tool method to choose a path
	bool callChoosePathDlg(CString &dest) const;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EXPORTDLG_H__F0E3FA13_0D4B_4A26_8CFA_1C315EAA1CBE__INCLUDED_)
