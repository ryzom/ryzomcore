// branch_patcherDlg.h : header file
//

#if !defined(AFX_BRANCH_PATCHERDLG_H__7CA97ADE_2C97_4144_AF32_B6F3B5116214__INCLUDED_)
#define AFX_BRANCH_PATCHERDLG_H__7CA97ADE_2C97_4144_AF32_B6F3B5116214__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000



/*
 * Class by Girish_Bharadwaj@Pictel.com
 */
class CDirDialog
{
public:
     CDirDialog();
     virtual ~CDirDialog();
     int DoBrowse ();
     CString m_strPath;
     CString m_strInitDir;
     CString m_strTitle;
     int  m_iImageIndex;

};



/////////////////////////////////////////////////////////////////////////////
// CBranch_patcherDlg dialog

class CBranch_patcherDlg : public CDialog
{
// Construction
public:
	CBranch_patcherDlg(CWnd* pParent = NULL);	// standard constructor

	void	processCommandLine();
	void	loadConfiguration();
	void	saveConfiguration();

	void	setSrcDirectory( const CString& s );
	void	setDestDirectory( const CString& s );
	void	displayFile( const CString& filename );
	void	displayMessage( const CString& msg, bool insertAtTop=false );
	void	saveFile( const CString& filename );
	void	colorizeDiff();
	void	guessDestDirectory();
	void	extractDirTokens();
	bool	hasTokens() const;
	void	displayTokens();

	CDirDialog		DirDialog;
	CRichEditCtrl	*m_Display;
	bool			SaveDiff, EnteringTokens;
	CString			Token1, Token2, SrcDirBackup, TargetDirBackup, PatchExeDir;
	int				CvsDiffDirLevel;

	
// Dialog Data
	//{{AFX_DATA(CBranch_patcherDlg)
	enum { IDD = IDD_BRANCH_PATCHER_DIALOG };
	CString	m_SrcDir;
	CString	m_DestDir;
	CString	m_Filename;
	CString	m_Tokens;
	CString	m_SrcDirLabel;
	CString	m_TargetDirLabel;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBranch_patcherDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CBranch_patcherDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnButtonSetSrcDir();
	afx_msg void OnButtonSetDestDir();
	afx_msg void OnButtonPatch();
	afx_msg void OnDoPatch();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnClose();
	afx_msg void OnButtonExtractTokens();
	afx_msg void OnButtonClearTokens();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};



//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BRANCH_PATCHERDLG_H__7CA97ADE_2C97_4144_AF32_B6F3B5116214__INCLUDED_)
