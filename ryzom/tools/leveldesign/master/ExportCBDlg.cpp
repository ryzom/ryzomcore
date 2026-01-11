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

// ExportCBDlg.cpp : implementation file
//

#include "stdafx.h"
#include "master.h"
#include "ExportCBDlg.h"

#ifdef _DEBUG
# ifdef new
#  undef new
# endif
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CExportCB

/////////////////////////////////////////////////////////////////////////////
CExportCB::CExportCB()
{
	_Canceled = false;
}

/////////////////////////////////////////////////////////////////////////////
void CExportCB::setExportCBDlg (CExportCBDlg *dlg)
{
	_Dialog = dlg;
}

/////////////////////////////////////////////////////////////////////////////
void CExportCB::cancel ()
{
	_Canceled = true;
}

/////////////////////////////////////////////////////////////////////////////
void CExportCB::pump()
{
	MSG	msg;
	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
		TranslateMessage (&msg);
		DispatchMessage (&msg);
	}
}

/////////////////////////////////////////////////////////////////////////////
bool CExportCB::isCanceled ()
{
	pump();
	return _Canceled;
}

/////////////////////////////////////////////////////////////////////////////
void CExportCB::dispPass (const std::string &Text)
{
	_Dialog->PassText = Text.c_str();
	_Dialog->ProgressBar.SetPos (0);
	_Dialog->UpdateData (FALSE); // Upload
	_Dialog->Invalidate();
	pump();
}

/////////////////////////////////////////////////////////////////////////////
void CExportCB::dispPassProgress (float percentage)
{
	_Dialog->ProgressBar.SetPos ((sint32)(100*percentage));
	_Dialog->UpdateData (FALSE); // Upload
	pump();
}

/////////////////////////////////////////////////////////////////////////////
void CExportCB::dispInfo (const std::string &Text)
{
	_Dialog->InfoText = CString(Text.c_str()) + "\r\n" + _Dialog->InfoText;
	_Dialog->UpdateData (FALSE); // Upload
	pump();
}

/////////////////////////////////////////////////////////////////////////////
void CExportCB::dispWarning (const std::string &Text)
{
	_Dialog->InfoText = CString("WARNING : ") + CString(Text.c_str()) + "\r\n" + _Dialog->InfoText;
	_Dialog->UpdateData (FALSE); // Upload
	pump();
}

/////////////////////////////////////////////////////////////////////////////
void CExportCB::dispError (const std::string &Text)
{
	_Dialog->InfoText = CString("ERROR : ") + CString(Text.c_str()) + "\r\n" + _Dialog->InfoText;
	_Dialog->UpdateData (FALSE); // Upload
	pump();
}

/////////////////////////////////////////////////////////////////////////////
// CExportCBDlg dialog


/////////////////////////////////////////////////////////////////////////////
CExportCBDlg::CExportCBDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CExportCBDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CExportCBDlg)
	PassText = _T("");
	InfoText = _T("");
	_Finished = false;
	//}}AFX_DATA_INIT
}

/////////////////////////////////////////////////////////////////////////////
void CExportCBDlg::setFinishedButton ()
{
	CButton *but = (CButton*)GetDlgItem (IDCANCEL);
	but->SetWindowText ("FINISHED");
	Invalidate ();
	pump ();
}

/////////////////////////////////////////////////////////////////////////////
void CExportCBDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CExportCBDlg)
	DDX_Control(pDX, IDC_EDIT1, EditCtrl);
	DDX_Control(pDX, IDC_PROGRESS1, ProgressBar);
	DDX_Text(pDX, IDC_PASS, PassText);
	DDX_Text(pDX, IDC_EDIT1, InfoText);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CExportCBDlg, CDialog)
	//{{AFX_MSG_MAP(CExportCBDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CExportCBDlg message handlers

/////////////////////////////////////////////////////////////////////////////
void CExportCBDlg::OnCancel() 
{
	CButton *but = (CButton*)GetDlgItem (IDCANCEL);
	CString zeText;
	but->GetWindowText (zeText);
	if (zeText == "Cancel")
		_ExportCB.cancel ();
	else
		_Finished = true;
	//CDialog::OnCancel();
}

/////////////////////////////////////////////////////////////////////////////
BOOL CExportCBDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	_ExportCB.setExportCBDlg (this);
	ProgressBar.SetRange (0, 100);
	EditCtrl.SetLimitText (200);
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
