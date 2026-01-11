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

#include "stdafx.h"
#include "log_analyser.h"
#include "LogSessions.h"

using namespace std;


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CLogSessions dialog


CLogSessions::CLogSessions(CWnd* pParent /*=NULL*/)
	: CDialog(CLogSessions::IDD, pParent)
{
	//{{AFX_DATA_INIT(CLogSessions)
	m_StartDate = _T("");
	//}}AFX_DATA_INIT
}


void CLogSessions::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLogSessions)
	DDX_Control(pDX, IDC_LIST1, m_ListBox);
	DDX_LBString(pDX, IDC_LIST1, m_StartDate);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CLogSessions, CDialog)
	//{{AFX_MSG_MAP(CLogSessions)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLogSessions message handlers


/*
 *
 */
void CLogSessions::addLogSession( const CString& line )
{
	Sessions.push_back( line );
}


/*
 *
 */
CString CLogSessions::getStartDate() const
{
	return m_StartDate;
	
	/*if ( m_ListBox.GetCurSel() == LB_ERR )
	{
		return "";
	}
	else
	{
		CString res;
		m_ListBox.GetText( m_ListBox.GetCurSel(), res );
		return res;
	}*/
}

BOOL CLogSessions::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	vector<CString>::iterator il;
	for (il=Sessions.begin(); il!=Sessions.end(); ++il )
	{
		m_ListBox.AddString( *il );
	}
	Sessions.clear();

	return TRUE;
}
