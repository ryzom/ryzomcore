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

#if !defined(AFX_EXPORTCBDLG_H__34C63700_C8D7_454C_A752_3722257D9160__INCLUDED_)
#define AFX_EXPORTCBDLG_H__34C63700_C8D7_454C_A752_3722257D9160__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ExportCBDlg.h : header file
//

#include "../export/export.h"

/////////////////////////////////////////////////////////////////////////////
class CExportCBDlg;

/////////////////////////////////////////////////////////////////////////////
// CExportCB

class CExportCB : public IExportCB
{
	CExportCBDlg *_Dialog;
	bool _Canceled;

public:
	CExportCB();

	void setExportCBDlg (CExportCBDlg *dlg);
	void cancel ();
	void pump ();

	// Interface

	virtual bool isCanceled (); // Tell the exporter if it must end as quick as possible
	// Display callbacks
	virtual void dispPass (const std::string &Text); // Pass (generate land, vegetable, etc...)
	virtual void dispPassProgress (float percentage); // [ 0.0 , 1.0 ]
	virtual void dispInfo (const std::string &Text); // Verbose
	virtual void dispWarning (const std::string &Text); // Error but not critical
	virtual void dispError (const std::string &Text); // Should block (misfunction)
};

/////////////////////////////////////////////////////////////////////////////
// CExportCBDlg dialog

class CExportCBDlg : public CDialog
{
	
	CExportCB	_ExportCB;
	bool		_Finished;

// Construction
public:
	CExportCBDlg(CWnd* pParent = NULL);   // standard constructor
	CExportCB *getExportCB () { return &_ExportCB; }

	void setFinishedButton ();
	bool getFinished () { return _Finished; }
	void pump () { _ExportCB.pump (); }

// Dialog Data
	//{{AFX_DATA(CExportCBDlg)
	enum { IDD = IDD_EXPORTCB };
	CEdit	EditCtrl;
	CProgressCtrl	ProgressBar;
	CString	PassText;
	CString	InfoText;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CExportCBDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CExportCBDlg)
	virtual void OnCancel();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EXPORTCBDLG_H__34C63700_C8D7_454C_A752_3722257D9160__INCLUDED_)
