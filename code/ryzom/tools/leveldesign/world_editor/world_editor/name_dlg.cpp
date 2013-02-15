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

// name_dlg.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "name_dlg.h"
#include "world_editor.h"
#include "main_frm.h"
#include "nel/misc/path.h"
#include "nel/misc/config_file.h"


/////////////////////////////////////////////////////////////////////////////
// CNameDlg dialog


CNameDlg::CNameDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CNameDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CNameDlg)
	m_nameFilter = _T("");
	//}}AFX_DATA_INIT
}


void CNameDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CNameDlg)
	DDX_Text(pDX, IDC_NAME_FILTER, m_nameFilter);
	DDX_Text(pDX, IDC_NAME_EBOX_GN, m_assignGn);
	DDX_Text(pDX, IDC_NAME_EBOX_IG, m_assignIg);
	DDX_Control(pDX, IDC_NAME_SEARCH, m_searchList);
	DDX_Control(pDX, IDC_NAME_ID, m_idList);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CNameDlg, CDialog)
	//{{AFX_MSG_MAP(CNameDlg)
	ON_EN_CHANGE(IDC_NAME_FILTER, OnChangeNameFilter)
	ON_EN_CHANGE(IDC_NAME_EBOX_GN, OnChangeNameEboxGn)
	ON_EN_CHANGE(IDC_NAME_EBOX_IG, OnChangeNameEboxIg)
	ON_LBN_SELCHANGE(IDC_NAME_SEARCH, OnSelNameSearch)
	ON_BN_CLICKED(ID_NAME_ASSIGN, OnBtnAssign)
	ON_BN_CLICKED(ID_NAME_RESET, OnBtnReset)
	ON_LBN_DBLCLK(IDC_NAME_ID, OnDblClkId)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNameDlg message handlers

BOOL CNameDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// read config var : "BotNamePath"
	NLMISC::CConfigFile::CVar *var = getMainFrame()->getConfigFile().getVarPtr("BotNamePath");
	if (var)
	{
		m_dataDir = NLMISC::CPath::standardizePath(var->asString());
	}
	else
	{
		getMainFrame()->errorMessage("Can't find variable BotNamePath in config file !\nPlease fill your config file.");
		return FALSE;
	}

	// print the data directory
	CWnd* pWnd = GetDlgItem(IDC_NAME_DIR);
	pWnd->SetWindowText(("Data directory: " + m_dataDir).c_str());

	// tab stops to simulate multi-columns edit boxes
	int tab_stop1[1] = {160};
	m_searchList.SetTabStops(1, tab_stop1);
	int tab_stop2[2] = {160, 300};
	m_idList.SetTabStops(2, tab_stop2);

	// reload data
	OnBtnReset();
	
	return TRUE;
}

void CNameDlg::OnOK()
{
	// write file to disk : "bot_names.txt"
	ucstring s = STRING_MANAGER::prepareExcelSheet(m_botNames);
	NLMISC::CI18N::writeTextFile(m_dataDir + "bot_names.txt", s, false);

	// write file to disk : "title_words_wk.txt"
	s = STRING_MANAGER::prepareExcelSheet(m_fcts);
	NLMISC::CI18N::writeTextFile(m_dataDir + "title_words_wk.txt", s, false);

	CDialog::OnOK();
}

void CNameDlg::OnChangeNameFilter()
{
	UpdateData(TRUE);
	updateSearchList();
	updateAssignBox();
}

void CNameDlg::OnChangeNameEboxGn()
{
	UpdateData(TRUE);
	m_searchList.SetCurSel(-1);
	checkNewGn();
	checkAssignBtn();
}

void CNameDlg::OnChangeNameEboxIg()
{
	UpdateData(TRUE);
	m_searchList.SetCurSel(-1);
	checkAssignBtn();
}

void CNameDlg::updateSearchList()
{
	// clear containers
	m_searchList.ResetContent();
	m_listToName.clear();
	
	// Fill search list
	uint i, j;
	for (i=1, j=0 ; i<m_fcts.size() ; i++)
	{
		std::string gn = m_fcts.getData(i, 0).toString();
		std::string ig = m_fcts.getData(i, 1).toString();
		std::string s = gn + "\t" + ig;

		// check if filter is active
		if (m_nameFilter.GetLength() == 0)
		{
			// no filter
			m_listToName.insert(std::make_pair(j, i));
			m_searchList.InsertString(j++, s.c_str());
		}
		else
		{
			std::string filter(m_nameFilter.LockBuffer());
			m_nameFilter.UnlockBuffer();

			// filter
			if (NLMISC::toLower(ig).find(NLMISC::toLower(filter)) != std::string::npos)
			{
				m_listToName.insert(std::make_pair(j, i));
				m_searchList.InsertString(j++, s.c_str());
			}
		}
	}	
}

void CNameDlg::updateSelectList()
{
	// clear containers
	m_idList.ResetContent();
	m_listToId.clear();

	// Fill list with selected primitives
	std::list<NLLIGO::IPrimitive*>::iterator it = m_sel.begin();
	for (uint i=0 ; it!=m_sel.end() ; ++it)
	{
		// Bad primitive selected : do nothing
		std::string id = (*it)->getName();
		if (id == "")
			continue;

		// try to remove "$" in name like : *$fct*$
		uint n = id.find("$");
		if (n != std::string::npos)
			id.erase(n);

		// Check if id exist in files
		std::string s = id;
		uint row;
		if (m_botNames.findRow(0, id, row))
		{
			std::string gn = m_botNames.getData(row, 1).toString();

			// remove $
			if (gn[0] == '$' && gn[gn.size()-1] == '$')
			{
				gn.erase(0, 1);
				gn.erase(gn.size()-1, 1);
			}
			uint r;
			s += ("\t" + gn);
			if (m_fcts.findRow(0, gn, r))
				s += ("\t" + m_fcts.getData(r, 1).toString());

			m_listToId.insert(std::make_pair(i, row));
		}

		m_idList.InsertString(i++, s.c_str());
	}
}

void CNameDlg::OnSelNameSearch()
{
	updateAssignBox();
}

void CNameDlg::updateAssignBox()
{
	uint nameSel = m_searchList.GetCurSel();
	if (nameSel != -1)
	{
		uint row = m_listToName[nameSel];
		m_assignGn = m_fcts.getData(row, 0).toString().c_str();
		m_assignIg = m_fcts.getData(row, 1).toString().c_str();
	}
	else
	{
		m_assignGn = "";
		m_assignIg = "";
	}
	
	checkNewGn();
	checkAssignBtn();
}

void CNameDlg::OnBtnAssign()
{
	// get selection
	std::vector<int> sel;
	sel.resize(m_idList.GetSelCount());
	m_idList.GetSelItems(sel.size(), &sel[0]);

	// get strings
	ucstring id;
	std::string gn = m_assignGn;
	std::string ig = m_assignIg;

	for (uint i=0 ; i<sel.size() ; i++)
	{
		if (m_listToId[sel[i]] != 0)
		{
			// id exists in file
			id = m_botNames.getData(m_listToId[sel[i]], 0);
		}
		else
		{
			// new entry : id not present in file
			CString str;
			uint n = m_idList.GetTextLen(i);
			m_idList.GetText(i, str.GetBuffer(n));
			str.ReleaseBuffer();
			id = str;
		}

		// assign name to selected id
		setName(id, gn, ig);
	}

	updateSelectList();
}

void CNameDlg::OnBtnReset()
{
	// clear maps
	m_fcts.Data.clear();
	m_botNames.Data.clear();

	// clear assign box fields
	m_assignGn = m_assignIg = "";
	
	// load worksheets
	STRING_MANAGER::loadExcelSheet(m_dataDir + "bot_names.txt", m_botNames, true);
	STRING_MANAGER::loadExcelSheet(m_dataDir + "title_words_wk.txt", m_fcts, true);

	// add new names in database
	std::list<NLLIGO::IPrimitive*>::iterator it = m_sel.begin();
	for ( ; it!=m_sel.end() ; ++it)
	{
		std::string id = (*it)->getName();
		
		// check if exist
		uint rowIndex;
		if (!m_botNames.findRow(0, id, rowIndex))
		{
			rowIndex = m_botNames.size();
			m_botNames.resize(rowIndex+1);
			m_botNames.setData(rowIndex, 0, id);
		}
	}

	updateSearchList();
	updateSelectList();
}


void CNameDlg::setSelection(std::list<NLLIGO::IPrimitive*> sel)
{
	// get selected primitives : don't add duplicate because they reference the same bot name

	std::list<NLLIGO::IPrimitive*>::iterator it;

	for (it = sel.begin() ; it!=sel.end() ; ++it)
	{
		std::list<NLLIGO::IPrimitive*>::iterator itFind = m_sel.begin();

		while(itFind != m_sel.end() && ((*it)->getName() != (*itFind)->getName()))
			++itFind;

		if (itFind == m_sel.end())
			m_sel.push_back(*it);
	}
}


void CNameDlg::setName(const ucstring &id, const ucstring &gn, const ucstring &ig)
{
	// search if id exists in the file : we added missing ones at init
	// so, it should never fail
	uint rowIndex;
	if (!m_botNames.findRow(0, id, rowIndex))
	{
		nlassert(1);
	}
	m_botNames.setData(rowIndex, 1, "$" + gn + "$");

	
	// search if generic name exists in the file
	// add an entry or modify it
	if (!m_fcts.findRow(0, gn, rowIndex))
	{
		rowIndex = m_fcts.size();
		m_fcts.resize(rowIndex+1);
		m_fcts.setData(rowIndex, 0, gn);
	}
	m_fcts.setData(rowIndex, 1, ig);
}

void CNameDlg::checkNewGn()
{
	// print a message if a new gn will be added to the list
	CWnd* pWnd = GetDlgItem(IDC_NAME_NEWGN);
	std::string s = m_assignGn;
	uint rowIndex;
	if (s == "")
	{
		pWnd->SetWindowText(" ");
	}
	else if (!m_fcts.findRow(0, s, rowIndex))
	{
		pWnd->SetWindowText("new gn !");
	}
	else
	{
		pWnd->SetWindowText(" ");
		// auto-fill ig field
		m_assignIg = m_fcts.getData(rowIndex, 1).toString().c_str();
	}

	UpdateData(FALSE);
}

void CNameDlg::checkAssignBtn()
{
	// enable if both fields are filled
	CWnd* pWnd = GetDlgItem(ID_NAME_ASSIGN);
	if (m_assignIg == "" || m_assignGn == "")
		pWnd->EnableWindow(FALSE);
	else
		pWnd->EnableWindow(TRUE);
}

void CNameDlg::OnDblClkId()
{
	// get selection
	std::vector<int> sel;
	sel.resize(m_idList.GetSelCount());
	m_idList.GetSelItems(sel.size(), &sel[0]);

	// assume that only the first item selected is valid
	uint rowIndex = m_listToId[sel[0]];

	// fill assign box
	m_assignGn = ("gn_" + m_botNames.getData(rowIndex, 0).toString()).c_str();

	// unselect in search list
	m_searchList.SetCurSel(-1);

	// check for update
	checkNewGn();
	checkAssignBtn();
}
