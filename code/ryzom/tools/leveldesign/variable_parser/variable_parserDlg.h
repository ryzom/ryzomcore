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

// VariableParserDlg.h : header file
//

#include "nel/misc/sstring.h"
#include "lua_helper.h"

using namespace NLMISC;
using namespace std;

#if !defined(AFX_VARIABLEPARSERDLG_H__65EA6C6E_3D9A_4929_944C_28D932505ECC__INCLUDED_)
#define AFX_VARIABLEPARSERDLG_H__65EA6C6E_3D9A_4929_944C_28D932505ECC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000



typedef vector< string > ParseParameters;



/////////////////////////////////////////////////////////////////////////////
// CVariableParserDlg dialog

class CVariableParserDlg : public CDialog
{
// Construction
public:
	CVariableParserDlg(CWnd* pParent = NULL);	// standard constructor
	~CVariableParserDlg();

// Dialog Data
	//{{AFX_DATA(CVariableParserDlg)
	enum { IDD = IDD_VARIABLEPARSER_DIALOG };
	CListBox	m_varDefList;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CVariableParserDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CVariableParserDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnHdrBrowse();
	afx_msg void OnTmplBrowse();
	afx_msg void OnFootBrowse();
	afx_msg void OnOutputBrowse();
	afx_msg void OnGenerate();
	afx_msg void OnGenBrowse();
	afx_msg void OnLUABrowse();
	afx_msg void OnAddvardef();
	afx_msg void OnRemvardef();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()


	void ProcessGeneratorFile( const string& generatorFile );

	void ParseTemplate( const ParseParameters& params );

	void BuildParseParameters( ParseParameters& params, uint ligne, uint colonne );


	vector< string > m_nomVariables;
	vector< vector< string > > m_variables;

	CSString m_templateText, m_outputText;

	CLuaState m_luaState;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VARIABLEPARSERDLG_H__65EA6C6E_3D9A_4929_944C_28D932505ECC__INCLUDED_)
