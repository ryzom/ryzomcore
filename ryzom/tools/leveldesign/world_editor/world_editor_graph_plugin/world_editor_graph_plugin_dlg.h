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

// world_editor_graph_plugin_dlg.h : header file
//

#if !defined(AFX_WORLDEDITORGRAPHPLUGINDLG_H__4CC93546_46D9_11D5_AE4A_0080ADB4DF70__INCLUDED_)
#define AFX_WORLDEDITORGRAPHPLUGINDLG_H__4CC93546_46D9_11D5_AE4A_0080ADB4DF70__INCLUDED_


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CGraphPlugin;

#include "resource.h"
#include "nel/misc/types_nl.h"
/////////////////////////////////////////////////////////////////////////////
// CWorldEditorGraphPluginDlg dialog

#include "stdafx.h"

class CWorldEditorGraphPluginDlg : public CDialog
{
// Construction
public:
	CWorldEditorGraphPluginDlg(CWnd* pParent = NULL);	// standard constructor
	~CWorldEditorGraphPluginDlg();
	void init(CGraphPlugin*);
	void refresh();
	void rescaleScroll();
	bool isWithin(CRect &rect,CPoint &point);
	int getXCenter(CRect &rect);
	int getYCenter(CRect &rect);
// Dialog Data
	//{{AFX_DATA(CWorldEditorGraphPluginDlg)
	enum { IDD = IDD_WORLDEDITORGRAPHPLUGIN_DIALOG };
	CStatic	m_border;
	CStatic	m_mainFrame;
	CScrollBar	m_scroll_vert;
	CScrollBar	m_scroll_horz;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWorldEditorGraphPluginDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	virtual BOOL OnInitDialog();

	//}}AFX_VIRTUAL
	
// Implementation
public:

	HBITMAP _Hbmp;

	HBITMAP _Hdib;

	BITMAPINFO _DibBitmapInfo;

	uint8* _DibBits;
	
protected:
	HICON m_hIcon;
	CRect	aff_size;
	CGraphPlugin	*graph_plug;
	double magnifier;
	//UINT m_magnifier;
	// Generated message map functions
	//{{AFX_MSG(CWorldEditorGraphPluginDlg)
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnZ50();
	afx_msg void OnZ100();
	afx_msg void OnZ150();
	afx_msg void OnZ200();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRefresh();
	afx_msg void OnClose();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual void OnCancel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_WORLDEDITORGRAPHPLUGINDLG_H__4CC93546_46D9_11D5_AE4A_0080ADB4DF70__INCLUDED_)
