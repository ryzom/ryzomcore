// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
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


// range_selector.cpp : implementation file
//

#include "std_afx.h"
#include "object_viewer.h"
#include "range_selector.h"
#include "editable_range.h"


/////////////////////////////////////////////////////////////////////////////
// CRangeSelector dialog


CRangeSelector::CRangeSelector(const CString &lowerBound, const CString &upperBound, CEditableRange *er, CWnd* pParent)
	: CDialog(CRangeSelector::IDD, pParent), _EditableRange(er)
{
	//{{AFX_DATA_INIT(CRangeSelector)
	m_LowerBound = lowerBound;
	m_UpperBound = upperBound;
	//}}AFX_DATA_INIT
}


void CRangeSelector::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CRangeSelector)
	DDX_Control(pDX, IDC_UPPER_BOUND, m_UpperBoundCtrl);
	DDX_Control(pDX, IDC_LOWER_BOUND, m_LowerBoundCtrl);
	DDX_Text(pDX, IDC_LOWER_BOUND, m_LowerBound);
	DDX_Text(pDX, IDC_UPPER_BOUND, m_UpperBound);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CRangeSelector, CDialog)
	//{{AFX_MSG_MAP(CRangeSelector)
	ON_EN_SETFOCUS(IDC_LOWER_BOUND, OnSetfocusLowerBound)
	ON_EN_SETFOCUS(IDC_UPPER_BOUND, OnSetfocusUpperBound)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRangeSelector message handlers

void CRangeSelector::OnOK() 
{
	UpdateData() ;
	if (_EditableRange->editableRangeValueValidator(m_LowerBound, m_UpperBound))
	{
		CDialog::OnOK();
	}
}

void CRangeSelector::OnSetfocusLowerBound() 
{
	
	m_LowerBoundCtrl.PostMessage(EM_SETSEL, 0, -1) ;	
	m_LowerBoundCtrl.Invalidate() ; 	
}

void CRangeSelector::OnSetfocusUpperBound() 
{
	m_UpperBoundCtrl.PostMessage(EM_SETSEL, 0, -1) ;	
	m_UpperBoundCtrl.Invalidate() ;
}

BOOL CRangeSelector::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	m_LowerBoundCtrl.PostMessage(EM_SETSEL, 0, -1) ;	
	m_LowerBoundCtrl.Invalidate() ; 	

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
