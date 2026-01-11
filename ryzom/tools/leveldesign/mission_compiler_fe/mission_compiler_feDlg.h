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

// mission_compiler_feDlg.h : header file
//

#if !defined(AFX_MISSION_COMPILER_FEDLG_H__5A765948_0BD6_4A62_982F_11C11323C840__INCLUDED_)
#define AFX_MISSION_COMPILER_FEDLG_H__5A765948_0BD6_4A62_982F_11C11323C840__INCLUDED_

#include "nel/misc/types_nl.h"
#include <map>
#include <vector>
#include <deque>
#include "nel/ligo/ligo_config.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CMissionCompilerFeDlg dialog

struct CMissionState
{
	std::string name;
	std::string state;
	std::string hashKey;
	CMissionState(std::string _name, std::string _state, std::string _hashKey)
	: name(_name), state(_state), hashKey(_hashKey) { }
};

struct CMission
{
	std::string name;
	std::string hashKey;
	CMission(std::string _name, std::string _hashKey)
	: name(_name), hashKey(_hashKey) { }
	bool parsePrim(NLLIGO::IPrimitive const* prim);
};

class CValidationFile
{
public:
	typedef std::map<std::string, CMissionState> TMissionStateContainer;
	std::deque<std::string> _AuthorizedStates;
	TMissionStateContainer _MissionStates;
public:
	//	CValidationFile() { }
	void loadMissionValidationFile(std::string filename);
	void saveMissionValidationFile(std::string filename);
	void insertMission(std::string const& mission, std::string const& hashKey)
	{
		_MissionStates.insert(std::make_pair(mission, CMissionState(mission, defaultState(), hashKey)));
	}
	std::string defaultState()
	{
		if (!_AuthorizedStates.empty())
			return _AuthorizedStates.front();
		else
			return "";
	}
};


class CMissionCompilerFeDlg : public CDialog
{
// Construction
public:
	CMissionCompilerFeDlg(CWnd* pParent = NULL);	// standard constructor

	TToolMode	Mode;

	bool readConfigFile();

	void updateSearchPath();

// Dialog Data
	//{{AFX_DATA(CMissionCompilerFeDlg)
	enum { IDD = IDD_MISSION_COMPILER_FE_DIALOG };
	CButton	m_validateBtn;
	CButton	m_compileBtn;
	CButton	m_publishBtn;
	CListBox	m_listDst;
	CListBox	m_listSrc;
	CString	m_filter;
	CString	m_dataDirectory;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMissionCompilerFeDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL


public:
	NLLIGO::CLigoConfig			LigoConfig;
// Implementation
protected:
	HICON m_hIcon;

	std::vector<std::string>	SearchPaths;
	std::string					LigoConfigFile;
	std::string					DefaultFilter;

	std::string					TestPrimitive;
	std::string					TestPrimitiveDest;
	std::string					ReferenceScript;

	// Publish data
	std::vector<std::string>	ServerName;
	std::vector<std::string>	ServerPathPrim;
	std::vector<std::string>	ServerPathText;
	std::string					LocalTextPath;


	typedef std::map<std::string, std::string>	TFileList;

	TFileList	_SrcList;
	TFileList	_DstList;

	void fillSourceList();
	void updateFileList();
	typedef std::map<std::string, CMission> TMissionContainer;
	bool parsePrimForMissions(NLLIGO::IPrimitive const* prim, TMissionContainer& missions);
	void compile(BOOL publish);
	
	// Generated message map functions
	//{{AFX_MSG(CMissionCompilerFeDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnAdd();
	afx_msg void OnAddAll();
	afx_msg void OnRemove();
	afx_msg void OnRemoveAll();
	afx_msg void OnChangeFilter();
	afx_msg void OnCompile();
	afx_msg void OnSpecialRuncompilertest();
	afx_msg void OnDblclkListSrc();
	afx_msg void OnDblclkListDst();
	afx_msg void OnPublish();
	afx_msg void OnSpecialValidateMissions();
	afx_msg void OnValidate();
	afx_msg void OnChangeDir();
	afx_msg void OnChangeDataDir();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MISSION_COMPILER_FEDLG_H__5A765948_0BD6_4A62_982F_11C11323C840__INCLUDED_)
