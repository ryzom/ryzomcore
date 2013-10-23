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

#if !defined(AFX_BROWSE_H__7C7A251D_86AF_4E56_8404_4B4073004C40__INCLUDED_)
#define AFX_BROWSE_H__7C7A251D_86AF_4E56_8404_4B4073004C40__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Browse.h : header file
//
#include "SelectionTerritoire.h"
#include "View.h"

#define REGKEY_TILEDIT "Software\\Nevrax\\Nel\\Tile_Edit"
#define REGKEY_BUTTONZOOM "Zoom button"
#define REGKEY_BUTTONVARIETY "Zoom variety"
#define REGKEY_BUTTONTEXTURE "Texture button"
#define REGKEY_BUTTONSORT "Sort button"
#define REGKEY_BUTTONTEXTINFO "Info button"
#define REGKEY_LISTCOMBOBOX "List type combo box"
#define REGKEY_WNDPL "Window placement"
#define REGKEY_LASTPATH "Last path"

#define SCROLL_MAX 50000

#define MAX_LENGTH_GROUP 25

/////////////////////////////////////////////////////////////////////////////
// Browse dialog : contient les boutons de parametres et la fenetre des textures (TileView)

#define EDGEFILE_EXT ".edge"

class Browse : public CDialog
{
// Construction
public:
	Browse(int nland, CWnd* pParent = NULL);   // standard constructor
	void Init();

	void LoadInThread(void);
	static unsigned long __stdcall MyControllingFunction( void* pParam );
	void UpdateAll(void);
//	TileCtrl m_ctrl;

//my data
	RECT minpos; //position minimum de la fenetre
	int border_x,border_y; //taille de la bordure de la fenetre tile_ctrl a droite et en bas
	int oldsel;
	RECT last_sel; int selection; int lbutton; int control;
	CPoint OriginalPos;
	int land;
	void OnDestroy();
	void UpdateFlags ();
	void Flags (int flagNumber, bool value);

//	listgroup theListGroup;

// Dialog Data
	//{{AFX_DATA(Browse)
	enum { IDD = IDD_BROWSER };
	int m_128x128;
	CTView	m_ctrl;
	CStatic	m_bmpsel;
	CButton	m_infotexte;
	CButton	m_rb_zoom1;
	CButton	m_rb_num;
	CButton	m_rb_jour;
	int		SubGroup0;
	int		SubGroup1;
	int		SubGroup2;
	int		SubGroup3;
	int		SubGroup4;
	int		SubGroup5;
	int		SubGroup6;
	int		SubGroup7;
	int		SubGroup10;
	int		SubGroup11;
	int		SubGroup8;
	int		SubGroup9;
	int		Oriented;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(Browse)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(Browse)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnAlpha();
	afx_msg void OnChangeVariety();
	afx_msg void OnJour();
	afx_msg void OnNuit();
	afx_msg void OnNum();
	afx_msg void OnCancel();
	afx_msg void OnOk();
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnSelchangeListtype();
	afx_msg void OnUpdateTiles();
	afx_msg void OnBatchLoad ();
	afx_msg void OnSubgroup0();
	afx_msg void OnSubgroup1();
	afx_msg void OnSubgroup2();
	afx_msg void OnSubgroup3();
	afx_msg void OnSubgroup4();
	afx_msg void OnSubgroup5();
	afx_msg void OnSubgroup6();
	afx_msg void OnSubgroup7();
	afx_msg void OnSubgroup8();
	afx_msg void OnSubgroup9();
	afx_msg void OnSubgroup10();
	afx_msg void OnSubgroup11();
	afx_msg void OnExportBorder();
	afx_msg void OnImportBorder();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif // !defined(AFX_BROWSE_H__7C7A251D_86AF_4E56_8404_4B4073004C40__INCLUDED_)
