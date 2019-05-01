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

#if !defined(AFX_BASE_DIALOG_H__C9035AC0_2958_4540_9FFD_D1DD434CFF9F__INCLUDED_)
#define AFX_BASE_DIALOG_H__C9035AC0_2958_4540_9FFD_D1DD434CFF9F__INCLUDED_

#include "nel/misc/types_nl.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// base_dialog.h : header file
//

class CGeorgesEditView;

/////////////////////////////////////////////////////////////////////////////
// CBaseDialog dialog

class CBaseDialog;

class CDoomyControl : public CStatic
{
public:
	CBaseDialog	*Dlg;
	uint		Index;
	void OnSetFocus( CWnd* pOldWnd );
	DECLARE_MESSAGE_MAP()
};

class CBaseDialog : public CDialog
{
// Construction
public:
	CBaseDialog(int accelResource);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CBaseDialog)
	enum { IDD = IDD_BASE_DIALOG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

	CGeorgesEditView	*View;

	enum
	{
		MinViewWidth = 400,
		MinViewHeight = 400,
		StaticHeight = 15,
		MaxWidgetWidth = 350,
		BigEditHeight = 40,
	};

	enum
	{
		WidgetSpaceWidth = 20,
		WidgetSpaceHeight = 15,

		LabelSpaceHeight = 2,

		EditHeight = 20,

		ListHeight = 200,

		ComboHeight = 25,
		ComboDropDownHeight = 100,

		ButtonHeight = 20,

		SpinWidth = 13,
		SpinHeight = 20,

		BrowseWidth = 30,
		BrowseHeight = 20,

		ResetColorWidth = 40,

		ColorHeight = 20,
		IconHeight = 40,
	};

	void	resizeWidgets (uint virtualWidth, uint evaluatedHeight);

	void	initWidget (CWnd &wnd);

	void	getFirstItemPos (RECT &rect);
	void	getNextPos (RECT &rect);
	void	getNextPosLabel (RECT &rect);
	void	getNextColumn (RECT &rect);
	void	getFirstColumn (RECT &rect);
	void	getNextSpinPos (RECT &rect);
	void	getNextBrowsePos (RECT &rect);
	void	getNextColorPos (RECT &rect);

	void	setComboSize (RECT &rect, uint width, uint height=ComboHeight);
	void	setComboSpinSize (RECT &rect);
	void	setComboBrowseSize (RECT &rect);
	void	adjusteComboSize (RECT &rect);
	void	setButtonSize (RECT &rect, uint width, uint height=ButtonHeight);
	void	setStaticSize (RECT &rect);
	void	setEditSize (RECT &rect, uint width, uint height=EditHeight);
	void	setColorSize (RECT &rect, uint width, uint height=ColorHeight);
	void	setBigEditSize (RECT &rect, uint width, uint height=BigEditHeight);
	void	setEditSpinSize (RECT &rect, uint width, uint height=EditHeight);
	void	setListSize (RECT &rect, uint width, uint height=ListHeight);
	void	setSpinSize (RECT &rect, uint width=SpinWidth, uint height=SpinHeight);
	void	setBrowseSize (RECT &rect, uint width=BrowseWidth, uint height=BrowseHeight);
	void	setResetColorSize (RECT &rect, uint width=ResetColorWidth, uint height=ColorHeight);

	virtual void	onFirstFocus ()=0;
	virtual void	onLastFocus ()=0;

	void	registerLastControl ();
	void	unRegisterLastControl ();

	static void setEditTextMultiLine (CEdit &edit, const char *text);

	virtual void onOpenSelected ();
	virtual void setFocusLastWidget ();

	virtual void OnOK ();
	virtual void OnCancel ();

	HACCEL	Accelerator;

	CDoomyControl	DummyStatic0;
	CDoomyControl	DummyStatic1;

	uint	Width, SmallWidget, SmallWidgetNotLimited, AdjusteHeight;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBaseDialog)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CBaseDialog)
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BASE_DIALOG_H__C9035AC0_2958_4540_9FFD_D1DD434CFF9F__INCLUDED_)
