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

#if !defined(AFX_SELECTIONTERRITOIRE_H__F0C921F8_E5F3_450B_96A1_870762FCD9FF__INCLUDED_)
#define AFX_SELECTIONTERRITOIRE_H__F0C921F8_E5F3_450B_96A1_870762FCD9FF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SelectionTerritoire.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// SelectionTerritoire dialog

extern void Start(void);

namespace NL3D
{
	class CTileBank;
}

class SelectionTerritoire : public CDialog
{
// Construction
public:
	SelectionTerritoire(CWnd* pParent = NULL);   // standard constructor

//owner data
	CString DefautPath; // folder which contains all data
	CString MainFileName; // txt main filename (contains territories list)
	CString CurrentTerritory; // territory name currently being editing
	int MainFileOk;

// Dialog Data
	//{{AFX_DATA(SelectionTerritoire)
	enum { IDD = IDD_TERRITOIREMANAGER };
	CEdit	SurfaceDataCtrl;
	int		SurfaceData;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(SelectionTerritoire)
	protected: 
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(SelectionTerritoire)
	afx_msg void OnAddTerritoire();
	afx_msg void OnEditTerritoire();
	afx_msg void OnRemoveTerritoire();
	afx_msg void OnAddTileSet();
	afx_msg void OnEditTileSet();
	afx_msg void OnRemoveTileSet();
	afx_msg void OnMonter();
	afx_msg void OnDescendre();
	afx_msg void OnSelect();
	virtual void OnOK();
	virtual void OnCancel();
	virtual void OnSave();
	virtual void OnSaveAs();
	afx_msg void OnPath();
	afx_msg void OnExport();
	afx_msg void OnChooseVeget();
	afx_msg void OnChangeSurfaceData();
	afx_msg void OnSelchangeTileSet();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	void Save(const char* path, NL3D::CTileBank &toSave);

	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SELECTIONTERRITOIRE_H__F0C921F8_E5F3_450B_96A1_870762FCD9FF__INCLUDED_)
