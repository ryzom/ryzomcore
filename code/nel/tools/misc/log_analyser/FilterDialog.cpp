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


// FilterDialog.cpp : implementation file
//

#include "stdafx.h"
#include "log_analyser.h"
#include "FilterDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace std;


/////////////////////////////////////////////////////////////////////////////
// CFilterDialog dialog


CFilterDialog::CFilterDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CFilterDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CFilterDialog)
	m_NegFilter = _T("");
	m_PosFilter = _T("");
	m_Sep = _T("");
	//}}AFX_DATA_INIT
}


void CFilterDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFilterDialog)
	DDX_Text(pDX, IDC_NegFilter, m_NegFilter);
	DDX_Text(pDX, IDC_PosFilter, m_PosFilter);
	DDX_Text(pDX, IDC_Sep, m_Sep);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CFilterDialog, CDialog)
	//{{AFX_MSG_MAP(CFilterDialog)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFilterDialog message handlers

BOOL CFilterDialog::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	if ( Trace )
	{
		GetDlgItem( IDC_PosFilterCap )->SetWindowText(_T("Service code"));
		GetDlgItem( IDC_NegFilterCap )->ShowWindow( SW_HIDE );
		GetDlgItem( IDC_SepCap )->ShowWindow( SW_HIDE );
		GetDlgItem( IDC_NegFilter )->ShowWindow( SW_HIDE );
		GetDlgItem( IDC_Sep )->ShowWindow( SW_HIDE );
	}
	else
	{
		GetDlgItem( IDC_PosFilterCap )->SetWindowText(_T("Positive filters (all lines must contain one of these substrings)"));
		GetDlgItem( IDC_NegFilterCap )->ShowWindow( SW_SHOW );
		GetDlgItem( IDC_SepCap )->ShowWindow( SW_SHOW );
		GetDlgItem( IDC_NegFilter )->ShowWindow( SW_SHOW );
		GetDlgItem( IDC_Sep )->SetWindowText(_T(";"));
		GetDlgItem( IDC_Sep )->ShowWindow( SW_SHOW );
	}

	return TRUE;
}


/*
 *
 */
std::vector<CString>	buildVectorFromString( const CString& str, const CString& sep )
{
	std::vector<CString> vec;
	CString str2 = str;
	TCHAR *token;
	token = _tcstok( str2.GetBuffer( str2.GetLength() ), sep );
	while ( token != NULL )
	{
		vec.push_back( CString(token) );
		token = _tcstok( NULL, sep );
	}
	str2.ReleaseBuffer();
	return vec;
}


/*
 *
 */
std::vector<CString>		CFilterDialog::getPosFilter() const
{
	return buildVectorFromString( m_PosFilter, m_Sep );
}


/*
 *
 */
std::vector<CString>		CFilterDialog::getNegFilter() const
{
	return buildVectorFromString( m_NegFilter, m_Sep );
}
