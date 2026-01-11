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


// log_analyserDlg.h : header file
//

#include "FilterDialog.h"
#include "ViewDialog.h"
#include "LogSessions.h"
#include "PlugInSelector.h"
#include <vector>
#include <map>

#if !defined(AFX_LOG_ANALYSERDLG_H__667551B0_360A_43CD_846F_7D02803A822A__INCLUDED_)
#define AFX_LOG_ANALYSERDLG_H__667551B0_360A_43CD_846F_7D02803A822A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


struct TStampedLine
{
	int		Index;
	CString	Line;
};


class CLAEdit : public CEdit
{
protected:

	//{{AFX_MSG(CLAEdit)
	afx_msg void OnKeyDown( UINT nChar, UINT nRepCnt, UINT nFlags );
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////
// CLog_analyserDlg dialog

class CLog_analyserDlg : public CDialog
{
// Construction
public:
	CLog_analyserDlg(CWnd* pParent = NULL);	// standard constructor

	///
	void						addView( std::vector<CString>& pathNames );

	///
	CViewDialog *				onAddCommon( const std::vector<CString>& filenames );

	///
	void						loadPluginConfiguration();

	///
	bool						addPlugIn( const std::string& dllName );

	///
	void						displayCurrentLine( const CString& line );

	///
	bool						selectText( int lineNum, int colNum, int length );

	///
	void						memorizeFileList( const CString& str ) { MemorizedFileList = str; }

	///
	void						displayFileList();

	///
	void						insertTraceLine( int index, char *traceLine );

	///
	void						getLogSeries( const CString& filenameStr, std::vector<CString>& filenameList );

	///
	bool						isLogSeriesEnabled() const { return ((CButton*)GetDlgItem( IDC_CheckAllFileSeries ))->GetCheck() == 1; }

	///
	void						resizeViews();

	///
	void						beginResizeView( int index );

	///
	CViewDialog					*getCurrentView() { return CurrentView; }

	/// 
	void						setCurrentView( int index ) { if ( index==-1 ) CurrentView=NULL; else CurrentView = Views[index]; }

	//{{AFX_MSG(CLog_analyserDlg)
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	//}}AFX_MSG

	
	CFilterDialog						FilterDialog;
	bool								Trace;
	int									ResizeViewInProgress;
	std::vector<CString>				Plugins;

// Dialog Data
	//{{AFX_DATA(CLog_analyserDlg)
	enum { IDD = IDD_LOG_ANALYSER_DIALOG };
	CScrollBar	m_ScrollBar;
	CLAEdit	m_Edit;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLog_analyserDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

	std::vector<CViewDialog*>			Views;
	CViewDialog*						CurrentView;

	CLogSessions						LogSessionsDialog;
	CPlugInSelector						PlugInSelectorDialog;

	std::multimap<int, TStampedLine>	TraceMap;

	CString								MemorizedFileList;

	TAnalyseFunc						AnalyseFunc;

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CLog_analyserDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnAddView();
	afx_msg void OnAddtraceview();
	afx_msg void OnComputeTraces();
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnReset();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnDestroy();
	afx_msg void OnHelpBtn();
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnDropFiles( HDROP hDropInfo );
	afx_msg void OnDispLineHeaders();
	afx_msg void OnAnalyse();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};


/// Smart sprintf() (from NeL)
int smprintf( char *buffer, size_t count, const char *format, ... );


//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LOG_ANALYSERDLG_H__667551B0_360A_43CD_846F_7D02803A822A__INCLUDED_)
