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

#if !defined(AFX_SNAPSHOT_TOOL_DLG_H__46526215_8CE7_4A83_BF42_056578B50D2A__INCLUDED_)
#define AFX_SNAPSHOT_TOOL_DLG_H__46526215_8CE7_4A83_BF42_056578B50D2A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// snapshot_tool_dlg.h : header file
//


namespace NL3D
{
	class CCamera;
}

class CObjectViewer;

/**
  * Snapshot tool dialog
  *
  */
class CSnapshotToolDlg : public CDialog
{
// Construction
public:

	enum 
	{
		OutputFormat_Tga = 0,
		OutputFormat_Jpg
	};

	enum
	{
		OutputPath_Custom = 0,
		OutputPath_SameAsInput,
		OutputPath_CurrShapeDirectory,
	};

	enum
	{
		SnapshotAngle_Front = 0,
		SnapshotAngle_Right,
		SnapshotAngle_Left,
		SnapshotAngle_Top,
		SnapshotAngle_Bottom,
		SnapshotAngle_Back
	};


	CSnapshotToolDlg(CObjectViewer *ov, CWnd* pParent = NULL);   // standard constructor
	~CSnapshotToolDlg();
	
// Dialog Data
	//{{AFX_DATA(CSnapshotToolDlg)
	enum { IDD = IDD_SNAPSHOT_TOOL };
	CListBox	m_Log;
	CListBox	m_Filters;
	CString	m_OutputPath;
	CString	m_InputPath;
	BOOL	m_RecurseSubFolder;	
	UINT	m_OutputHeight;
	UINT	m_OutputWidth;	
	int		m_Format;
	int		m_OutputPathOption;
	BOOL	m_DumpTextureSets;
	BOOL	m_ViewBack;
	BOOL	m_ViewBottom;
	BOOL	m_ViewFront;
	BOOL	m_ViewLeft;
	BOOL	m_ViewRight;
	BOOL	m_ViewTop;
	BOOL	m_PostFixViewName;
	//}}AFX_DATA
	


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSnapshotToolDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSnapshotToolDlg)
	afx_msg void OnBrowseInputPath();
	afx_msg void OnBrowseOutputPath();
	afx_msg void OnGo();
	afx_msg void OnAddFilter();
	afx_msg void OnRemoveFilter();
	afx_msg void OnEditFilter();
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	afx_msg void OnChangeWidth();
	afx_msg void OnKillfocusHeight();
	afx_msg void OnKillfocusWidth();
	afx_msg void OnSelchangeOutputpathOption();
	afx_msg void OnSelchangeFormat();
	afx_msg void OnCloseButton();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnStopSnapshots();
	afx_msg void OnViewFront();
	afx_msg void OnViewLeft();
	afx_msg void OnViewRight();
	afx_msg void OnViewTop();
	afx_msg void OnViewAll();
	afx_msg void OnViewNone();
	afx_msg void OnPostFixViewName();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:

	std::deque<std::string> _FilteredFiles;

	// Retrieve the options chosen in the snapshot tool ui from the registry.
	void fromRegistry();
	// Save the options chosen in the snapshot tool ui into the registry.
	void toRegistry();
	void stringFromRegistry(HKEY hKey, const char *name, CString &dest, const CString &defaultStr);
	void updateUIEnabledState();

	static void setCamFromView(uint view, NL3D::CCamera *cam, const NLMISC::CAABBox &bbox);
	uint getSelectedViewCount();
	static std::string viewToString(uint view);
private:
	CObjectViewer *_ObjectViewer;

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SNAPSHOT_TOOL_DLG_H__46526215_8CE7_4A83_BF42_056578B50D2A__INCLUDED_)
