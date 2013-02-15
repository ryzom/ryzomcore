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

#if !defined(AFX_EXPORTDLG_H__D698050D_CF34_4D22_8F5B_FF8BAEFF6774__INCLUDED_)
#define AFX_EXPORTDLG_H__D698050D_CF34_4D22_8F5B_FF8BAEFF6774__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ExportDlg.h : header file
//

#include "../export/export.h"

/////////////////////////////////////////////////////////////////////////////
// CExportDlg dialog

class CExportDlg : public CDialog
{

	SExportOptions				*_Options;
	std::vector<std::string>	*_Continents;
	std::string					_ActiveContinent;
	bool						_Finished;

// Construction
public:
	CExportDlg (CWnd* pParent = NULL);   // standard constructor

	void setOptions (SExportOptions &options, std::vector<std::string> &regNames);

// Dialog Data
	//{{AFX_DATA(CExportDlg)
	enum { IDD = IDD_EXPORTOPT };
	CComboBox	ContinentList;
	//BOOL	GenerateLandscape;
	/*
	BOOL	GenerateVegetable;
	CString	OutLandscapeDir;
	CString	OutIGDir;*/
	CString	ContinentName;
	/*CString	LandBankFile;
	CString	LandFarBankFile;
	CString	LandTileNoiseDir;*/
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
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	/*
	afx_msg void OnExploreOutLandscapeDir();
	afx_msg void OnExploreOutIGDir();
	afx_msg void OnExploreLandSmallBankFile();
	afx_msg void OnExploreLandFarBankFile();
	afx_msg void OnExploreLandTileNoiseDir();
	*/
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EXPORTDLG_H__D698050D_CF34_4D22_8F5B_FF8BAEFF6774__INCLUDED_)
