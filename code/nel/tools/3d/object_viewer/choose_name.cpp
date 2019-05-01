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

#include "std_afx.h"
#include "object_viewer.h"
#include "choose_name.h"

/////////////////////////////////////////////////////////////////////////////
// CChooseName dialog


CChooseName::CChooseName(const CString &initialName, CWnd* pParent /*=NULL*/)
	: CDialog(CChooseName::IDD, pParent)
{
	//{{AFX_DATA_INIT(CChooseName)
	m_Name = initialName;
	//}}AFX_DATA_INIT
}


std::string CChooseName::getName()
{
	return NLMISC::tStrToUtf8(m_Name);
}

void CChooseName::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CChooseName)
	DDX_Text(pDX, IDC_NAME_CHOSEN, m_Name);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CChooseName, CDialog)
	//{{AFX_MSG_MAP(CChooseName)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CChooseName message handlers
