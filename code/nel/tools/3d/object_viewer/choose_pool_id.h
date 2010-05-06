#if !defined(AFX_CHOOSE_POOL_ID_H__FBCD9FA0_F8A8_4336_AE2A_81D007E5C855__INCLUDED_)
#define AFX_CHOOSE_POOL_ID_H__FBCD9FA0_F8A8_4336_AE2A_81D007E5C855__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// choose_pool_id.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CChoosePoolID dialog

class CChoosePoolID : public CDialog
{
// Construction
public:
	/// you should set the 'Name' and 'PoolID' values before to create this dialog
	CChoosePoolID(bool freezeID , CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CChoosePoolID)
	enum { IDD = IDD_CHOOSE_POOL_ID };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CChoosePoolID)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL
	public:
	uint32		PoolID; // must be set and then retrieved by the caller
	std::string Name;   // the name to edit

// Implementation
protected:
	bool		_FreezeID;

	// Generated message map functions
	//{{AFX_MSG(CChoosePoolID)
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CHOOSE_POOL_ID_H__FBCD9FA0_F8A8_4336_AE2A_81D007E5C855__INCLUDED_)
