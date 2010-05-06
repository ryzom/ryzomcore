#if !defined(AFX_VEGETABLE_COPY_DLG_H__DA5F58B2_7C9D_4ABE_965E_5D01143A627D__INCLUDED_)
#define AFX_VEGETABLE_COPY_DLG_H__DA5F58B2_7C9D_4ABE_965E_5D01143A627D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// vegetable_copy_dlg.h : header file
//


class	CVegetableDlg;


/////////////////////////////////////////////////////////////////////////////
// CVegetableCopyDlg dialog

class CVegetableCopyDlg : public CDialog
{
// Construction
public:
	CVegetableCopyDlg(CVegetableDlg *vegetableDlg, CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CVegetableCopyDlg)
	enum { IDD = IDD_VEGETABLE_COPY };
	CListBox	VegetableList;
	CButton	CheckSubsetCopy;
	CButton	CheckScaleZ;
	CButton	CheckScaleXY;
	CButton	CheckRotateZ;
	CButton	CheckRotateY;
	CButton	CheckRotateX;
	CButton	CheckMesh;
	CButton	CheckMaxDensity;
	CButton	CheckDistance;
	CButton	CheckDensity;
	CButton	CheckColorSetup;
	CButton	CheckColorNoise;
	CButton	CheckBendPhase;
	CButton	CheckBendFactor;
	CButton	CheckAngleSetup;
	BOOL	SubsetCopy;
	BOOL	ScaleZ;
	BOOL	ScaleXY;
	BOOL	RotateZ;
	BOOL	RotateY;
	BOOL	RotateX;
	BOOL	Mesh;
	BOOL	MaxDensity;
	BOOL	Distance;
	BOOL	Density;
	BOOL	ColorSetup;
	BOOL	ColorNoise;
	BOOL	BendPhase;
	BOOL	BendFactor;
	BOOL	AngleSetup;
	int		VegetableSelected;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CVegetableCopyDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	
	CVegetableDlg	*_VegetableDlg;


	void	enableChecks(bool enable);


	// Generated message map functions
	//{{AFX_MSG(CVegetableCopyDlg)
	afx_msg void OnButtonVegetableAppearanceNone();
	afx_msg void OnButtonVegetableApperanceAll();
	afx_msg void OnButtonVegetableGeneralAll();
	afx_msg void OnButtonVegetableGeneralNone();
	afx_msg void OnButtonVegetablePositionAll();
	afx_msg void OnButtonVegetablePositionNone();
	afx_msg void OnCheckVegetableSubsetCopy();
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	virtual void OnOK();
	afx_msg void OnDblclkList1();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	// LastSetup, stored and restored at each init / destroy.
	struct	CLastSetup
	{
		BOOL	SubsetCopy;
		BOOL	ScaleZ;
		BOOL	ScaleXY;
		BOOL	RotateZ;
		BOOL	RotateY;
		BOOL	RotateX;
		BOOL	Mesh;
		BOOL	MaxDensity;
		BOOL	Distance;
		BOOL	Density;
		BOOL	ColorSetup;
		BOOL	ColorNoise;
		BOOL	BendPhase;
		BOOL	BendFactor;
		BOOL	AngleSetup;

		CLastSetup();
	};

	static	CLastSetup		_LastSetup;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VEGETABLE_COPY_DLG_H__DA5F58B2_7C9D_4ABE_965E_5D01143A627D__INCLUDED_)
