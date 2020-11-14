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

#if !defined(AFX_INSTALL_TASK_DLG_H__34C5E208_AE87_4709_8508_E1F7FB694802__INCLUDED_)
#define AFX_INSTALL_TASK_DLG_H__34C5E208_AE87_4709_8508_E1F7FB694802__INCLUDED_


#include "resource.h"
#include "../client/login_patch.h"
#include "SimpleBrowser.h"
#include "web_page_embedder.h"
//#include "transparent_text.h"
#include "color_static.h"
#include "textured_progress_ctrl.h"
#include "rich_edit_ctrl_ex.h"
#include "blended_bitmap.h"

class CInstallTaskDlg : public CDialog
{
// Construction
public:			
	// check task
	CInstallTaskDlg(CPatchManager::SPatchInfo &infoOnPatch, CWnd *pParent = NULL);   // standard constructor
	// patch task
	CInstallTaskDlg(const std::vector<std::string> &categories, uint totalPatchSize, CWnd *pParent = NULL);   // standard constructor
	CInstallTaskDlg(CWnd *pParent = NULL);
	// Ask the Form to update itself at the new update
	void askGuiUpdate(){_MustUpdateGui = true;}

// Dialog Data
	//{{AFX_DATA(CInstallTaskDlg)
	enum { IDD = IDD_INSTALL_TASK };
	CBlendedBitmap			m_ProgressBM;
	CRichEditCtrlEx			m_TaskTypeText;
	CColorStatic			m_StatusText;
	CColorStatic			m_RemainingTimeText;	
	CTexturedProgressCtrl	m_ProgressCtrl;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CInstallTaskDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation

protected:

	enum TTaskType
	{
		TaskType_Unknwown = 0,
		TaskType_Download,
		TaskType_Install
	};

	enum TTask 
	{ 
		TaskCheck = 0, 
		TaskPatch, 
		WaitPatchTaskEnd,
		TestWebPage = 666 // TMP TMP
	};
	std::vector<std::string>    _Categories; // categories to patch
	CPatchManager::SPatchInfo	*_InfoOnPatch;
	TTask						_Task;
	TTaskType					_TaskType;
	uint						_TotalPatchSize;	
	uint						_AnimStep;
	//SimpleBrowser				_Browser;
	enum { TIMER_MAIN_LOOP = 0 };
	// Generated message map functions
	//{{AFX_MSG(CInstallTaskDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnExit();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnClose();
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);	
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	void setStatusString(const ucstring &str, 
						 uint32 currentFilesToGet, 
						 uint32 totalFilesToGet,
						 uint32 patchingSize,
						 uint32 totalSize,
						 float currentFileProgress
						);

	void handleTaskEnd(bool ok);
	
private:
	bool _MustUpdateGui;	//Must update Lable at next update
	bool _MustExit;	//Must exit at next update
	void beginBrowse();
	void updateWebPageRegion();
	void updateWebPage();
	void updateTaskTypeText();
	void updateProgressIcon();


	void errorMsg(const ucstring &error);

	struct CBrowserEventSink : public CWebPageEmbedder::CBrowserEventSink
	{
		CInstallTaskDlg *Parent;
		virtual void onInvoke(DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS FAR *pDispParams, VARIANT FAR *pVarResult, EXCEPINFO FAR *pExcepInfo, UINT FAR *puArgErr);
	};

	CBrowserEventSink _BrowserEventSink;
	friend struct CBrowserEventSink;
	std::string StartURL;



};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_INSTALL_TASK_DLG_H__34C5E208_AE87_4709_8508_E1F7FB694802__INCLUDED_)
