// data_mirrorDlg.h : header file
//

#if !defined(AFX_DATA_MIRRORDLG_H__D812F8AA_58F2_4ED2_B0F3_0CAB1B0DCB77__INCLUDED_)
#define AFX_DATA_MIRRORDLG_H__D812F8AA_58F2_4ED2_B0F3_0CAB1B0DCB77__INCLUDED_

#include "my_list_ctrl.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CData_mirrorDlg dialog

class CEntryFile
{
public:
	enum
	{
		Path = 0,
		NewSize,
		OldSize,
		NewDate,
		OldDate,
		Type,
		ParametersCount,
	};

	std::string		Strings[ParametersCount];
	int				Image;
	uint			NewSizeUI;
	uint			OldSizeUI;
	FILETIME		NewDateST;
	FILETIME		OldDateST;
};

class CData_mirrorDlg : public CDialog
{
// Construction
public:
	CData_mirrorDlg(CWnd* pParent = NULL);	// standard constructor

	uint	ButtonXModifiedFromRight;
	uint	ButtonXAddedFromRight;
	uint	ButtonXRemovedFromRight;
	uint	ListBottomFromBottom;
	uint	ListRightFromRight;

	void	resize ();
	void	buildSourceFiles ();
	void	updateList ();
	void	updateSort ();
	void	addEntry (uint where, const char *file, FILETIME &newDate, FILETIME &oldDate);

	enum
	{
		Modified = 0,
		Added,
		Removed,
		FileTypes
	};

	// Vectors
	std::list<CEntryFile>		Files[FileTypes];
	std::vector<std::string>	FilesToUpdate[FileTypes];

	bool SortOrder;
	uint SortedColumn;

// Dialog Data
	//{{AFX_DATA(CData_mirrorDlg)
	enum { IDD = IDD_DATA_MIRROR_DIALOG };
	CButton	IgnoreCtrl;
	CButton	ModifiedFilterCtrl;
	CButton	AddedFilterCtrl;
	CButton	RemovedFilterCtrl;
	CMyListCtrl	List;
	int		ModifiedFilter;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CData_mirrorDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CData_mirrorDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnIgnore();
	virtual void OnOK();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnClickList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnUpdate();
	afx_msg void OnAddedFilters();
	afx_msg void OnModifiedFilters();
	afx_msg void OnRemovedFilters();
	afx_msg void OnColumnclickList(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DATA_MIRRORDLG_H__D812F8AA_58F2_4ED2_B0F3_0CAB1B0DCB77__INCLUDED_)
