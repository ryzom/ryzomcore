// source_sounds_builderDlg.h : header file
//

#if !defined(AFX_SOURCE_SOUNDS_BUILDERDLG_H__0F86251C_19A9_49CD_9D3C_315B7ADD8177__INCLUDED_)
#define AFX_SOURCE_SOUNDS_BUILDERDLG_H__0F86251C_19A9_49CD_9D3C_315B7ADD8177__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "../src/sound/sound.h"
using namespace NLSOUND;

#include <vector>
using namespace std;

#include "soundpage.h"

/////////////////////////////////////////////////////////////////////////////
// CSource_sounds_builderDlg dialog

class CSource_sounds_builderDlg : public CDialog
{
// Construction
public:
	CSource_sounds_builderDlg(CWnd* pParent = NULL);	// standard constructor

	//void	setModified() { _Modified = true; }
	CString SoundName( HTREEITEM hitem );
	
// Dialog Data
	//{{AFX_DATA(CSource_sounds_builderDlg)
	enum { IDD = IDD_SOURCE_SOUNDS_BUILDER_DIALOG };
	CTreeCtrl	m_Tree;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSource_sounds_builderDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CSource_sounds_builderDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnAddSound();
	afx_msg void OnSelchangedTree1(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSave();
	afx_msg void OnDeleteitemTree1(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnLoad();
	afx_msg void OnMoveUp();
	afx_msg void OnMoveDown();
	afx_msg void OnImport();
	afx_msg void OnBeginlabeleditTree1(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnEndlabeleditTree1(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnOK();
	afx_msg void OnCancel();
	afx_msg void OnImpDir();
	afx_msg void OnKeydownTree1(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSortView();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	vector<CSound*>		_Sounds;

	CSoundPage			*_SoundPage;

	//bool				_Modified;
	bool				_EditingName;

	CString				_Filename;

	void				ResetTree();
	void				UpdateTree();
	HTREEITEM 			AddSound( const char *name );
	HTREEITEM			FindInTree( const char *name );
	uint				addSoundAndFile( const string& name );

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SOURCE_SOUNDS_BUILDERDLG_H__0F86251C_19A9_49CD_9D3C_315B7ADD8177__INCLUDED_)
