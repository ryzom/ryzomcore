/*
 * $Id:
 */

// client_background_downloaderDlg.h : header file
//

#if !defined(AFX_CLIENT_BACKGROUND_DOWNLOADERDLG_H__BAF8369B_BE81_4FE6_9A6B_72CAF4B26AEE__INCLUDED_)
#define AFX_CLIENT_BACKGROUND_DOWNLOADERDLG_H__BAF8369B_BE81_4FE6_9A6B_72CAF4B26AEE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000



#include "nel/misc/time_nl.h"
#include "nel/misc/mem_stream.h"
#include "nel/misc/inter_window_msg_queue.h"
#include "game_share/bg_downloader_msg.h"
#include "blended_bitmap.h"


namespace NLMISC
{
	class IThread;
}


class CDownloadTask;

#ifndef NIF_INFO
	#define NIF_INFO        16
	#define NIIF_NONE       0
	#define NIIF_INFO       1
	#define NIIF_WARNING    2
	#define NIIF_ERROR		3
	#define NIM_SETVERSION  4
	#define NOTIFYICON_VERSION 3
#endif


/////////////////////////////////////////////////////////////////////////////
// CClient_background_downloaderDlg dialog

/** Main dialog for the ryzom background downloader  
  *
  * \author Nicolas Vizerie
  * \author Nevrax France
  * \date 2006
  */
class CClient_background_downloaderDlg : public CDialog
{
// Construction
public:
	CClient_background_downloaderDlg(CWnd* pParent = NULL);	// standard constructor

	~CClient_background_downloaderDlg();

// Dialog Data
	//{{AFX_DATA(CClient_background_downloaderDlg)
	enum { IDD = IDD_CLIENT_BACKGROUND_DOWNLOADER_DIALOG };
	CBlendedBitmap	m_StatusBitmap;
	CComboBox	m_ThreadPriorityCtrl;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CClient_background_downloaderDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	//}}AFX_VIRTUAL

// Implementation
protected:
	


	BGDownloader::TThreadPriority _ThreadPriority;
	BGDownloader::TThreadPriority _WantedThreadPriority;
	// current state
	BGDownloader::TDownloaderMode _Mode;
	BGDownloader::CTaskDesc		  _Task;	
	BGDownloader::TTaskResult	  _LastTaskResult;	
	uint32						  _AvailablePatchs; // bitfield indexed by 'TDownloadID', filled after a task completion
	bool						  _Verbose;			// true if during the update of the task progress, the client should also
													// display the progress
	bool						  _PriorityChangeAsked;


	enum { TIMER_CHECK_DBL_CLICK = 1, TIMER_MAIN_LOOP, TIMER_SEND_PRIORITY };

	// client request
	bool						  _StopWanted;
	BGDownloader::CTaskDesc		  _WantedTask;
	//
	friend class CDownloadTask;
	CDownloadTask *_DownloadTask;
		
	
	
	HICON m_hIcon;	
	HICON m_hTrayIcon;
	BOOL  _TrayIconAdded;



	// we use VC6 SP5 and don't have the good headers, so redefine that struct here

	struct NOTIFYICONDATAW_V2
	{ 
		DWORD cbSize; 
		HWND hWnd; 
		UINT uID; 
		UINT uFlags; 
		UINT uCallbackMessage; 
		HICON hIcon; 
		WCHAR szTip[128];
		DWORD dwState; //Version 5.0
		DWORD dwStateMask; //Version 5.0
		WCHAR szInfo[256]; //Version 5.0
		union {
			UINT  uTimeout; //Version 5.0
			UINT  uVersion; //Version 5.0
		} DUMMYUNIONNAME;
		WCHAR szInfoTitle[64]; //Version 5.0
		DWORD dwInfoFlags; //Version 5.0
	};

	union
	{
		NOTIFYICONDATAW	   _NID_V1;
		NOTIFYICONDATAW_V2 _NID;		
	};
	

	bool _WaitSingleClick;
	sint64 _WaitSingleClickStartTime;	
	bool _BypassWanted;	
	bool _IsWin2KOrMore;
	// Generated message map functions
	//{{AFX_MSG(CClient_background_downloaderDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnDestroy();
	afx_msg LRESULT OnTrayIconMsg(WPARAM wParam, LPARAM lParam);
	afx_msg void OnMenuShowHide();
	afx_msg void OnMenuExit();
	afx_msg void OnClose();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnBypass();
	afx_msg void OnSelchangeThreadPriority();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	void popTrayMenu();
	void setStatusString(const ucstring &str, 
					     uint32 currentFilesToGet, 
					     uint32 totalFilesToGet,
					     uint32 patchingSize,
					     uint32 totalSize,
						 float currentFileProgress
						);
	void setLocalStatusString(const ucstring &str);
	void setProgress(BOOL enabled, float progress);
	void updateTaskProgressDisplay();
	void setFailBitmap();
	void setSuccessBitmap();		
	void errorMessage(const ucstring &message);
	void protocolError();
	void askStop();
	bool checkValidEnum(int value, int maxValue);
	void handleClientMessage(NLMISC::CMemStream &inMsg);
	void initSearchPaths();
	void setDownloadThreadPriority(BGDownloader::TThreadPriority priority);
	void updateThreadPriority();
	bool isThreadPaused() const;
	void popBalloon(const ucstring &text, const ucstring &title, DWORD balloonIcon = NIIF_INFO);
	void removeTrayIcon();

	
	NLMISC::CInterWindowMsgQueue _ClientMsgQueue;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CLIENT_BACKGROUND_DOWNLOADERDLG_H__BAF8369B_BE81_4FE6_9A6B_72CAF4B26AEE__INCLUDED_)
